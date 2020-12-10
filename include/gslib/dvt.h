/*
 * Copyright (c) 2016-2020 lymastee, All rights reserved.
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

#pragma once

#ifndef dvt_31727ef2_2dfa_4e64_b93d_deab9c280036_h
#define dvt_31727ef2_2dfa_4e64_b93d_deab9c280036_h

#include <gslib/config.h>
#include <gslib/type.h>
#include <gslib/std.h>

__gslib_begin__

template<class _targetfn>
inline uint method_address(_targetfn fn)
{
    uint r;
    memcpy(&r, &fn, sizeof(r));
    return r;
}

extern uint dsm_get_address(uint addr, uint pthis);
extern void dvt_recover_vtable(void* pthis, void* ovt, int vtsize);

class dvt_bridge_code;

template<class _cls>
class vtable_ops:
    public _cls
{
public:
    typedef vtable_ops<_cls> myref;

public:
    myref() {}
    template<class _arg1>
    myref(_arg1 a1): _cls(a1) {}
    template<class _arg1, class _arg2>
    myref(_arg1 a1, _arg2 a2): _cls(a1, a2) {}
    template<class _arg1, class _arg2, class _arg3>
    myref(_arg1 a1, _arg2 a2, _arg3 a3): _cls(a1, a2, a3) {}
    virtual void end_of_vtable() { __asm { nop } }      /* add some real stuff in this function to prevent optimization in release. */

public:
    int get_virtual_method_index(uint m) const
    {
        uint mv = dsm_final_address(m);
        uint eov = dsm_final_address(method_address(&myref::end_of_vtable));
        uint* ovt = *(uint**)this;
        int i = 0;
        for(;; ++ i) {
            uint cv = dsm_final_address(ovt[i]);
            if(cv == mv)
                return i;
            if(cv == eov) {
                assert(!"get index failed.");
                return i;
            }
        }
        return -1;
    }
    int size_of_vtable() const
    {
        return get_virtual_method_index(
            dsm_final_address(method_address(&myref::end_of_vtable))
            );
    }
    void* create_per_instance_vtable(_cls* p, dvt_bridge_code& bridges)
    {
        assert(p);
        int count = size_of_vtable();
        byte* ovt = *(byte**)p;
        void** pvt = (void**)VirtualAlloc(nullptr, count * 4, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
        assert(pvt);
        bridges.install(ovt, count);
        for(int i = 0; i < count; i ++)
            pvt[i] = bridges.get_bridge(i);
        memcpy(p, &pvt, 4);
        DWORD oldpro;
        VirtualProtect(pvt, count * 4, PAGE_EXECUTE_READ, &oldpro);
        return ovt;
    }
    void destroy_per_instance_vtable(_cls* p, void* ovt)
    {
        assert(p && ovt);
        dvt_recover_vtable(p, ovt, size_of_vtable());
    }
    template<class _func>
    uint replace_vtable_method(_cls* p, int index, _func func)
    {
        uint* vt = *(uint**)p;
        vt += index;
        uint r = *vt;
        DWORD oldpro;
        VirtualProtect(vt, 4, PAGE_EXECUTE_READWRITE, &oldpro);
        *vt = (uint)func;
        VirtualProtect(vt, 4, oldpro, &oldpro);
        return r;
    }

private:
    uint dsm_final_address(uint addr) const
    {
        return dsm_get_address(
            addr, (uint)((void*)this)
        );
    }
};

class dvt_detour_code
{
    typedef unsigned long ulong;

public:
    enum detour_type
    {
        detour_notify,
        detour_reflect,
    };

public:
    dvt_detour_code(detour_type dty, int argsize);
    ~dvt_detour_code();
    void finalize(uint old_func, uint host, uint action);
    uint get_code_address() const { return (uint)_ptr; }

private:
    detour_type     _type;
    int             _argsize;
    byte*           _ptr;
    int             _len;
    ulong           _oldpro;
};

/*
 * The two kind of DVT callings:
 * 1.notify:    the original function would be called first, and then the notify function will be called after, you MUST specify the size of the argument table.
 * 2.reflect:   the original function would NOT be called, simply jump to the reflect function, and the argument table of the reflect function SHOULD be exactly the same with the original function.
 */
#define connect_typed_detour(targettype, target, trigger, host, action, dty, argsize) { \
    auto& vo = vtable_ops<targettype>(nullptr); \
    (target)->ensure_dvt_available<targettype>(); \
    auto* detour = (target)->add_detour(dty, argsize); \
    assert(detour); \
    uint old_func = vo.replace_vtable_method(target, vo.get_virtual_method_index(method_address(trigger)), detour->get_code_address()); \
    detour->finalize(old_func, (uint)host, method_address(action)); \
}

#define connect_typed_notify(targettype, target, trigger, host, action, argsize) \
    connect_typed_detour(targettype, target, trigger, host, action, dvt_detour_code::detour_notify, argsize)

#define connect_notify(target, trigger, host, action, argsize) \
    connect_typed_notify(std::remove_reference_t<decltype(*target)>, target, trigger, host, action, argsize)

#define connect_typed_reflect(targettype, target, trigger, host, action) \
    connect_typed_detour(targettype, target, trigger, host, action, dvt_detour_code::detour_reflect, 0)

#define connect_reflect(target, trigger, host, action) \
    connect_typed_reflect(std::remove_reference_t<decltype(*target)>, target, trigger, host, action)

/*
 * Bridge to jump to the original vtable
 * The reason why we can't simply copy the original vtable was that it's hard to determine how long the vtable actually was.
 * If the target class had a complex derivations, missing a part of the vtable would make you unable to call __super::function.
 */
class dvt_bridge_code
{
public:
    dvt_bridge_code();
    ~dvt_bridge_code();
    void install(void* vt, int count);
    void* get_bridge(int index) const;

protected:
    byte*           _bridges;
    int             _bridge_size;       /* in bytes */
    int             _jt_stride;         /* jump table stride in bytes */
};

class dvt_holder
{
public:
    typedef vector<dvt_detour_code*> detour_list;

public:
    dvt_holder();
    virtual ~dvt_holder();

private:
    void*           _backvt;
    int             _backvtsize;
    void*           _switchvt;
    detour_list     _detours;
    bool            _delete_later;
    dvt_bridge_code _bridges;

public:
    template<class _cls>
    void ensure_dvt_available()
    {
        if(_backvt)
            return;
        auto& vo = vtable_ops<_cls>(nullptr);
        _backvtsize = vo.size_of_vtable();
        _backvt = vo.create_per_instance_vtable(static_cast<_cls*>(this), _bridges);
    }
    dvt_detour_code* add_detour(dvt_detour_code::detour_type dty, int argsize);
    dvt_holder* switch_to_ovt();        /* switch to original vtable */
    dvt_holder* switch_to_dvt();
    /*
     * In case the holder would be tagged more than once by the garbage collector, the cause might be
     * a message re-entrant of the msg callback function. So here we tag it.
     */
    void set_delete_later() { _delete_later = true; }
    bool is_delete_later() const { return _delete_later; }
};

class dvt_collector
{
public:
    typedef vector<dvt_holder*> holder_list;

private:
    holder_list     _holders;
    dvt_collector() {}

public:
    static dvt_collector* get_singleton_ptr()
    {
        static dvt_collector inst;
        return &inst;
    }
    ~dvt_collector() { cleanup(); }
    bool set_delete_later(dvt_holder* holder);
    void cleanup();
};

__gslib_end__

#endif
