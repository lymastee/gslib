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

#ifndef rasm_150d55c1_d63d_40ea_8fd2_9bc280975fe0_h
#define rasm_150d55c1_d63d_40ea_8fd2_9bc280975fe0_h

#include <gslib\pool.h>
#include <gslib\string.h>
#include <gslib\std.h>
#include <rathen\config.h>

/*
 * @ 2013.02.26
 * New assemble tool for rathen. This version we gonna solve the problem that last version of rasm could not support labeled jump.
 * In this version, every chuck of assembly code would be a separate and independent node, linked to a whole code segment, and the
 * label would be a particular node in the tree. In this version, we would not need vslen to do the linkage stuff any more.
 * And the short jump would be available.
 * Plus, in this way, we could see the machine code correspond to every single assemble instruction, and yet we've been very close
 * to a disassemble toolkit, but there were still some problems. The rathen didn't use all the x86 machine code. This assemble tool 
 * just concerned about a subset of all the x86 machine codes. I didn't intend to provide such a tool to decode all the machine codes
 * into assemble, or rathen source code, either. As the optimize work we might probably did in the future, this decode works would
 * be a hard problem, which was considered unnecessary.
 */

__rathen_begin__

enum reg
{
    _regzr = 0,
    _eax = 1,
    _ecx = 2,
    _edx,
    _ebx,
    _esp,
    _ebp,
    _esi,
    _edi,
    /* more to come... */
};

#ifdef _GS_X86

inline byte get_register_index(reg r)
{
    switch(r)
    {
    case _eax:      return 0;
    case _ecx:      return 1;
    case _edx:      return 2;
    case _ebx:      return 3;
    case _esp:      return 4;   /* why "no index" */
    case _ebp:      return 5;
    case _esi:      return 6;
    case _edi:      return 7;
    default:        assert(0);
    }
    return -1;
}

typedef const gchar*    lab;

#define __rasm_make_type(tname, dname) \
    struct tname \
    { \
        dname       _data; \
        \
        explicit tname(dname d = (dname)0) { _data = d; } \
        bool operator == (const tname& d) const { return _data == d._data; } \
        dname inner_data() const { return _data; } \
    }

__rasm_make_type(_bias_, int);
__rasm_make_type(_bias8_, int8);
__rasm_make_type(_bias16_, int16);
__rasm_make_type(_mem_, int);
__rasm_make_type(_reg_, reg);

enum cccc
{
    cccc_o = 0,
    cccc_no,
    cccc_c,
    cccc_nc,
    cccc_e,
    cccc_ne,
    cccc_be,
    cccc_a,
    cccc_s,
    cccc_ns,
    cccc_p,
    cccc_np,
    cccc_l,
    cccc_ge,
    cccc_le,
    cccc_g,
};

struct __gs_novtable rasm_instr abstract
{
    virtual ~rasm_instr() {}
    virtual const gchar* get_name() const = 0;
    virtual bool finalize(vessel& vsl) = 0;
    virtual void to_string(string& str) const = 0;
    virtual bool compare(const rasm_instr* that) const = 0;
};

inline const gchar* nameofreg(reg r)
{
    switch(r)
    {
    case _eax:  return _t("eax");
    case _ecx:  return _t("ecx");
    case _edx:  return _t("edx");
    case _ebx:  return _t("ebx");
    case _esp:  return _t("esp");
    case _ebp:  return _t("ebp");
    case _esi:  return _t("esi");
    case _edi:  return _t("edi");
    }
    return 0;
}

template<class _in>
struct rasm_instr0:
    public _in
{
    virtual void to_string(string& str) const override { print(str, get_name(), _t(";")); }
    virtual bool compare(const rasm_instr* that) const override { return strtool::compare(get_name(), that->get_name()) == 0; }
    inline void print(string& str, const gchar* p, const gchar* a) const
    {
        if(p) { str += p; }
        if(a) { str += a; }
    }
    inline void print(string& str, int n, const gchar* a) const
    {
        string s;
        s.format(_t("%08x"), n);
        str += s;
        if(a) { str += a; }
    }
    inline void print(string& str, reg n, const gchar* a) const
    {
        string s;
        s.format(_t("[%s]"), nameofreg(n));
        str += s;
        if(a) { str += a; }
    }
    inline void print(string& str, _bias8_ n, const gchar* a) const
    {
        string s;
        s.format(_t("[s8:%02x]"), (int)n.inner_data());
        str += s;
        if(a) { str += a; }
    }
    inline void print(string& str, _bias16_ n, const gchar* a) const
    {
        string s;
        s.format(_t("[s16:%04x]"), (int)n.inner_data());
        str += s;
        if(a) { str += a; }
    }
    inline void print(string& str, _bias_ n, const gchar* a) const
    {
        string s;
        s.format(_t("[s:%08x]"), (int)n.inner_data());
        str += s;
        if(a) { str += a; }
    }
    inline void print(string& str, _mem_ n, const gchar* a) const
    {
        string s;
        s.format(_t("[m:%08x]"), (int)n.inner_data());
        str += s;
        if(a) { str += a; }
    }
    inline void print(string& str, _reg_ n, const gchar* a) const
    {
        string s;
        s.format(_t("[%s]"), nameofreg(n.inner_data()));
        str += s;
        if(a) { str += a; }
    }
};

template<class c1>
struct rasm_instr1:
    public rasm_instr0<rasm_instr>
{
    typedef c1  type1;
    typedef rasm_instr1<c1> myref;

    c1              _data1;

    rasm_instr1(c1 a) { _data1 = a; }
    virtual void to_string(string& str) const override
    {
        print(str, get_name(), _t("  "));
        print(str, _data1, _t(";"));
    }
    virtual bool compare(const rasm_instr* that) const override
    {
        if(strtool::compare(get_name(), that->get_name()) != 0)
            return false;
        const myref* ptr = static_cast<const myref*>(that);
        return _data1 == ptr->_data1;
    }
};

struct __gs_novtable rasm_link abstract:
    public rasm_instr
{
    virtual bool is_linker() const = 0;
    virtual const string& get_label() const = 0;
};

struct rasm_linker;

struct rasm_label:
    public rasm_instr0<rasm_link>
{
    typedef rasm_linker linker;
    typedef gs::vector<linker*> linker_list;

    string          _label;
    linker_list     _linkers;

public:
    rasm_label(lab l) { _label.assign(l); }
    virtual const gchar* get_name() const override { return _t("label"); }
    virtual bool finalize(vessel& vsl) override { return true; }
    virtual void to_string(string& str) const override;
    virtual bool compare(const rasm_instr* that) const override;
    virtual bool is_linker() const override { return false; }
    virtual const string& get_label() const override { return _label; }

public:
    int get_linker_ctr() const { return (int)_linkers.size(); }
    rasm_linker* get_linker(int i) { return _linkers.at(i); }
    void add_linker(linker* p) { _linkers.push_back(p); }
};

struct rasm_linker:
    public rasm_instr0<rasm_link>
{
    typedef lab type1;
    typedef rasm_linker linker;
    typedef gs::vector<linker*> linker_list;

    string          _label;
    int             _linkeeid;
    linker_list     _affects;

public:
    rasm_linker(lab l): _linkeeid(0) { _label.assign(l); }
    virtual void to_string(string& str) const override;
    virtual bool compare(const rasm_instr* that) const override;
    virtual bool is_linker() const override { return true; }
    virtual const string& get_label() const override { return _label; }

public:
    void set_linkee_index(int i) { _linkeeid = i; }
    int get_linkee_index() const { return _linkeeid; }
    void add_affects(linker* p) { _affects.push_back(p); }
};

template<class c1, class c2>
struct rasm_instr2:
    public rasm_instr1<c1>
{
    typedef c2  type2;
    typedef rasm_instr2<c1, c2> myref;

    c2              _data2;

    rasm_instr2(c1 a, c2 b): rasm_instr1(a) { _data2 = b; }
    virtual void to_string(string& str) const override
    {
        print(str, get_name(), _t("  "));
        print(str, _data1, _t(","));
        print(str, _data2, _t(";"));
    }
    virtual bool compare(const rasm_instr* that) const override
    {
        if(strtool::compare(get_name(), that->get_name()) != 0)
            return false;
        const myref* ptr = static_cast<const myref*>(that);
        return _data1 == ptr->_data1 && _data2 == ptr->_data2;
    }
};

template<class c1, class c2, class c3>
struct rasm_instr3:
    public rasm_instr2<c1, c2>
{
    typedef c3  type3;
    typedef rasm_instr3<c1, c2, c3> myref;

    c3              _data3;

    rasm_instr3(c1 a, c2 b, c3 c): rasm_instr2(a, b) { _data3 = c; }
    virtual void to_string(string& str) const override
    {
        print(str, get_name(), _t("  "));
        print(str, _data1, _t(","));
        print(str, _data2, _t(","));
        print(str, _data3, _t(";"));
    }
    virtual bool compare(const rasm_instr* that) const override
    {
        if(strtool::compare(get_name(), that->get_name()) != 0)
            return false;
        const myref* ptr = static_cast<const myref*>(that);
        return _data1 == ptr->_data1 && _data2 == ptr->_data2 && _data3 == ptr->_data3;
    }
};

#define rasm_reginstr(in) \
    static rasm_instr* make_##in(); \
    void in() { _list.push_back(make_##in()); }

#define rasm_reginstr_i_begin(in, c1) \
    static rasm_instr* make_##in(c1); \
    void in(c1 a) {

#define rasm_reginstr_i_end(in) \
    _list.push_back(make_##in(a)); }

#define rasm_reginstr_i(in, c1) \
    rasm_reginstr_i_begin(in, c1) \
    rasm_reginstr_i_end(in)

#define rasm_reginstr_ii_begin(in, c1, c2) \
    static rasm_instr* make_##in(c1, c2); \
    void in(c1 a, c2 b) {

#define rasm_reginstr_ii_end(in) \
    _list.push_back(make_##in(a, b)); }

#define rasm_reginstr_ii(in, c1, c2) \
    rasm_reginstr_ii_begin(in, c1, c2) \
    rasm_reginstr_ii_end(in)

#define rasm_reginstr_iii_begin(in, c1, c2, c3) \
    static rasm_instr* make_##in(c1, c2, c3); \
    void in(c1 a, c2 b, c3 c) {

#define rasm_reginstr_iii_end(in) \
    _list.push_back(make_##in(a, b, c)); }

#define rasm_reginstr_iii(in, c1, c2, c3) \
    rasm_reginstr_iii_begin(in, c1, c2, c3) \
    rasm_reginstr_iii_end(in)

#define rasm_reginstr_lab(in, c1) \
    static rasm_instr* make_##in(c1); \
    void in(c1 a) \
    { \
        rasm_label* ptr = static_cast<rasm_label*>(make_##in(a)); \
        assert(ptr && "alloc failed."); \
        _list.push_back(ptr); \
        verify(_label.insert(std::make_pair(ptr->get_label(), get_last_instruction_index())).second && "insert failed."); \
    }

#define rasm_reginstr_lnk(in, c1) \
    static rasm_instr* make_##in(c1); \
    void in(c1 a) \
    { \
        rasm_linker* ptr = static_cast<rasm_linker*>(make_##in(a)); \
        assert(ptr && "alloc failed."); \
        _list.push_back(ptr); \
        _link.push_back(ptr); \
    }

/*
 * Only the 8 - bit offsets need this optimization, but DONOT apply such kind of optimizations to immediate operands,
 * that might cause problems like "mov eax, 1" become "mov ah, 1", which was incorrect
 */
#define rasm_optimize_b8(calling, bparam) \
    if((int)bparam.inner_data() > (int)-0x80 && (int)bparam.inner_data() < (int)0x80) \
        return calling;

#define rasm_optimize_b16(calling, bparam) \
    if((int)bparam.inner_data() > (int)-0x8000 && (int)bparam.inner_data() < (int)0x8000) \
        return calling;

/*
 * If a instruction was operated on the eax register, it could be optimized like:
 */
#define rasm_optimize_acc(calling, rparam) \
    if(rparam == _eax) \
        return calling;

/*
 * Optimize for inc or dec
 */
#define rasm_optimize_i1(calling, iparam) \
    if(iparam == 1) \
        return calling;

/*
 * Optimize for immediate operants less than 1 byte. Instructions like loop need this optimization
 */
#define rasm_optimize_i8(calling, iparam) \
    if((int)iparam > (int)-0x80 && (int)iparam < (int)0x80) \
        return calling;

class rasm
{
public:
    typedef gs::vector<rasm_instr*> instr_list;
    typedef gs::vector<rasm_link*> link_list;
    typedef gs::unordered_map<gs::string, int> label_map;       /* label name => index in instruction list */

public:
    rasm() {}
    ~rasm();
    void print(string& str) const;
    int finalize(vessel& vsl);
    int get_last_instruction_index() const;
    //link get_label(lab l) { return _label.find(l); }
    //link no_label() { return _label.end(); }
    //const_link get_label(lab l) const { return _label.find(l); }
    //const_link no_label() const { return _label.end(); }
    //iterator get_current() { return _list.end(); }
    //const_iterator get_current() const { return _list.end(); }
    //iterator get_head() { return _list.begin(); }
    //const_iterator get_head() const { return _list.begin(); }
    //void attach(const_iterator pos, rasm& other);
    //iterator tick() { return _list.size() ? -- _list.end() : _list.end(); }
    //void tick(iterator& i) { i = tick(); }

protected:
    instr_list      _list;
    label_map       _label;
    link_list       _link;

public:
    rasm_reginstr_ii(mov, reg, int8);
    rasm_reginstr_ii(mov, reg, int);
    rasm_reginstr_ii(mov, reg, reg);
    rasm_reginstr_ii(mov, reg, _mem_);
    rasm_reginstr_ii(mov, reg, _bias8_);
    rasm_reginstr_ii_begin(mov, reg, _bias_)
        rasm_optimize_b8(mov(a, (_bias8_)b.inner_data()), b)
        rasm_reginstr_ii_end(mov);
    rasm_reginstr_ii(mov, _mem_, reg);
    rasm_reginstr_ii(mov, _bias8_, reg);
    rasm_reginstr_ii_begin(mov, _bias_, reg)
        rasm_optimize_b8(mov((_bias8_)a.inner_data(), b), a)
        rasm_reginstr_ii_end(mov);
    rasm_reginstr_ii(mov, _mem_, int);
    rasm_reginstr_ii(mov, _bias8_, int);
    rasm_reginstr_ii_begin(mov, _bias_, int)
        rasm_optimize_b8(mov((_bias8_)a.inner_data(), b), a)
        rasm_reginstr_ii_end(mov);
    rasm_reginstr_ii(mov, reg, _reg_);
    rasm_reginstr_ii(mov, _reg_, reg);
    rasm_reginstr_ii(lea, reg, _bias8_);
    rasm_reginstr_ii_begin(lea, reg, _bias_)
        rasm_optimize_b8(lea(a, (_bias8_)b.inner_data()), b)
        rasm_reginstr_ii_end(lea);
    rasm_reginstr(nop);
    rasm_reginstr(hlt);
    rasm_reginstr_i(call, int);
    rasm_reginstr_i(call, reg);
    rasm_reginstr_i(call, _bias_);
    rasm_reginstr_lnk(call, lab);
    rasm_reginstr(ret);
    rasm_reginstr_i(ret, int);
    rasm_reginstr_i(push, reg);
    rasm_reginstr_i(push, int8);
    rasm_reginstr_i(push, int);
    rasm_reginstr_i(push, _mem_);
    rasm_reginstr_i(push, _bias8_);
    rasm_reginstr_i_begin(push, _bias_)
        rasm_optimize_b8(push((_bias8_)a.inner_data()), a)
        rasm_reginstr_i_end(push);
    rasm_reginstr_i(push, _reg_);
    rasm_reginstr_i(pushw, _bias8_);
    rasm_reginstr_i_begin(pushw, _bias_)
        rasm_optimize_b8(pushw((_bias8_)a.inner_data()), a)
        rasm_reginstr_i_end(pushw);
    rasm_reginstr(pushad);
    rasm_reginstr_i(pop, reg);
    rasm_reginstr_i(pop, _mem_);
    rasm_reginstr_i(pop, _bias8_);
    rasm_reginstr_i_begin(pop, _bias_)
        rasm_optimize_b8(pop((_bias8_)a.inner_data()), a)
        rasm_reginstr_i_end(pop);
    rasm_reginstr(popad);
    rasm_reginstr_lab(label, lab);
    rasm_reginstr_i(jmp, _mem_);
    rasm_reginstr_i(jmp, _bias8_);
    rasm_reginstr_i_begin(jmp, _bias_)
        rasm_optimize_b8(jmp((_bias8_)a.inner_data()), a)
        rasm_reginstr_i_end(jmp);
    rasm_reginstr_i(jmp, reg);
    rasm_reginstr_i(jmps, _bias8_);         /* jmp bias from ss */
    rasm_reginstr_i_begin(jmps, _bias_)
        rasm_optimize_b8(jmps((_bias8_)a.inner_data()), a)
        rasm_reginstr_i_end(jmps);
    //rasm_reginstr_i(jmp, lab);
    rasm_reginstr_lnk(jmp, lab);
    rasm_reginstr_i(jes, _bias8_);
    rasm_reginstr_i_begin(jes, _bias_)
        rasm_optimize_b8(jes((_bias8_)a.inner_data()), a)
        rasm_reginstr_i_end(jes);
    rasm_reginstr_i(jnes, _bias8_);
    rasm_reginstr_i_begin(jnes, _bias_)
        rasm_optimize_b8(jnes((_bias8_)a.inner_data()), a)
        rasm_reginstr_i_end(jnes);
    rasm_reginstr_i(loop, int8);
    rasm_reginstr_i_begin(loop, int)        /* I'm not sure about this, can the loop instruction operate on a short? */
        rasm_optimize_i8(loop((int8)a), (a - 2))
        rasm_reginstr_i_end(loop);
    rasm_reginstr_lnk(loop, lab);
    rasm_reginstr(cld);
    rasm_reginstr(std);
    rasm_reginstr(movsb);
    rasm_reginstr(movsw);
    rasm_reginstr(movs);
    rasm_reginstr(rep);
    rasm_reginstr_ii(cmp, _bias_, int);
    rasm_reginstr_ii(cmp, reg, int8);
    rasm_reginstr_ii(cmp, reg, int);
    rasm_reginstr_ii(cmp, reg, _bias8_);
    rasm_reginstr_ii_begin(cmp, reg, _bias_)
        rasm_optimize_b8(cmp(a, (_bias8_)b.inner_data()), b)
        rasm_reginstr_ii_end(cmp);
    rasm_reginstr_ii(cmp, _reg_, int8);
    rasm_reginstr_ii(cmp, _reg_, int);
    rasm_reginstr_ii(cmp, reg, _reg_);
    rasm_reginstr_i(inc, reg);
    rasm_reginstr_i(inc, _mem_);
    rasm_reginstr_i(inc, _bias8_);
    rasm_reginstr_i_begin(inc, _bias_)
        rasm_optimize_b8(inc((_bias8_)a.inner_data()), a)
        rasm_reginstr_i_end(inc);
    rasm_reginstr_ii(add, reg, reg);
    rasm_reginstr_ii(add, reg, int8);
    rasm_reginstr_ii(add, reg, int);
    rasm_reginstr_i(add, int);
    rasm_reginstr_ii(add, reg, _mem_);
    rasm_reginstr_ii(add, reg, _bias8_);
    rasm_reginstr_ii(add, reg, _bias_);
    rasm_reginstr_ii(add, _mem_, int8);
    rasm_reginstr_ii(add, _mem_, int);
    rasm_reginstr_ii(add, _bias8_, int8);
    rasm_reginstr_ii(add, _bias_, int8);
    rasm_reginstr_ii(add, _bias8_, int);
    rasm_reginstr_ii(add, _bias_, int);
    rasm_reginstr_ii(add, _mem_, reg);
    rasm_reginstr_ii(add, _bias8_, reg);
    rasm_reginstr_ii(add, _bias_, reg);
    rasm_reginstr_ii(add, reg, _reg_);
    rasm_reginstr_ii(add, _reg_, reg);
    rasm_reginstr_i(dec, reg);
    rasm_reginstr_i(dec, _mem_);
    rasm_reginstr_i(dec, _bias_);
    rasm_reginstr_ii(sub, reg, reg);
    rasm_reginstr_ii(sub, reg, int);
    rasm_reginstr_i(sub, int);
    rasm_reginstr_ii(sub, reg, _mem_);
    rasm_reginstr_ii(sub, reg, _bias_);
    rasm_reginstr_ii(sub, _mem_, int);
    rasm_reginstr_ii(sub, _bias_, int);
    rasm_reginstr_ii(sub, _mem_, reg);
    rasm_reginstr_ii(sub, _bias_, reg);
    rasm_reginstr_ii(sub, reg, _reg_);
    rasm_reginstr_ii(sub, _reg_, reg);
    rasm_reginstr_iii(imul, reg, reg, int);
    rasm_reginstr_ii(imul, reg, int);
    rasm_reginstr_i(imul, reg);
    rasm_reginstr_ii(imul, reg, _mem_);
    rasm_reginstr_ii(imul, reg, _bias_);
    rasm_reginstr_iii(imul, reg, _mem_, int);
    rasm_reginstr_iii(imul, reg, _bias_, int);
    rasm_reginstr_ii(imul, reg, reg);
    rasm_reginstr_i(imul, _mem_);
    rasm_reginstr_i(imul, _bias_);
    rasm_reginstr_ii(imul, reg, _reg_);
    rasm_reginstr_i(idiv, reg);
    rasm_reginstr_i(idiv, _mem_);
    rasm_reginstr_i(idiv, _bias_);
    rasm_reginstr_i(idiv, _reg_);
    rasm_reginstr(cdq);
    rasm_reginstr_i(neg, reg);
    rasm_reginstr_i(not, reg);
    rasm_reginstr_ii(xor, reg, reg);
    rasm_reginstr_ii(xor, reg, int);
    rasm_reginstr_ii(xor, reg, _bias_);
    rasm_reginstr_ii(xor, reg, _reg_);
    rasm_reginstr_i(shl, reg);
    rasm_reginstr_ii(shl, reg, int);
    rasm_reginstr_i(shr, reg);
    rasm_reginstr_ii(shr, reg, int);
    //rasm_reginstr
    //setcc
    rasm_reginstr_ii(and, reg, int);
    rasm_reginstr_ii(and, reg, _bias_);
    rasm_reginstr_ii(and, reg, _reg_);
    rasm_reginstr_ii(or , reg, int);
    rasm_reginstr_ii(or , reg, _bias_);
    rasm_reginstr_ii(or , reg, _reg_);
};

#endif

__rathen_end__

#endif