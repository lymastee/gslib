/*
 * Copyright (c) 2016-2017 lymastee, All rights reserved.
 * Contact: lymastee@hotmail.com
 *
 * This file is part of the gslib project.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <malloc.h>
#include <vector>
#include <gslib/pool.h>

__gslib_begin__

class system_pool
{
public:
    static void* born(int size)
    {
        void* newptr = malloc(size);
        assert(newptr && "alloc failed!");
        return newptr;
    }
    static void* flex(void* ptr, int size)
    {
        void* newptr = realloc(ptr, size);
        assert(newptr && "alloc failed!");
        return newptr;
    }
    static void kill(void* ptr) { if(ptr) free(ptr); }
    /* better record the size outside, this function cost too much */
    static int query_size(void* ptr) { return (int)_msize(ptr); }
};

class __gs_novtable free_list abstract
{
public:
    virtual ~free_list() {}
    virtual bool is_empty() const = 0;
    virtual bool is_mine(const void* ptr) const = 0;
    virtual void* born() = 0;
    virtual bool kill(void* ptr) = 0;
};

template<int _elem, int _init, int _max>
class _free_list:
    public free_list
{
protected:
    struct node
    {
        byte    _data[_elem];
        node*   _next;
    };
    node*       _head;
    node*       _record;
    
    struct block
    {
        node*   _ptr;
        int     _size;
    };

    typedef std::vector<block>  blklist;
    blklist     _block;

    friend class _free_list;

public:
    _free_list()
    {
        _head = initialize(_init);
        _record = 0;
    }
    node* initialize(int ctr)
    {
        node* ptr = (node*)malloc(sizeof(node)*ctr);
        assert(ptr && "alloc failed!");
        block blk = { ptr, ctr };
        ctr --;
        for(int i = 0; i < ctr; i ++)
            ptr[i]._next = &ptr[i+1];
        ptr[ctr]._next = 0;
        _block.push_back(blk);
        return ptr;
    }
    virtual ~_free_list()
    {
        destroy();
    }
    void destroy()
    {
        blklist::iterator i = _block.begin();
        for( ; i != _block.end(); ++ i) {
            if(i->_ptr) {
                free(i->_ptr);
                i->_ptr = 0;
            }
        }
    }
    void duplicate()
    {
        assert(_block.size());
        block& blk = _block.back();
        assert(blk._ptr);
        int size = blk._size << 1;
        node* newblk = 0;
        if(_expand(blk._ptr, sizeof(node)*size)) {
            node* ptr = blk._ptr;
            size --;
            for(int i = blk._size; i < size; i ++)
                ptr[i]._next = &ptr[i+1];
            ptr[size]._next = 0;
            newblk = ptr + blk._size;
            blk._size = size + 1;
        }
        else {
            if(size > _max)
                size = _max;
            newblk = initialize(size);
        }
        assert(newblk);
        if(!_head && _record) {
            assert(!_record->_next);
            _head = newblk;
            _record->_next = newblk;
        }
        else if(_head) {
            for(node* p = _head; ; p = p->_next) {
                if(!p->_next) {
                    p->_next = newblk;
                    break;
                }
            }
        }
    }
    virtual bool is_empty() const
    {
        assert(!"unused api!");
        return false;
    }
    virtual bool is_mine(const void* ptr) const
    {
        blklist::const_iterator i = _block.begin();
        for( ; i != _block.end(); ++ i) {
            if((const int)ptr >= (int)i->_ptr && 
                (const int)ptr < (int)(i->_ptr+i->_size)
                )
                return true;
        }
        return false;
    }
    virtual void* born()
    {
        if(!_head)
            duplicate();
        assert(_head);
        void* r = (void*)_head;
        if(!_head->_next)
            _record = _head;
        _head = _head->_next;
        return r;
    }
    virtual bool kill(void* ptr)
    {
        assert(!check_free(ptr) && "ptr already killed!");
        if(is_mine(ptr)) {
            ((node*)ptr)->_next = _head;
            _head = (node*)ptr;
            check_shrink();
            return true;
        }
        return false;
    }
    void check_shrink()
    {
        /* unsupported.. */
    }
    bool check_free(void* ptr)
    {
        for(node* p = _head; p; p = p->_next) {
            if(p == ptr)
                return true;
        }
        return false;
    }
};

class regular_pool
{
protected:
    free_list*      _image[6];

public:
    static regular_pool* get_singleton_ptr()
    {
        static regular_pool inst;
        return &inst;
    }
    ~regular_pool()
    {
        destroy();
    }
    void destroy()
    {
        for(int i = 0; i < _countof(_image); i ++) {
            if(_image[i]) {
                delete _image[i];
                _image[i] = 0;
            }
        }
    }
    int get_flsize(int idx)
    {
        assert(idx >= 0 && idx < _countof(_image));
        idx += 2;
        return 1<<idx;
    }
    void* born(int sz)
    {
        assert(sz <= 128 && "oversized for regular pool!");
        int idx = 0;
        -- sz;
        __asm { /* log2 */
            bsr  eax, sz;
            mov  idx, eax;
        }
        -- idx;
        if(!_image[idx]) {
            switch(idx)
            {
            case 0:
                _image[0] = new _free_list<4, 16, 2048>();
                break;
            case 1:
                _image[1] = new _free_list<8, 16, 2048>();
                break;
            case 2:
                _image[2] = new _free_list<16, 16, 2048>();
                break;
            case 3:
                _image[3] = new _free_list<32, 64, 2048>();
                break;
            case 4:
                _image[4] = new _free_list<64, 8, 2048>();
                break;
            case 5:
                _image[5] = new _free_list<128, 8, 2048>();
                break;
            }
        }
        assert(_image[idx]);
        return _image[idx]->born();
    }
    bool kill(void* ptr)
    {
        for(int i = 0; i < _countof(_image); i ++) {
            if(_image[i] && _image[i]->kill(ptr))
                return true;
        }
        return false;
    }
    bool kill(void* ptr, int size)
    {
        assert(size > 0 && size <= 128);
        return _image[(size-1)>>2]->kill(ptr);
    }
    int query_size(void* ptr)
    {
        for(int i = 0; i < _countof(_image); i ++) {
            if(_image[i] && _image[i]->is_mine(ptr))
                return get_flsize(i);
        }
        return 0;
    }
    bool detect(const void* ptr) const
    {
        for(int i = 0; i < _countof(_image); i ++) {
            if(_image[i] && _image[i]->is_mine(ptr))
                return true;
        }
        return false;
    }

private:
    regular_pool()
    {
        memset(_image, 0, sizeof(_image));
    }
};

#define _regpool regular_pool::get_singleton_ptr()

inline void quick_scrapcpy(dword* des, const dword* src, int loops)
{
    for( ; loops; loops --)
        *des ++ = *src ++;
}

void* pool::born(int size)
{
    if(size > 128)
        return system_pool::born(size);
    return _regpool->born(size);
}

void* pool::flex(void* ptr, int size)
{
    if(size == 0) {
        kill(ptr); 
        return 0;
    }
    if(ptr == 0)
        return born(size);
    bool in_regular_pool = _regpool->detect(ptr);
    if(!in_regular_pool && size > 128) {
        return system_pool::flex(ptr, size);
    }
    else if(in_regular_pool && size > 128) {
        void* newptr = system_pool::born(size);
        int oldsize = _regpool->query_size(ptr);
        assert(oldsize && oldsize < size);
        quick_scrapcpy((dword*)newptr, (const dword*)ptr, oldsize>>2);
        _regpool->kill(ptr);
        return newptr;
    }
    else if(!in_regular_pool && size <= 128) {
        void* newptr = _regpool->born(size);
        int regsize = _regpool->query_size(newptr);
        quick_scrapcpy((dword*)newptr, (const dword*)ptr, regsize>>2);
        system_pool::kill(ptr);
        return newptr;
    }
    else {
        int oldsize = _regpool->query_size(ptr);
        if(oldsize >= size)
            return ptr;
        void* newptr = _regpool->born(size);
        int regsize = _regpool->query_size(newptr);
        quick_scrapcpy((dword*)newptr, (const dword*)ptr, regsize>>2);
        _regpool->kill(ptr);
        return newptr;
    }
}

void pool::kill(void* ptr)
{
    if(!_regpool->kill(ptr))
        system_pool::kill(ptr);
}

void pool::kill(void* ptr, int size)
{
    if(!ptr) return;
    if(size > 128)
        system_pool::kill(ptr);
    bool b = _regpool->kill(ptr, size);
    assert(b);
}

int pool::query_size(void* ptr)
{
    if(int n = _regpool->query_size(ptr))
        return n;
    return system_pool::query_size(ptr);
}

__gslib_end__
