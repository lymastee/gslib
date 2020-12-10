/*
 * Copyright (c) 2016-2018 lymastee, All rights reserved.
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

#include <rathen\rasm.h>

__rathen_begin__

/*
 * Represents for:
 * r - register
 * i - immediate
 * m - memory
 * b - bias
 */
static const byte
    __rasm_hdr_mov_r_i8[] = { 0xb0 },
    __rasm_hdr_mov_r_i[] = { 0xb8 },
    __rasm_hdr_mov_r_r[] = { 0x8b, 0xc0 },
    __rasm_hdr_mov_r_m[] = { 0x8b, 0x05 },
    __rasm_hdr_mov_r_b8[] = { 0x8b, 0x45 },
    __rasm_hdr_mov_r_b[] = { 0x8b, 0x85 },
    __rasm_hdr_mov_m_r[] = { 0x89, 0x05 },
    __rasm_hdr_mov_b8_r[] = { 0x89, 0x45 },
    __rasm_hdr_mov_b_r[] = { 0x89, 0x85 },
    __rasm_hdr_mov_m_i[] = { 0xc7, 0x05 },
    __rasm_hdr_mov_b8_i[] = { 0xc7, 0x45 },
    __rasm_hdr_mov_b_i[] = { 0xc7, 0x85 },
    __rasm_hdr_movra[] = { 0x8b, 0x00 },
    __rasm_hdr_movar[] = { 0x89, 0x00 },
    __rasm_hdr_lea_r_b8[] = { 0x8d, 0x45 },
    __rasm_hdr_lea_r_b[] = { 0x8d, 0x85 },
    __rasm_hdr_nop[] = { 0x90 },
    __rasm_hdr_hlt[] = { 0xf4 },
    __rasm_hdr_call_m[] = { 0xff, 0x1d },
    __rasm_hdr_call_r[] = { 0xff, 0xd0 },
    __rasm_hdr_calls_b[] = { 0xe8 },
    __rasm_hdr_ret[] = { 0xc3 },
    __rasm_hdr_ret_i[] = { 0xc2 },
    __rasm_hdr_push_r[] = { 0x50 },
    __rasm_hdr_push_m[] = { 0xff, 0x35 },
    __rasm_hdr_push_b8[] = { 0xff, 0x75 },
    __rasm_hdr_push_b[] = { 0xff, 0xb5 },
    __rasm_hdr_push_i8[] = { 0x6a },
    __rasm_hdr_push_i[] = { 0x68 },
    __rasm_hdr_pusha_r[] = { 0xff, 0x30 }, /* push [eax], like this */
    __rasm_hdr_pushw_b8[] = { 0x66, 0xff, 0x75 },
    __rasm_hdr_pushw_b[] = { 0x66, 0xff, 0xb5 },
    __rasm_hdr_pushad[] = { 0x60 },
    __rasm_hdr_pop_r[] = { 0x58 },
    __rasm_hdr_pop_m[] = { 0x8f, 0x05 },
    __rasm_hdr_pop_b8[] = { 0x8f, 0x45 },
    __rasm_hdr_pop_b[] = { 0x8f, 0x85 },
    __rasm_hdr_popad[] = { 0x61 },
    __rasm_hdr_jmp_m[] = { 0xff, 0x25 },
    __rasm_hdr_jmp_b8[] = { 0xff, 0x65 },
    __rasm_hdr_jmp_b[] = { 0xff, 0xa5 },
    __rasm_hdr_jmp_r[] = { 0xff, 0xe0 },
    __rasm_hdr_jmps_b8[] = { 0xeb },
    __rasm_hdr_jmps_b[] = { 0xe9 },
    __rasm_hdr_jes_b8[] = { 0x74 },
    __rasm_hdr_jes_b[] = { 0x0f, 0x84 },
    __rasm_hdr_jnes_b8[] = { 0x75 },
    __rasm_hdr_jnes_b[] = { 0x0f, 0x85 },
    __rasm_hdr_loop[] = { 0xe2 },
    __rasm_hdr_cld[] = { 0xfc },
    __rasm_hdr_std[] = { 0xfd },
    __rasm_hdr_movsb[] = { 0xa4 },
    __rasm_hdr_movsw[] = { 0x66, 0xa5 },
    __rasm_hdr_movs[] = { 0xa5 },   /* movsd in 486+ */
    __rasm_hdr_rep[] = { 0xf3 },
    __rasm_hdr_cmp_b_i[] = { 0x81, 0x3d },
    __rasm_hdr_cmp_r_i8[] = { 0x83, 0xf8 },
    __rasm_hdr_cmp_r_i[] = { 0x81, 0xf8 },
    __rasm_hdr_cmp_r_b8[] = { 0x3b, 0x45 },
    __rasm_hdr_cmp_r_b[] = { 0x3b, 0x85 },
    __rasm_hdr_cmpa_r_i8[] = { 0x80, 0x38 },
    __rasm_hdr_cmpa_r_i[] = { 0x81, 0x38 },
    __rasm_hdr_cmpra_r_r[] = { 0x3b, 0x00 },
    __rasm_hdr_add_r_r[] = { 0x03, 0xc0 },
    __rasm_hdr_add_r_i8[] = { 0x83, 0xc0 },
    __rasm_hdr_add_acc[] = { 0x05 },
    __rasm_hdr_inc_r[] = { 0x40 },
    __rasm_hdr_add_r_i[] = { 0x81, 0xc0 },
    __rasm_hdr_add_r_m[] = { 0x03, 0x05 },
    __rasm_hdr_add_r_b8[] = { 0x03, 0x45 },
    __rasm_hdr_add_r_b[] = { 0x03, 0x85 },
    __rasm_hdr_add_m_i8[] = { 0x83, 0x05 },
    __rasm_hdr_inc_m[] = { 0xff, 0x05 },
    __rasm_hdr_add_m_i[] = { 0x81, 0x05 },
    __rasm_hdr_add_b8_i8[] = { 0x83, 0x45 },
    __rasm_hdr_add_b_i8[] = { 0x83, 0x85 },
    __rasm_hdr_inc_b8[] = { 0xff, 0x45 },
    __rasm_hdr_inc_b[] = { 0xff, 0x85 },
    __rasm_hdr_add_b8_i[] = { 0x81, 0x45 },
    __rasm_hdr_add_b_i[] = { 0x81, 0x85 },
    __rasm_hdr_add_m_r[] = { 0x01, 0x05 },
    __rasm_hdr_add_b8_r[] = { 0x01, 0x45 },
    __rasm_hdr_add_b_r[] = { 0x01, 0x85 },
    __rasm_hdr_addra[] = { 0x03, 0x00 },
    __rasm_hdr_addar[] = { 0x01, 0x00 },
    __rasm_hdr_sub_r_r[] = { 0x2b, 0xc0 },
    __rasm_hdr_sub_r_i8[] = { 0x83, 0xe8 },
    __rasm_hdr_sub_acc[] = { 0x2d },
    __rasm_hdr_dec_r[] = { 0x48 },
    __rasm_hdr_sub_r_i[] = { 0x81, 0xe8 },
    __rasm_hdr_sub_r_m[] = { 0x2b, 0x05 },
    __rasm_hdr_sub_r_b8[] = { 0x2b, 0x45 },
    __rasm_hdr_sub_r_b[] = { 0x2b, 0x85 },
    __rasm_hdr_sub_m_i8[] = { 0x83, 0x2d },
    __rasm_hdr_dec_m[] = { 0xff, 0x0d },
    __rasm_hdr_sub_m_i[] = { 0x81, 0x2d },
    __rasm_hdr_sub_b8_i8[] = { 0x83, 0x6d },
    __rasm_hdr_sub_b_i8[] = { 0x83, 0xad },
    __rasm_hdr_dec_b8[] = { 0xff, 0x4d },
    __rasm_hdr_dec_b[] = { 0xff, 0x8d },
    __rasm_hdr_sub_b8_i[] = { 0x81, 0x6d },
    __rasm_hdr_sub_b_i[] = { 0x81, 0xad },
    __rasm_hdr_sub_m_r[] = { 0x29, 0x05 },
    __rasm_hdr_sub_b8_r[] = { 0x29, 0x45 },
    __rasm_hdr_sub_b_r[] = { 0x29, 0x85 },
    __rasm_hdr_subra[] = { 0x2b, 0x00 },
    __rasm_hdr_subar[] = { 0x29, 0x00 },
    __rasm_hdr_imul_r_r_i8[] = { 0x6b, 0xc0 },
    /* __rasm_hdr_imul_r_i8[] = {}, */
    __rasm_hdr_imul_r_r_i[] = { 0x69, 0xc0 },
    /* __rasm_hdr_imul_r_i[] = {}, */
    __rasm_hdr_imul_r[] = { 0xf7, 0xe8 },
    __rasm_hdr_imul_r_m[] = { 0x0f, 0xaf, 0x05 },
    __rasm_hdr_imul_r_b8[] = { 0x0f, 0xaf, 0x45 },
    __rasm_hdr_imul_r_b[] = { 0x0f, 0xaf, 0x85 },
    __rasm_hdr_imul_r_m_i8[] = { 0x6b, 0x05 },
    __rasm_hdr_imul_r_m_i[] = { 0x69, 0x05 },
    __rasm_hdr_imul_r_b8_i8[] = { 0x6b, 0x45 },
    __rasm_hdr_imul_r_b8_i[] = { 0x69, 0x45 },
    __rasm_hdr_imul_r_b_i8[] = { 0x6b, 0x85 },
    __rasm_hdr_imul_r_b_i[] = { 0x69, 0x85 },
    __rasm_hdr_imul_r_r[] = { 0x0f, 0xaf, 0xc0 },
    __rasm_hdr_imul_m[] = { 0xf7, 0x2d },
    __rasm_hdr_imul_b8[] = { 0xf7, 0x6d },
    __rasm_hdr_imul_b[] = { 0xf7, 0xad },
    __rasm_hdr_imulra[] = { 0x0f, 0xaf, 0x00 },
    __rasm_hdr_idiv_r[] = { 0xf7, 0xf8 },
    __rasm_hdr_idiv_m[] = { 0xf7, 0x3d },
    __rasm_hdr_idiv_b8[] = { 0xf7, 0x7d },
    __rasm_hdr_idiv_b[] = { 0xf7, 0xbd },
    __rasm_hdr_idiva[] = { 0xf6, 0x38 },
    __rasm_hdr_cdq[] = { 0x99 },
    __rasm_hdr_neg_r[] = { 0xf7, 0xd8 },
    __rasm_hdr_not_r[] = { 0xf7, 0xd0 },
    __rasm_hdr_xor_r_r[] = { 0x33, 0xc0 },
    __rasm_hdr_xor_r_i8[] = { 0x83, 0xf0 },
    __rasm_hdr_xor_r_i[] = { 0x81, 0xf0 },
    __rasm_hdr_xor_r_b8[] = { 0x33, 0x45 },
    __rasm_hdr_xor_r_b[] = { 0x33, 0x85 },
    __rasm_hdr_xorra[] = { 0x33, 0x00 },
    __rasm_hdr_shl_r[] = { 0xd3, 0xe0 },
    __rasm_hdr_shl_r_i[] = { 0xc1, 0xe0 },
    __rasm_hdr_shr_r[] = { 0xd3, 0xe8 },
    __rasm_hdr_shr_r_i[] = { 0xc1, 0xe8 },
    __rasm_hdr_set_r[] = { 0x0f, 0x90, 0xc0 },
    __rasm_hdr_and_r_i8[] = { 0x83, 0xe0 },
    __rasm_hdr_and_r_i[] = { 0x81, 0xe0 },
    __rasm_hdr_and_r_b8[] = { 0x23, 0x45 },
    __rasm_hdr_and_r_b[] = { 0x23, 0x85 },
    __rasm_hdr_andra[] = { 0x23, 0x00 },
    __rasm_hdr_or_r_i8[] = { 0x83, 0xc8 },
    __rasm_hdr_or_r_i[] = { 0x81, 0xc8 },
    __rasm_hdr_or_r_b8[] = { 0x0b, 0x45 },
    __rasm_hdr_or_r_b[] = { 0x0b, 0x85 },
    __rasm_hdr_orra[] = { 0x0b, 0x00 };

#define rasm_announce(hdr) \
    virtual const gchar* get_name() const { return _t(#hdr); }

#define rasm_announce_i(hdr) \
    rasm_##hdr(type1 d1): rasm_instr1(d1) {} \
    virtual const gchar* get_name() const { return _t(#hdr); }

#define rasm_announce_i_lnk(hdr) \
    rasm_##hdr(type1 d1): rasm_linker(d1) {} \
    virtual const gchar* get_name() const { return _t(#hdr); }

#define rasm_announce_ii(hdr) \
    rasm_##hdr(type1 d1, type2 d2): rasm_instr2(d1, d2) {} \
    virtual const gchar* get_name() const { return _t(#hdr); }

#define rasm_announce_iii(hdr) \
    rasm_##hdr(type1 d1, type2 d2, type3 d3): rasm_instr3(d1, d2, d3) {} \
    virtual const gchar* get_name() const { return _t(#hdr); }


#define rasm_impinstr(in) \
    rasm_instr* rasm::make_##in() { \
        return gs_new(rasm_##in); \
    }

#define rasm_impinstr_l(in, c1) \
    rasm_instr* rasm::make_##in(c1 a) { \
        typedef rasm_##in cst; \
        return gs_new(cst, a); \
    }

#define rasm_impinstr_x(in) \
    rasm_instr* rasm::make_##in() { \
        typedef rasm_##in<void> cst; \
        return gs_new(cst); \
    }

#define rasm_impinstr_i(in, c1) \
    rasm_instr* rasm::make_##in(c1 a) { \
        typedef rasm_##in<c1> cst; \
        return gs_new(cst, a); \
    }

#define rasm_impinstr_ii(in, c1, c2) \
    rasm_instr* rasm::make_##in(c1 a, c2 b) { \
        typedef rasm_##in<c1, c2> cst; \
        return gs_new(cst, a, b); \
    }

#define rasm_impinstr_ix(in, c1) \
    rasm_instr* rasm::make_##in(c1 a) { \
        typedef rasm_##in<c1, void> cst; \
        return gs_new(cst, a); \
    }

#define rasm_impinstr_iii(in, c1, c2, c3) \
    rasm_instr* rasm::make_##in(c1 a, c2 b, c3 c) { \
        typedef rasm_##in<c1, c2, c3> cst; \
        return gs_new(cst, a, b, c); \
    }

#define rasm_impinstr_iix(in, c1, c2) \
    rasm_instr* rasm::make_##in(c1 a, c2 b) { \
        typedef rasm_##in<c1, c2, void> cst; \
        return gs_new(cst, a, b); \
    }

#define rasm_impinstr_ixx(in, c1) \
    rasm_instr* rasm::make_##in(c1 a) { \
        typedef rasm_##in<c1, void, void> cst; \
        return gs_new(cst, a); \
    }

static inline void rasm_set_header(vessel& vsl, const byte buf[], int size)
{
    vsl.store((const void*)buf, size);
}

static inline void rasm_set_register(vessel& vsl, reg r)
{
    vsl.current<byte>(-1) |= get_register_index(r);
}

static inline void rasm_set_register(vessel& vsl, reg r, int shift_left)
{
    vsl.current<byte>(-1) |= (get_register_index(r) << shift_left);
}

template<class tpl>
static inline void rasm_append_data(vessel& vsl, tpl x)
{
    vsl.expand(sizeof(tpl));
    vsl.current<tpl>() = x;
    vsl.occupy(sizeof(tpl));
}

void rasm_label::to_string(string& str) const
{
    print(str, get_name(), _t("  "));
    print(str, _label.c_str(), _t(";"));
}

bool rasm_label::compare(const rasm_instr* that) const
{
    if(strtool::compare(get_name(), that->get_name()) != 0)
        return false;
    const rasm_label* ptr = static_cast<const rasm_label*>(that);
    return _label == ptr->_label;
}

void rasm_linker::to_string(string& str) const
{
    print(str, get_name(), _t("  "));
    print(str, _label.c_str(), _t(";"));
}

bool rasm_linker::compare(const rasm_instr* that) const
{
    if(strtool::compare(get_name(), that->get_name()) != 0)
        return false;
    const rasm_linker* ptr = static_cast<const rasm_linker*>(that);
    return _label == ptr->_label;
}

// struct rasm_label:
//     public rasm_instr0<rasm_linkee>
// {
//     typedef rasm_linker666 linker;
//     typedef gs::vector<linker*> linker_list;
// 
//     string  _label;
//     linker_list _list;
// 
//     rasm_label(lab l) { _label.assign(l); }
//     virtual const gchar* get_name() const override { return _t("label"); }
//     virtual bool finalize(vessel& vsl) override { return true; }
//     virtual void to_string(string& str) const override
//     {
//         print(str, get_name(), _t("  "));
//         print(str, _label.c_str(), _t(";"));
//     }
//     virtual bool compare(const rasm_instr* that) const override
//     {
//         if(strtool::compare(get_name(), that->get_name()) != 0)
//             return false;
//         const rasm_label* ptr = static_cast<const rasm_label*>(that);
//         return _label == ptr->_label;
//     }
//     virtual const string& get_label() const override { return _label; }
//     virtual int get_linker_ctr() const override { return (int)_list.size(); }
//     virtual rasm_instr* get_linker(int idx) override { return _list.at(idx); }
// };

template<class c1, class c2>
struct rasm_mov {};

template<>
struct rasm_mov<reg, int8>:
    public rasm_instr2<reg, int8>
{
    rasm_announce_ii(mov);
    virtual bool finalize(vessel& vsl) override
    {
        rasm_set_header(vsl, __rasm_hdr_mov_r_i8, sizeof(__rasm_hdr_mov_r_i8));
        rasm_set_register(vsl, _data1);
        rasm_append_data(vsl, _data2);
        return true;
    }
};

template<>
struct rasm_mov<reg, int>:
    public rasm_instr2<reg, int>
{
    rasm_announce_ii(mov);
    virtual bool finalize(vessel& vsl) override
    {
        rasm_set_header(vsl, __rasm_hdr_mov_r_i, sizeof(__rasm_hdr_mov_r_i));
        rasm_set_register(vsl, _data1);
        rasm_append_data(vsl, _data2);
        return true;
    }
};

template<>
struct rasm_mov<reg, reg>:
    public rasm_instr2<reg, reg>
{
    rasm_announce_ii(mov);
    virtual bool finalize(vessel& vsl) override
    {
        rasm_set_header(vsl, __rasm_hdr_mov_r_r, sizeof(__rasm_hdr_mov_r_r));
        rasm_set_register(vsl, _data1, 3);
        rasm_set_register(vsl, _data2);
        return true;
    }
};

template<>
struct rasm_mov<reg, _mem_>:
    public rasm_instr2<reg, _mem_>
{
    rasm_announce_ii(mov);
    virtual bool finalize(vessel& vsl) override
    {
        rasm_set_header(vsl, __rasm_hdr_mov_r_m, sizeof(__rasm_hdr_mov_r_m));
        rasm_set_register(vsl, _data1, 3);
        rasm_append_data(vsl, _data2);
        return true;
    }
};

template<>
struct rasm_mov<reg, _bias8_>:
    public rasm_instr2<reg, _bias8_>
{
    rasm_announce_ii(mov);
    virtual bool finalize(vessel& vsl) override
    {
        rasm_set_header(vsl, __rasm_hdr_mov_r_b8, sizeof(__rasm_hdr_mov_r_b8));
        rasm_set_register(vsl, _data1, 3);
        rasm_append_data(vsl, _data2);
        return true;
    }
};

template<>
struct rasm_mov<reg, _bias_>:
    public rasm_instr2<reg, _bias_>
{
    rasm_announce_ii(mov);
    virtual bool finalize(vessel& vsl) override
    {
        rasm_set_header(vsl, __rasm_hdr_mov_r_b, sizeof(__rasm_hdr_mov_r_b));
        rasm_set_register(vsl, _data1, 3);
        rasm_append_data(vsl, _data2);
        return true;
    }
};

template<>
struct rasm_mov<_mem_, reg>:
    public rasm_instr2<_mem_, reg>
{
    rasm_announce_ii(mov);
    virtual bool finalize(vessel& vsl) override
    {
        rasm_set_header(vsl, __rasm_hdr_mov_m_r, sizeof(__rasm_hdr_mov_m_r));
        rasm_set_register(vsl, _data2, 3);
        rasm_append_data(vsl, _data1);
        return true;
    }
};

template<>
struct rasm_mov<_bias8_, reg>:
    public rasm_instr2<_bias8_, reg>
{
    rasm_announce_ii(mov);
    virtual bool finalize(vessel& vsl) override
    {
        rasm_set_header(vsl, __rasm_hdr_mov_b8_r, sizeof(__rasm_hdr_mov_b8_r));
        rasm_set_register(vsl, _data2, 3);
        rasm_append_data(vsl, _data1);
        return true;
    }
};

template<>
struct rasm_mov<_bias_, reg>:
    public rasm_instr2<_bias_, reg>
{
    rasm_announce_ii(mov);
    virtual bool finalize(vessel& vsl) override
    {
        rasm_set_header(vsl, __rasm_hdr_mov_b_r, sizeof(__rasm_hdr_mov_b_r));
        rasm_set_register(vsl, _data2, 3);
        rasm_append_data(vsl, _data1);
        return true;
    }
};

template<>
struct rasm_mov<_mem_, int>:
    public rasm_instr2<_mem_, int>
{
    rasm_announce_ii(mov);
    virtual bool finalize(vessel& vsl) override
    {
        rasm_set_header(vsl, __rasm_hdr_mov_m_i, sizeof(__rasm_hdr_mov_m_i));
        rasm_append_data(vsl, _data1);
        rasm_append_data(vsl, _data2);
        return true;
    }
};

template<>
struct rasm_mov<_bias8_, int>:
    public rasm_instr2<_bias8_, int>
{
    rasm_announce_ii(mov);
    virtual bool finalize(vessel& vsl) override
    {
        rasm_set_header(vsl, __rasm_hdr_mov_b8_i, sizeof(__rasm_hdr_mov_b8_i));
        rasm_append_data(vsl, _data1);
        rasm_append_data(vsl, _data2);
        return true;
    }
};

template<>
struct rasm_mov<_bias_, int>:
    public rasm_instr2<_bias_, int>
{
    rasm_announce_ii(mov);
    virtual bool finalize(vessel& vsl) override
    {
        rasm_set_header(vsl, __rasm_hdr_mov_b_i, sizeof(__rasm_hdr_mov_b_i));
        rasm_append_data(vsl, _data1);
        rasm_append_data(vsl, _data2);
        return true;
    }
};

template<>
struct rasm_mov<reg, _reg_>:
    public rasm_instr2<reg, _reg_>
{
    rasm_announce_ii(mov);
    virtual bool finalize(vessel& vsl) override
    {
        rasm_set_header(vsl, __rasm_hdr_movra, sizeof(__rasm_hdr_movra));
        rasm_set_register(vsl, _data1, 3);
        rasm_set_register(vsl, _data2.inner_data());
        return true;
    }
};

template<>
struct rasm_mov<_reg_, reg>:
    public rasm_instr2<_reg_, reg>
{
    rasm_announce_ii(mov);
    virtual bool finalize(vessel& vsl) override
    {
        rasm_set_header(vsl, __rasm_hdr_movar, sizeof(__rasm_hdr_movar));
        rasm_set_register(vsl, _data1.inner_data());
        rasm_set_register(vsl, _data2, 3);
        return true;
    }
};

template<class c1, class c2>
struct rasm_lea {};

template<>
struct rasm_lea<reg, _bias8_>:
    public rasm_instr2<reg, _bias8_>
{
    rasm_announce_ii(lea);
    virtual bool finalize(vessel& vsl) override
    {
        rasm_set_header(vsl, __rasm_hdr_lea_r_b8, sizeof(__rasm_hdr_lea_r_b8));
        rasm_set_register(vsl, _data1, 3);
        rasm_append_data(vsl, _data2);
        return true;
    }
};

template<>
struct rasm_lea<reg, _bias_>:
    public rasm_instr2<reg, _bias_>
{
    rasm_announce_ii(lea);
    virtual bool finalize(vessel& vsl) override
    {
        rasm_set_header(vsl, __rasm_hdr_lea_r_b, sizeof(__rasm_hdr_lea_r_b));
        rasm_set_register(vsl, _data1, 3);
        rasm_append_data(vsl, _data2);
        return true;
    }
};

struct rasm_nop:
    public rasm_instr0<rasm_instr>
{
    rasm_announce(nop);
    virtual bool finalize(vessel& vsl) override
    {
        rasm_set_header(vsl, __rasm_hdr_nop, sizeof(__rasm_hdr_nop));
        return true;
    }
};

struct rasm_hlt:
    public rasm_instr0<rasm_instr>
{
    rasm_announce(hlt);
    virtual bool finalize(vessel& vsl) override
    {
        rasm_set_header(vsl, __rasm_hdr_hlt, sizeof(__rasm_hdr_hlt));
        return true;
    }
};

template<class c1>
struct rasm_call {};

template<>
struct rasm_call<int>:
    public rasm_instr1<int>
{
    rasm_announce_i(call);
    virtual bool finalize(vessel& vsl) override
    {
        rasm_set_header(vsl, __rasm_hdr_call_m, sizeof(__rasm_hdr_call_m));
        rasm_append_data(vsl, _data1);
        return true;
    }
};

template<>
struct rasm_call<reg>:
    public rasm_instr1<reg>
{
    rasm_announce_i(call);
    virtual bool finalize(vessel& vsl) override
    {
        rasm_set_header(vsl, __rasm_hdr_call_r, sizeof(__rasm_hdr_call_r));
        rasm_set_register(vsl, _data1);
        return true;
    }
};

template<>
struct rasm_call<_bias_>:
    public rasm_instr1<_bias_>
{
    rasm_announce_i(call);
    virtual bool finalize(vessel& vsl) override
    {
        rasm_set_header(vsl, __rasm_hdr_calls_b, sizeof(__rasm_hdr_calls_b));
        rasm_append_data(vsl, _data1);
        return true;
    }
};

template<>
struct rasm_call<lab>:
    public rasm_linker
{
    rasm_announce_i_lnk(call);
    virtual bool finalize(vessel& vsl) override
    {
        // todo:
        return true;
    }
};

template<class c1>
struct rasm_ret {};

template<>
struct rasm_ret<void>:
    public rasm_instr0<rasm_instr>
{
    rasm_announce(ret);
    virtual bool finalize(vessel& vsl) override
    {
        rasm_set_header(vsl, __rasm_hdr_ret, sizeof(__rasm_hdr_ret));
        return true;
    }
};

template<>
struct rasm_ret<int>:
    public rasm_instr1<int>
{
    rasm_announce_i(ret);
    virtual bool finalize(vessel& vsl) override
    {
        rasm_set_header(vsl, __rasm_hdr_ret_i, sizeof(__rasm_hdr_ret_i));
        rasm_append_data(vsl, _data1);
        return true;
    }
};

template<class c1>
struct rasm_push {};

template<>
struct rasm_push<reg>:
    public rasm_instr1<reg>
{
    rasm_announce_i(push);
    virtual bool finalize(vessel& vsl) override
    {
        rasm_set_header(vsl, __rasm_hdr_push_r, sizeof(__rasm_hdr_push_r));
        rasm_set_register(vsl, _data1);
        return true;
    }
};

template<>
struct rasm_push<int8>:
    public rasm_instr1<int8>
{
    rasm_announce_i(push);
    virtual bool finalize(vessel& vsl) override
    {
        rasm_set_header(vsl, __rasm_hdr_push_i8, sizeof(__rasm_hdr_push_i8));
        rasm_append_data(vsl, _data1);
        return true;
    }
};

template<>
struct rasm_push<int>:
    public rasm_instr1<int>
{
    rasm_announce_i(push);
    virtual bool finalize(vessel& vsl) override
    {
        rasm_set_header(vsl, __rasm_hdr_push_i, sizeof(__rasm_hdr_push_i));
        rasm_append_data(vsl, _data1);
        return true;
    }
};

template<>
struct rasm_push<_mem_>:
    public rasm_instr1<_mem_>
{
    rasm_announce_i(push);
    virtual bool finalize(vessel& vsl) override
    {
        rasm_set_header(vsl, __rasm_hdr_push_m, sizeof(__rasm_hdr_push_m));
        rasm_append_data(vsl, _data1);
        return true;
    }
};

template<>
struct rasm_push<_bias8_>:
    public rasm_instr1<_bias8_>
{
    rasm_announce_i(push);
    virtual bool finalize(vessel& vsl) override
    {
        rasm_set_header(vsl, __rasm_hdr_push_b8, sizeof(__rasm_hdr_push_b8));
        rasm_append_data(vsl, _data1);
        return true;
    }
};

template<>
struct rasm_push<_bias_>:
    public rasm_instr1<_bias_>
{
    rasm_announce_i(push);
    virtual bool finalize(vessel& vsl) override
    {
        rasm_set_header(vsl, __rasm_hdr_push_b, sizeof(__rasm_hdr_push_b));
        rasm_append_data(vsl, _data1);
        return true;
    }
};

template<>
struct rasm_push<_reg_>:
    public rasm_instr1<_reg_>
{
    rasm_announce_i(push);
    virtual bool finalize(vessel& vsl) override
    {
        if(_data1.inner_data() == _ebp) {
            static const byte xpct_hdr[] = { 0xff, 0x75, 0x00 };
            rasm_set_header(vsl, xpct_hdr, sizeof(xpct_hdr));
            return true;
        }
        rasm_set_header(vsl, __rasm_hdr_pusha_r, sizeof(__rasm_hdr_pusha_r));
        rasm_set_register(vsl, _data1.inner_data());
        return true;
    }
};

template<class c1>
struct rasm_pushw {};

template<>
struct rasm_pushw<_bias8_>:
    public rasm_instr1<_bias8_>
{
    rasm_announce_i(pushw);
    virtual bool finalize(vessel& vsl) override
    {
        rasm_set_header(vsl, __rasm_hdr_pushw_b8, sizeof(__rasm_hdr_pushw_b8));
        rasm_append_data(vsl, _data1);
        return true;
    }
};

template<>
struct rasm_pushw<_bias_>:
    public rasm_instr1<_bias_>
{
    rasm_announce_i(pushw);
    virtual bool finalize(vessel& vsl) override
    {
        rasm_set_header(vsl, __rasm_hdr_pushw_b, sizeof(__rasm_hdr_pushw_b));
        rasm_append_data(vsl, _data1);
        return true;
    }
};

struct rasm_pushad:
    public rasm_instr0<rasm_instr>
{
    rasm_announce(pushad);
    virtual bool finalize(vessel& vsl) override
    {
        rasm_set_header(vsl, __rasm_hdr_pushad, sizeof(__rasm_hdr_pushad));
        return true;
    }
};

template<class c1>
struct rasm_pop {};

template<>
struct rasm_pop<reg>:
    public rasm_instr1<reg>
{
    rasm_announce_i(pop);
    virtual bool finalize(vessel& vsl) override
    {
        rasm_set_header(vsl, __rasm_hdr_pop_r, sizeof(__rasm_hdr_pop_r));
        rasm_set_register(vsl, _data1);
        return true;
    }
};

template<>
struct rasm_pop<_mem_>:
    public rasm_instr1<_mem_>
{
    rasm_announce_i(pop);
    virtual bool finalize(vessel& vsl) override
    {
        rasm_set_header(vsl, __rasm_hdr_pop_m, sizeof(__rasm_hdr_pop_m));
        rasm_append_data(vsl, _data1);
        return true;
    }
};

template<>
struct rasm_pop<_bias8_>:
    public rasm_instr1<_bias8_>
{
    rasm_announce_i(pop);
    virtual bool finalize(vessel& vsl) override
    {
        rasm_set_header(vsl, __rasm_hdr_pop_b8, sizeof(__rasm_hdr_pop_b8));
        rasm_append_data(vsl, _data1);
        return true;
    }
};

template<>
struct rasm_pop<_bias_>:
    public rasm_instr1<_bias_>
{
    rasm_announce_i(pop);
    virtual bool finalize(vessel& vsl) override
    {
        rasm_set_header(vsl, __rasm_hdr_pop_b, sizeof(__rasm_hdr_pop_b));
        rasm_append_data(vsl, _data1);
        return true;
    }
};

struct rasm_popad:
    public rasm_instr0<rasm_instr>
{
    rasm_announce(popad);
    virtual bool finalize(vessel& vsl) override
    {
        rasm_set_header(vsl, __rasm_hdr_popad, sizeof(__rasm_hdr_popad));
        return true;
    }
};

/*
template<class c1>
struct rasm_label {};

template<>
struct rasm_label<lab>:
    public rasm_instr1<lab>
{
    rasm_announce_i(label);
    virtual bool finalize(vessel& vsl)
    {
        return true;
    }
};
*/

template<class c1>
struct rasm_jmp {};

template<>
struct rasm_jmp<_mem_>:
    public rasm_instr1<_mem_>
{
    rasm_announce_i(jmp);
    virtual bool finalize(vessel& vsl) override
    {
        rasm_set_header(vsl, __rasm_hdr_jmp_m, sizeof(__rasm_hdr_jmp_m));
        rasm_append_data(vsl, _data1);
        return true;
    }
};

template<>
struct rasm_jmp<_bias8_>:
    public rasm_instr1<_bias8_>
{
    rasm_announce_i(jmp);
    virtual bool finalize(vessel& vsl) override
    {
        rasm_set_header(vsl, __rasm_hdr_jmp_b8, sizeof(__rasm_hdr_jmp_b8));
        rasm_append_data(vsl, _data1);
        return true;
    }
};

template<>
struct rasm_jmp<_bias_>:
    public rasm_instr1<_bias_>
{
    rasm_announce_i(jmp);
    virtual bool finalize(vessel& vsl) override
    {
        rasm_set_header(vsl, __rasm_hdr_jmp_b, sizeof(__rasm_hdr_jmp_b));
        rasm_append_data(vsl, _data1);
        return true;
    }
};

template<>
struct rasm_jmp<reg>:
    public rasm_instr1<reg>
{
    rasm_announce_i(jmp);
    virtual bool finalize(vessel& vsl) override
    {
        rasm_set_header(vsl, __rasm_hdr_jmp_r, sizeof(__rasm_hdr_jmp_r));
        rasm_set_register(vsl, _data1);
        return true;
    }
};

template<>
struct rasm_jmp<lab>:
    public rasm_linker
{
    rasm_announce_i_lnk(jmp);
    virtual bool finalize(vessel& vsl) override
    {
        // todo
        return true;
    }
};

template<class c1>
struct rasm_jmps {};

template<>
struct rasm_jmps<_bias8_>:
    public rasm_instr1<_bias8_>
{
    rasm_announce_i(jmps);
    virtual bool finalize(vessel& vsl) override
    {
        rasm_set_header(vsl, __rasm_hdr_jmps_b8, sizeof(__rasm_hdr_jmps_b8));
        rasm_append_data(vsl, _data1);
        return true;
    }
};

template<>
struct rasm_jmps<_bias_>:
    public rasm_instr1<_bias_>
{
    rasm_announce_i(jmps);
    virtual bool finalize(vessel& vsl) override
    {
        rasm_set_header(vsl, __rasm_hdr_jmps_b, sizeof(__rasm_hdr_jmps_b));
        rasm_append_data(vsl, _data1);
        return true;
    }
};

template<class c1>
struct rasm_jes {};

template<>
struct rasm_jes<_bias8_>:
    public rasm_instr1<_bias8_>
{
    rasm_announce_i(jes);
    virtual bool finalize(vessel& vsl) override
    {
        rasm_set_header(vsl, __rasm_hdr_jes_b8, sizeof(__rasm_hdr_jes_b8));
        rasm_append_data(vsl, _data1);
        return true;
    }
};

template<>
struct rasm_jes<_bias_>:
    public rasm_instr1<_bias_>
{
    rasm_announce_i(jes);
    virtual bool finalize(vessel& vsl) override
    {
        rasm_set_header(vsl, __rasm_hdr_jes_b, sizeof(__rasm_hdr_jes_b));
        rasm_append_data(vsl, _data1);
        return true;
    }
};

template<class c1>
struct rasm_jnes {};

template<>
struct rasm_jnes<_bias8_>:
    public rasm_instr1<_bias8_>
{
    rasm_announce_i(jnes);
    virtual bool finalize(vessel& vsl) override
    {
        rasm_set_header(vsl, __rasm_hdr_jnes_b8, sizeof(__rasm_hdr_jnes_b8));
        rasm_append_data(vsl, _data1);
        return true;
    }
};

template<>
struct rasm_jnes<_bias_>:
    public rasm_instr1<_bias_>
{
    rasm_announce_i(jnes);
    virtual bool finalize(vessel& vsl) override
    {
        rasm_set_header(vsl, __rasm_hdr_jnes_b, sizeof(__rasm_hdr_jnes_b));
        rasm_append_data(vsl, _data1);
        return true;
    }
};

template<class c1>
struct rasm_loop {};

template<>
struct rasm_loop<int8>:
    public rasm_instr1<int8>
{
    rasm_announce_i(loop);
    virtual bool finalize(vessel& vsl) override
    {
        rasm_set_header(vsl, __rasm_hdr_loop, sizeof(__rasm_hdr_loop));
        rasm_append_data(vsl, _data1 - 2);
        return true;
    }
};

template<>
struct rasm_loop<int>:
    public rasm_instr1<int>
{
    rasm_announce_i(loop);
    virtual bool finalize(vessel& vsl) override
    {
        rasm_set_header(vsl, __rasm_hdr_loop, sizeof(__rasm_hdr_loop));
        rasm_append_data<word>(vsl, _data1 - 3);
        return true;
    }
};

template<>
struct rasm_loop<lab>:
    public rasm_linker
{
    rasm_announce_i_lnk(loop);
    virtual bool finalize(vessel& vsl) override
    {
        // todo
        return true;
    }
};

struct rasm_cld:
    public rasm_instr0<rasm_instr>
{
    rasm_announce(cld);
    virtual bool finalize(vessel& vsl) override
    {
        rasm_set_header(vsl, __rasm_hdr_cld, sizeof(__rasm_hdr_cld));
        return true;
    }
};

struct rasm_std:
    public rasm_instr0<rasm_instr>
{
    rasm_announce(std);
    virtual bool finalize(vessel& vsl) override
    {
        rasm_set_header(vsl, __rasm_hdr_std, sizeof(__rasm_hdr_std));
        return true;
    }
};

struct rasm_movsb:
    public rasm_instr0<rasm_instr>
{
    rasm_announce(movsb);
    virtual bool finalize(vessel& vsl) override
    {
        rasm_set_header(vsl, __rasm_hdr_movsb, sizeof(__rasm_hdr_movsb));
        return true;
    }
};

struct rasm_movsw:
    public rasm_instr0<rasm_instr>
{
    rasm_announce(movsw);
    virtual bool finalize(vessel& vsl) override
    {
        rasm_set_header(vsl, __rasm_hdr_movsw, sizeof(__rasm_hdr_movsw));
        return true;
    }
};

struct rasm_movs:
    public rasm_instr0<rasm_instr>
{
    rasm_announce(movs);
    virtual bool finalize(vessel& vsl) override
    {
        rasm_set_header(vsl, __rasm_hdr_movs, sizeof(__rasm_hdr_movs));
        return true;
    }
};

struct rasm_rep:
    public rasm_instr0<rasm_instr>
{
    rasm_announce(rep);
    virtual bool finalize(vessel& vsl) override
    {
        rasm_set_header(vsl, __rasm_hdr_rep, sizeof(__rasm_hdr_rep));
        return true;
    }
};

template<class c1, class c2>
struct rasm_cmp {};

template<>
struct rasm_cmp<_bias_, int>:
    public rasm_instr2<_bias_, int>
{
    rasm_announce_ii(cmp);
    virtual bool finalize(vessel& vsl) override
    {
        bool b8, i8 = false;
        rasm_set_header(vsl, __rasm_hdr_cmp_b_i, sizeof(__rasm_hdr_cmp_b_i));
        if((int)_data1.inner_data() > (int)-0x80 && (int)_data1.inner_data() < (int)0x80) {
            b8 = true;
            vsl.current<byte>(-1) |= 0x40;
        }
        else {
            b8 = false;
            vsl.current<byte>(-1) |= 0x80;
        }
        if((int)_data2 > (int)-0x80 && (int)_data2 < (int)0x80) {
            i8 = true;
            vsl.current<byte>(-2) |= 0x02;
        }
        if(b8) rasm_append_data<byte>(vsl, (byte)_data1.inner_data());
        else   rasm_append_data<dword>(vsl, (dword)_data1.inner_data());
        if(i8) rasm_append_data<byte>(vsl, (byte)_data2);
        else   rasm_append_data<dword>(vsl, (dword)_data2);
        return true;
    }
};

template<>
struct rasm_cmp<reg, int8>:
    public rasm_instr2<reg, int8>
{
    rasm_announce_ii(cmp);
    virtual bool finalize(vessel& vsl) override
    {
        rasm_set_header(vsl, __rasm_hdr_cmp_r_i8, sizeof(__rasm_hdr_cmp_r_i8));
        rasm_set_register(vsl, _data1);
        rasm_append_data<byte>(vsl, _data2);
        return true;
    }
};

template<>
struct rasm_cmp<reg, int>:
    public rasm_instr2<reg, int>
{
    rasm_announce_ii(cmp);
    virtual bool finalize(vessel& vsl) override
    {
        rasm_set_header(vsl, __rasm_hdr_cmp_r_i, sizeof(__rasm_hdr_cmp_r_i));
        rasm_set_register(vsl, _data1);
        rasm_append_data<dword>(vsl, (dword)_data2);
        return true;
    }
};

template<>
struct rasm_cmp<reg, _bias8_>:
    public rasm_instr2<reg, _bias8_>
{
    rasm_announce_ii(cmp);
    virtual bool finalize(vessel& vsl) override
    {
        rasm_set_header(vsl, __rasm_hdr_cmp_r_b8, sizeof(__rasm_hdr_cmp_r_b8));
        rasm_set_register(vsl, _data1, 3);
        rasm_append_data<byte>(vsl, (byte)_data2.inner_data());
        return true;
    }
};

template<>
struct rasm_cmp<reg, _bias_>:
    public rasm_instr2<reg, _bias_>
{
    rasm_announce_ii(cmp);
    virtual bool finalize(vessel& vsl) override
    {
        rasm_set_header(vsl, __rasm_hdr_cmp_r_b, sizeof(__rasm_hdr_cmp_r_b));
        rasm_set_register(vsl, _data1, 3);
        rasm_append_data<dword>(vsl, (dword)_data2.inner_data());
        return true;
    }
};

template<>
struct rasm_cmp<_reg_, int8>:
    public rasm_instr2<_reg_, int8>
{
    rasm_announce_ii(cmp);
    virtual bool finalize(vessel& vsl) override
    {
        rasm_set_header(vsl, __rasm_hdr_cmpa_r_i8, sizeof(__rasm_hdr_cmpa_r_i8));
        rasm_set_register(vsl, _data1.inner_data());
        rasm_append_data<byte>(vsl, (byte)_data2);
        return true;
    }
};

template<>
struct rasm_cmp<_reg_, int>:
    public rasm_instr2<_reg_, int>
{
    rasm_announce_ii(cmp);
    virtual bool finalize(vessel& vsl) override
    {
        rasm_set_header(vsl, __rasm_hdr_cmpa_r_i, sizeof(__rasm_hdr_cmpa_r_i));
        rasm_set_register(vsl, _data1.inner_data());
        rasm_append_data<dword>(vsl, _data2);
        return true;
    }
};

template<>
struct rasm_cmp<reg, _reg_>:
    public rasm_instr2<reg, _reg_>
{
    rasm_announce_ii(cmp);
    virtual bool finalize(vessel& vsl) override
    {
        rasm_set_header(vsl, __rasm_hdr_cmpra_r_r, sizeof(__rasm_hdr_cmpra_r_r));
        rasm_set_register(vsl, _data1, 3);
        rasm_set_register(vsl, _data2.inner_data());
        return true;
    }
};

template<class c1>
struct rasm_inc {};

template<>
struct rasm_inc<reg>:
    public rasm_instr1<reg>
{
    rasm_announce_i(inc);
    virtual bool finalize(vessel& vsl) override
    {
        rasm_set_header(vsl, __rasm_hdr_inc_r, sizeof(__rasm_hdr_inc_r));
        rasm_set_register(vsl, _data1);
        return true;
    }
};

template<>
struct rasm_inc<_mem_>:
    public rasm_instr1<_mem_>
{
    rasm_announce_i(inc);
    virtual bool finalize(vessel& vsl) override
    {
        rasm_set_header(vsl, __rasm_hdr_inc_m, sizeof(__rasm_hdr_inc_m));
        rasm_append_data<dword>(vsl, _data1.inner_data());
        return true;
    }
};

template<>
struct rasm_inc<_bias8_>:
    public rasm_instr1<_bias8_>
{
    rasm_announce_i(inc);
    virtual bool finalize(vessel& vsl) override
    {
        rasm_set_header(vsl, __rasm_hdr_inc_b8, sizeof(__rasm_hdr_inc_b8));
        rasm_append_data<byte>(vsl, _data1.inner_data());
        return true;
    }
};

template<>
struct rasm_inc<_bias_>:
    public rasm_instr1<_bias_>
{
    rasm_announce_i(inc);
    virtual bool finalize(vessel& vsl) override
    {
        rasm_set_header(vsl, __rasm_hdr_inc_b, sizeof(__rasm_hdr_inc_b));
        rasm_append_data<dword>(vsl, _data1.inner_data());
        return true;
    }
};

template<class c1, class c2>
struct rasm_add {};

template<>
struct rasm_add<reg, reg>:
    public rasm_instr2<reg, reg>
{
    rasm_announce_ii(add);
    virtual bool finalize(vessel& vsl) override
    {
        rasm_set_header(vsl, __rasm_hdr_add_r_r, sizeof(__rasm_hdr_add_r_r));
        rasm_set_register(vsl, _data1, 3);
        rasm_set_register(vsl, _data2);
        return true;
    }
};

template<>
struct rasm_add<reg, int8>:
    public rasm_instr2<reg, int8>
{
    rasm_announce_ii(add);
    virtual bool finalize(vessel& vsl) override
    {
        return true;
    }
};

template<>
struct rasm_add<reg, int>:
    public rasm_instr2<reg, int>
{
    rasm_announce_ii(add);
    virtual bool finalize(vessel& vsl) override
    {
        return true;
    }
};

template<>
struct rasm_add<int, void>:
    public rasm_instr1<int>
{
    rasm_announce_i(add);
    virtual bool finalize(vessel& vsl) override
    {
        return true;
    }
};

template<>
struct rasm_add<reg, _mem_>:
    public rasm_instr2<reg, _mem_>
{
    rasm_announce_ii(add);
    virtual bool finalize(vessel& vsl) override
    {
        return true;
    }
};

template<>
struct rasm_add<reg, _bias8_>:
    public rasm_instr2<reg, _bias8_>
{
    rasm_announce_ii(add);
    virtual bool finalize(vessel& vsl) override
    {
        return true;
    }
};

template<>
struct rasm_add<reg, _bias_>:
    public rasm_instr2<reg, _bias_>
{
    rasm_announce_ii(add);
    virtual bool finalize(vessel& vsl) override
    {
        return true;
    }
};

template<>
struct rasm_add<_mem_, int8>:
    public rasm_instr2<_mem_, int8>
{
    rasm_announce_ii(add);
    virtual bool finalize(vessel& vsl) override
    {
        return true;
    }
};

template<>
struct rasm_add<_mem_, int>:
    public rasm_instr2<_mem_, int>
{
    rasm_announce_ii(add);
    virtual bool finalize(vessel& vsl) override
    {
        return true;
    }
};

template<>
struct rasm_add<_bias8_, int8>:
    public rasm_instr2<_bias8_, int8>
{
    rasm_announce_ii(add);
    virtual bool finalize(vessel& vsl) override
    {
        return true;
    }
};

template<>
struct rasm_add<_bias8_, int>:
    public rasm_instr2<_bias8_, int>
{
    rasm_announce_ii(add);
    virtual bool finalize(vessel& vsl) override
    {
        return true;
    }
};

template<>
struct rasm_add<_bias_, int8>:
    public rasm_instr2<_bias_, int8>
{
    rasm_announce_ii(add);
    virtual bool finalize(vessel& vsl) override
    {
        return true;
    }
};

template<>
struct rasm_add<_bias_, int>:
    public rasm_instr2<_bias_, int>
{
    rasm_announce_ii(add);
    virtual bool finalize(vessel& vsl) override
    {
        return true;
    }
};

template<>
struct rasm_add<_mem_, reg>:
    public rasm_instr2<_mem_, reg>
{
    rasm_announce_ii(add);
    virtual bool finalize(vessel& vsl) override
    {
        return true;
    }
};

template<>
struct rasm_add<_bias8_, reg>:
    public rasm_instr2<_bias8_, reg>
{
    rasm_announce_ii(add);
    virtual bool finalize(vessel& vsl) override
    {
        return true;
    }
};

template<>
struct rasm_add<_bias_, reg>:
    public rasm_instr2<_bias_, reg>
{
    rasm_announce_ii(add);
    virtual bool finalize(vessel& vsl) override
    {
        return true;
    }
};

template<>
struct rasm_add<reg, _reg_>:
    public rasm_instr2<reg, _reg_>
{
    rasm_announce_ii(add);
    virtual bool finalize(vessel& vsl) override
    {
        return true;
    }
};

template<>
struct rasm_add<_reg_, reg>:
    public rasm_instr2<_reg_, reg>
{
    rasm_announce_ii(add);
    virtual bool finalize(vessel& vsl) override
    {
        return true;
    }
};

template<class c1>
struct rasm_dec {};

template<>
struct rasm_dec<reg>:
    public rasm_instr1<reg>
{
    rasm_announce_i(dec);
    virtual bool finalize(vessel& vsl) override
    {
        return true;
    }
};

template<>
struct rasm_dec<_mem_>:
    public rasm_instr1<_mem_>
{
    rasm_announce_i(dec);
    virtual bool finalize(vessel& vsl) override
    {
        return true;
    }
};

template<>
struct rasm_dec<_bias_>:
    public rasm_instr1<_bias_>
{
    rasm_announce_i(dec);
    virtual bool finalize(vessel& vsl) override
    {
        return true;
    }
};

template<class c1, class c2>
struct rasm_sub {};

template<>
struct rasm_sub<reg, reg>:
    public rasm_instr2<reg, reg>
{
    rasm_announce_ii(sub);
    virtual bool finalize(vessel& vsl) override
    {
        return true;
    }
};

template<>
struct rasm_sub<reg, int>:
    public rasm_instr2<reg, int>
{
    rasm_announce_ii(sub);
    virtual bool finalize(vessel& vsl) override
    {
        return true;
    }
};

template<>
struct rasm_sub<int, void>:
    public rasm_instr1<int>
{
    rasm_announce_i(sub);
    virtual bool finalize(vessel& vsl) override
    {
        return true;
    }
};

template<>
struct rasm_sub<reg, _mem_>:
    public rasm_instr2<reg, _mem_>
{
    rasm_announce_ii(sub);
    virtual bool finalize(vessel& vsl) override
    {
        return true;
    }
};

template<>
struct rasm_sub<reg, _bias_>:
    public rasm_instr2<reg, _bias_>
{
    rasm_announce_ii(sub);
    virtual bool finalize(vessel& vsl) override
    {
        return true;
    }
};

template<>
struct rasm_sub<_mem_, int>:
    public rasm_instr2<_mem_, int>
{
    rasm_announce_ii(sub);
    virtual bool finalize(vessel& vsl) override
    {
        return true;
    }
};

template<>
struct rasm_sub<_bias_, int>:
    public rasm_instr2<_bias_, int>
{
    rasm_announce_ii(sub);
    virtual bool finalize(vessel& vsl) override
    {
        return true;
    }
};

template<>
struct rasm_sub<_mem_, reg>:
    public rasm_instr2<_mem_, reg>
{
    rasm_announce_ii(sub);
    virtual bool finalize(vessel& vsl) override
    {
        return true;
    }
};

template<>
struct rasm_sub<_bias_, reg>:
    public rasm_instr2<_bias_, reg>
{
    rasm_announce_ii(sub);
    virtual bool finalize(vessel& vsl) override
    {
        return true;
    }
};

template<>
struct rasm_sub<reg, _reg_>:
    public rasm_instr2<reg, _reg_>
{
    rasm_announce_ii(sub);
    virtual bool finalize(vessel& vsl) override
    {
        return true;
    }
};

template<>
struct rasm_sub<_reg_, reg>:
    public rasm_instr2<_reg_, reg>
{
    rasm_announce_ii(sub);
    virtual bool finalize(vessel& vsl) override
    {
        return true;
    }
};

template<class c1, class c2, class c3>
struct rasm_imul {};

template<>
struct rasm_imul<reg, reg, int>:
    public rasm_instr3<reg, reg, int>
{
    rasm_announce_iii(imul);
    virtual bool finalize(vessel& vsl) override
    {
        return true;
    }
};

template<>
struct rasm_imul<reg, int, void>:
    public rasm_instr2<reg, int>
{
    rasm_announce_ii(imul);
    virtual bool finalize(vessel& vsl) override
    {
        return true;
    }
};

template<>
struct rasm_imul<reg, void, void>:
    public rasm_instr1<reg>
{
    rasm_announce_i(imul);
    virtual bool finalize(vessel& vsl) override
    {
        return true;
    }
};

template<>
struct rasm_imul<reg, _mem_, void>:
    public rasm_instr2<reg, _mem_>
{
    rasm_announce_ii(imul);
    virtual bool finalize(vessel& vsl) override
    {
        return true;
    }
};

template<>
struct rasm_imul<reg, _bias_, void>:
    public rasm_instr2<reg, _bias_>
{
    rasm_announce_ii(imul);
    virtual bool finalize(vessel& vsl) override
    {
        return true;
    }
};

template<>
struct rasm_imul<reg, _mem_, int>:
    public rasm_instr3<reg, _mem_, int>
{
    rasm_announce_iii(imul);
    virtual bool finalize(vessel& vsl) override
    {
        return true;
    }
};

template<>
struct rasm_imul<reg, _bias_, int>:
    public rasm_instr3<reg, _bias_, int>
{
    rasm_announce_iii(imul);
    virtual bool finalize(vessel& vsl) override
    {
        return true;
    }
};

template<>
struct rasm_imul<reg, reg, void>:
    public rasm_instr2<reg, reg>
{
    rasm_announce_ii(imul);
    virtual bool finalize(vessel& vsl) override
    {
        return true;
    }
};

template<>
struct rasm_imul<_mem_, void, void>:
    public rasm_instr1<_mem_>
{
    rasm_announce_i(imul);
    virtual bool finalize(vessel& vsl) override
    {
        return true;
    }
};

template<>
struct rasm_imul<_bias_, void, void>:
    public rasm_instr1<_bias_>
{
    rasm_announce_i(imul);
    virtual bool finalize(vessel& vsl) override
    {
        return true;
    }
};

template<>
struct rasm_imul<reg, _reg_, void>:
    public rasm_instr2<reg, _reg_>
{
    rasm_announce_ii(imul);
    virtual bool finalize(vessel& vsl) override
    {
        return true;
    }
};

template<class c1>
struct rasm_idiv {};

template<>
struct rasm_idiv<reg>:
    public rasm_instr1<reg>
{
    rasm_announce_i(idiv);
    virtual bool finalize(vessel& vsl) override
    {
        return true;
    }
};

template<>
struct rasm_idiv<_mem_>:
    public rasm_instr1<_mem_>
{
    rasm_announce_i(idiv);
    virtual bool finalize(vessel& vsl) override
    {
        return true;
    }
};

template<>
struct rasm_idiv<_bias_>:
    public rasm_instr1<_bias_>
{
    rasm_announce_i(idiv);
    virtual bool finalize(vessel& vsl) override
    {
        return true;
    }
};

template<>
struct rasm_idiv<_reg_>:
    public rasm_instr1<_reg_>
{
    rasm_announce_i(idiv);
    virtual bool finalize(vessel& vsl) override
    {
        return true;
    }
};

struct rasm_cdq:
    public rasm_instr0<rasm_instr>
{
    rasm_announce(cdq);
    virtual bool finalize(vessel& vsl) override
    {
        return true;
    }
};

template<class c1>
struct rasm_neg {};

template<>
struct rasm_neg<reg>:
    public rasm_instr1<reg>
{
    rasm_announce_i(neg);
    virtual bool finalize(vessel& vsl) override
    {
        return true;
    }
};

template<class c1>
struct rasm_not {};

template<>
struct rasm_not<reg>:
    public rasm_instr1<reg>
{
    rasm_announce_i(not);
    virtual bool finalize(vessel& vsl) override
    {
        return true;
    }
};

template <class c1, class c2>
struct rasm_xor {};

template<>
struct rasm_xor<reg, reg>:
    public rasm_instr2<reg, reg>
{
    rasm_announce_ii(xor);
    virtual bool finalize(vessel& vsl) override
    {
        return true;
    }
};

template<>
struct rasm_xor<reg, int>:
    public rasm_instr2<reg, int>
{
    rasm_announce_ii(xor);
    virtual bool finalize(vessel& vsl) override
    {
        return true;
    }
};

template<>
struct rasm_xor<reg, _bias_>:
    public rasm_instr2<reg, _bias_>
{
    rasm_announce_ii(xor);
    virtual bool finalize(vessel& vsl) override
    {
        return true;
    }
};

template<>
struct rasm_xor<reg, _reg_>:
    public rasm_instr2<reg, _reg_>
{
    rasm_announce_ii(xor);
    virtual bool finalize(vessel& vsl) override
    {
        return true;
    }
};

template<class c1, class c2>
struct rasm_shl {};

template<>
struct rasm_shl<reg, void>:
    public rasm_instr1<reg>
{
    rasm_announce_i(shl);
    virtual bool finalize(vessel& vsl) override
    {
        return true;
    }
};

template<>
struct rasm_shl<reg, int>:
    public rasm_instr2<reg, int>
{
    rasm_announce_ii(shl);
    virtual bool finalize(vessel& vsl) override
    {
        return true;
    }
};

template<class c1, class c2>
struct rasm_shr {};

template<>
struct rasm_shr<reg, void>:
    public rasm_instr1<reg>
{
    rasm_announce_i(shr);
    virtual bool finalize(vessel& vsl) override
    {
        return true;
    }
};

template<>
struct rasm_shr<reg, int>:
    public rasm_instr2<reg, int>
{
    rasm_announce_ii(shr);
    virtual bool finalize(vessel& vsl) override
    {
        return true;
    }
};

template<class c1, class c2>
struct rasm_and {};

template<>
struct rasm_and<reg, int>:
    public rasm_instr2<reg, int>
{
    rasm_announce_ii(and);
    virtual bool finalize(vessel& vsl) override
    {
        return true;
    }
};

template<>
struct rasm_and<reg, _bias_>:
    public rasm_instr2<reg, _bias_>
{
    rasm_announce_ii(and);
    virtual bool finalize(vessel& vsl) override
    {
        return true;
    }
};

template<>
struct rasm_and<reg, _reg_>:
    public rasm_instr2<reg, _reg_>
{
    rasm_announce_ii(and);
    virtual bool finalize(vessel& vsl) override
    {
        return true;
    }
};

template<class c1, class c2>
struct rasm_or {};

template<>
struct rasm_or<reg, int>:
    public rasm_instr2<reg, int>
{
    rasm_announce_ii(or);
    virtual bool finalize(vessel& vsl) override
    {
        return true;
    }
};

template<>
struct rasm_or<reg, _bias_>:
    public rasm_instr2<reg, _bias_>
{
    rasm_announce_ii(or);
    virtual bool finalize(vessel& vsl) override
    {
        return true;
    }
};

template<>
struct rasm_or<reg, _reg_>:
    public rasm_instr2<reg, _reg_>
{
    rasm_announce_ii(or);
    virtual bool finalize(vessel& vsl) override
    {
        return true;
    }
};

rasm::~rasm()
{
    auto end = _list.end();
    for(auto i = _list.begin(); i != end; ++ i)
        gs_del(rasm_instr, *i);
    _list.clear();
    _label.clear();
    _link.clear();
}

void rasm::print(string& str) const
{
}

int rasm::finalize(vessel& vsl)
{
    return 0;
}

int rasm::get_last_instruction_index() const
{
    int l = (int)_list.size();
    assert(l > 0);
    return -- l;
}

//void rasm::attach(const_iterator pos, rasm& other)
//{
//    instr_list& othlist = other._list;
//    _list.splice(_list.end(), othlist);
//    assert(othlist.size() == 0);
//}

rasm_impinstr_ii(mov, reg, int8);
rasm_impinstr_ii(mov, reg, int);
rasm_impinstr_ii(mov, reg, reg);
rasm_impinstr_ii(mov, reg, _mem_);
rasm_impinstr_ii(mov, reg, _bias8_);
rasm_impinstr_ii(mov, reg, _bias_);
rasm_impinstr_ii(mov, _mem_, reg);
rasm_impinstr_ii(mov, _bias8_, reg);
rasm_impinstr_ii(mov, _bias_, reg);
rasm_impinstr_ii(mov, _mem_, int);
rasm_impinstr_ii(mov, _bias8_, int);
rasm_impinstr_ii(mov, _bias_, int);
rasm_impinstr_ii(mov, reg, _reg_);
rasm_impinstr_ii(mov, _reg_, reg);
rasm_impinstr_ii(lea, reg, _bias8_);
rasm_impinstr_ii(lea, reg, _bias_);
rasm_impinstr(nop);
rasm_impinstr(hlt);
rasm_impinstr_i(call, int);
rasm_impinstr_i(call, reg);
rasm_impinstr_i(call, _bias_);
rasm_impinstr_i(call, lab);
rasm_impinstr_x(ret);
rasm_impinstr_i(ret, int);
rasm_impinstr_i(push, reg);
rasm_impinstr_i(push, int8);
rasm_impinstr_i(push, int);
rasm_impinstr_i(push, _mem_);
rasm_impinstr_i(push, _bias8_);
rasm_impinstr_i(push, _bias_);
rasm_impinstr_i(push, _reg_);
rasm_impinstr_i(pushw, _bias8_);
rasm_impinstr_i(pushw, _bias_);
rasm_impinstr(pushad);
rasm_impinstr_i(pop, reg);
rasm_impinstr_i(pop, _mem_);
rasm_impinstr_i(pop, _bias8_);
rasm_impinstr_i(pop, _bias_);
rasm_impinstr(popad);
rasm_impinstr_l(label, lab);
rasm_impinstr_i(jmp, _mem_);
rasm_impinstr_i(jmp, _bias8_);
rasm_impinstr_i(jmp, _bias_);
rasm_impinstr_i(jmp, reg);
rasm_impinstr_i(jmps, _bias8_);
rasm_impinstr_i(jmps, _bias_);
rasm_impinstr_i(jmp, lab);
rasm_impinstr_i(jes, _bias8_);
rasm_impinstr_i(jes, _bias_);
rasm_impinstr_i(jnes, _bias8_);
rasm_impinstr_i(jnes, _bias_);
rasm_impinstr_i(loop, int8);
rasm_impinstr_i(loop, int);
rasm_impinstr_i(loop, lab);
rasm_impinstr(cld);
rasm_impinstr(std);
rasm_impinstr(movsb);
rasm_impinstr(movsw);
rasm_impinstr(movs);
rasm_impinstr(rep);
rasm_impinstr_ii(cmp, _bias_, int);
rasm_impinstr_ii(cmp, reg, int8);
rasm_impinstr_ii(cmp, reg, int);
rasm_impinstr_ii(cmp, reg, _bias8_);
rasm_impinstr_ii(cmp, reg, _bias_);
rasm_impinstr_ii(cmp, _reg_, int8);
rasm_impinstr_ii(cmp, _reg_, int);
rasm_impinstr_ii(cmp, reg, _reg_);
rasm_impinstr_i(inc, reg);
rasm_impinstr_i(inc, _mem_);
rasm_impinstr_i(inc, _bias8_);
rasm_impinstr_i(inc, _bias_);
rasm_impinstr_ii(add, reg, reg);
rasm_impinstr_ii(add, reg, int8);
rasm_impinstr_ii(add, reg, int);
rasm_impinstr_ix(add, int);
rasm_impinstr_ii(add, reg, _mem_);
rasm_impinstr_ii(add, reg, _bias8_);
rasm_impinstr_ii(add, reg, _bias_);
rasm_impinstr_ii(add, _mem_, int8);
rasm_impinstr_ii(add, _mem_, int);
rasm_impinstr_ii(add, _bias8_, int8);
rasm_impinstr_ii(add, _bias8_, int);
rasm_impinstr_ii(add, _bias_, int8);
rasm_impinstr_ii(add, _bias_, int);
rasm_impinstr_ii(add, _mem_, reg);
rasm_impinstr_ii(add, _bias8_, reg);
rasm_impinstr_ii(add, _bias_, reg);
rasm_impinstr_ii(add, reg, _reg_);
rasm_impinstr_ii(add, _reg_, reg);
rasm_impinstr_i(dec, reg);
rasm_impinstr_i(dec, _mem_);
rasm_impinstr_i(dec, _bias_);
rasm_impinstr_ii(sub, reg, reg);
rasm_impinstr_ii(sub, reg, int);
rasm_impinstr_ix(sub, int);
rasm_impinstr_ii(sub, reg, _mem_);
rasm_impinstr_ii(sub, reg, _bias_);
rasm_impinstr_ii(sub, _mem_, int);
rasm_impinstr_ii(sub, _bias_, int);
rasm_impinstr_ii(sub, _mem_, reg);
rasm_impinstr_ii(sub, _bias_, reg);
rasm_impinstr_ii(sub, reg, _reg_);
rasm_impinstr_ii(sub, _reg_, reg);
rasm_impinstr_iii(imul, reg, reg, int);
rasm_impinstr_iix(imul, reg, int);
rasm_impinstr_ixx(imul, reg);
rasm_impinstr_iix(imul, reg, _mem_);
rasm_impinstr_iix(imul, reg, _bias_);
rasm_impinstr_iii(imul, reg, _mem_, int);
rasm_impinstr_iii(imul, reg, _bias_, int);
rasm_impinstr_iix(imul, reg, reg);
rasm_impinstr_ixx(imul, _mem_);
rasm_impinstr_ixx(imul, _bias_);
rasm_impinstr_iix(imul, reg, _reg_);
rasm_impinstr_i(idiv, reg);
rasm_impinstr_i(idiv, _mem_);
rasm_impinstr_i(idiv, _bias_);
rasm_impinstr_i(idiv, _reg_);
rasm_impinstr(cdq);
rasm_impinstr_i(neg, reg);
rasm_impinstr_i(not, reg);
rasm_impinstr_ii(xor, reg, reg);
rasm_impinstr_ii(xor, reg, int);
rasm_impinstr_ii(xor, reg, _bias_);
rasm_impinstr_ii(xor, reg, _reg_);
rasm_impinstr_ix(shl, reg);
rasm_impinstr_ii(shl, reg, int);
rasm_impinstr_ix(shr, reg);
rasm_impinstr_ii(shr, reg, int);
rasm_impinstr_ii(and, reg, int);
rasm_impinstr_ii(and, reg, _bias_);
rasm_impinstr_ii(and, reg, _reg_);
rasm_impinstr_ii(or, reg, int);
rasm_impinstr_ii(or, reg, _bias_);
rasm_impinstr_ii(or, reg, _reg_);

__rathen_end__
