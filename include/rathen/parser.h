#ifndef parser_496ce533_26fe_412b_8db6_e61b852be838_h
#define parser_496ce533_26fe_412b_8db6_e61b852be838_h

#include <rathen\config.h>
#include <rathen\basis.h>
#include <gslib\tree.h>

__rathen_begin__

class __gs_novtable ps_obj:
    public object
{
public:
    enum
    {
        pt_root,
        pt_block,
        pt_value,
        pt_expression,
        pt_operator,
        pt_function,
        pt_calling,
        pt_if_statement,
        pt_loop_statement,
        pt_go_statement,
        pt_stop_statement,
        pt_exit_statement,
        pt_variable,
        pt_constant,
    };

    enum
    {
        vt_string,
        vt_integer,
        vt_float,
    };

public:
    ps_obj() { _parent = 0; }
    ps_obj(ps_obj* parent) { _parent = parent; }
    ps_obj* get_parent() const { return _parent; }

public:
    virtual ~ps_obj() {}
    virtual bool is_anonymous() const = 0;

protected:
    ps_obj*         _parent;
};

struct ps_trait_pass {};
struct ps_trait_hold {};

template<uint tag, bool anony, class ph_trait, class bsc = ps_obj>
class ps_proto:
    public bsc
{
public:
    ps_proto();
    ps_proto(ps_obj* parent);
    bool is_holder() const override;
    uint get_tag() const override;
    bool is_anonymous() const override;
};

template<uint tag, bool anony, class bsc>
class ps_proto<tag, anony, ps_trait_pass, bsc>:
    public bsc
{
public:
    ps_proto() {}
    ps_proto(ps_obj* parent): bsc(parent) {}
    bool is_holder() const override { return false; }
    uint get_tag() const override { return tag; }
    bool is_anonymous() const override { return anony; }
};

template<uint tag, bool anony, class bsc>
class ps_proto<tag, anony, ps_trait_hold, bsc>:
    public bsc
{
public:
    ps_proto() {}
    ps_proto(ps_obj* parent): bsc(parent) {}
    bool is_holder() const override { return true; }
    uint get_tag() const override { return tag; }
    bool is_anonymous() const override { return anony; }

public:
    virtual void add_indexing(object* obj) { _indexing.add_value(obj); }
    virtual indexing& get_indexing() { return _indexing; }

protected:
    indexing        _indexing;

public:
    object* find_object(const gchar* name) { return _indexing.find_value(name); }
    const object* find_object(const gchar* name) const { return _indexing.find_value(name); } 
};

template<class mc, class pc>
class ps_join:
    public mc,
    public pc
{
public:
    ps_join() {}
    ps_join(ps_obj* parent): mc(parent) {}
};

class ps_comp_value
{
public:
    ps_comp_value() { _type = 0, _global = false, _ref = false; }
    void set_type(type* ty) { _type = ty; }
    void set_global(bool g) { _global = g; }
    void set_ref(bool r) { _ref = r; }
    type* get_type() const { return _type; }
    bool is_complex() const { return _type->is_complex(); }
    bool is_global() const { return _global; }
    bool is_ref() const { return _ref; }

protected:
    type*           _type;
    bool            _global;
    bool            _ref;
};

class ps_comp_operator
{
public:
    ps_comp_operator() { _opr = 0; }
    void set_opr(const oprnode* opr) { _opr = opr; }
    void set_opr(const oprinfo& opr) { _opr = opr_manager::get_singleton_ptr()->get_operator(opr); }
    const oprnode* get_opr_info() const { return _opr; }
    bool is_unary() const { return _opr->unary; }
    int get_priority() const { return _opr->prior; }
    const gchar* get_opr_name() const { return _opr->opr; }
    
protected:
    const oprnode*  _opr;
};

class ps_comp_variable
{
public:
    ps_comp_variable() { _value = 0; }
    void set_value(ps_obj* value) { _value = value; }
    ps_obj* get_value() const { return _value; }

protected:
    ps_obj*         _value;
};

class ps_comp_constant
{
public:
    union vdif
    {
        int         i;
        real32      r;
        int64       i64;
        real        r64;
    };

public:
    ps_comp_constant() {}
    void set_string(const gchar* str) { _cvtag = ps_obj::vt_string, _vstr = str; }
    void set_integer(int i) { _cvtag = ps_obj::vt_integer, _value.i = i; }
    void set_float(real32 r) { _cvtag = ps_obj::vt_float, _value.r = r; }
    uint get_value_tag() const { return _cvtag; }
    const string& get_string() const { return _vstr; }
    int get_integer() const { return _value.i; }
    real32 get_float() const { return _value.r; }

protected:
    uint            _cvtag;
    string          _vstr;
    vdif            _value;
};

class ps_comp_function
{
public:
    ps_comp_function() { _rety = 0, _parent = 0, _retr = false; }
    void set_retv_type(type* ty) { _rety = ty; }
    void set_retv_ref(bool tr) { _retr = tr; }
    type* get_retv_type() const { return _rety; }
    bool get_retv_ref() const { return _retr; }
    void set_parent(ps_obj* obj) { _parent = obj; }
    ps_obj* get_parent() const { return _parent; }
    bool is_top_level() const { return _parent->get_tag() == ps_obj::pt_root; }

protected:
    type*           _rety;
    bool            _retr;
    ps_obj*         _parent;
};

class ps_comp_statement
{
public:
    ps_comp_statement() { _condition = 0, _block = 0; }
    void set_condition(ps_obj* condition) { _condition = condition; }
    void set_block(ps_obj* block) { _block = block; }
    ps_obj* get_condition() const { return _condition; }
    ps_obj* get_block() const { return _block; }

protected:
    ps_obj*         _condition;
    ps_obj*         _block;
};

class ps_comp_if_statement:
    public ps_comp_statement
{
public:
    ps_comp_if_statement() { _prevbr = _nextbr = _lastbr = 0; }
    void set_prev_branch(ps_obj* branch) { _prevbr = branch; }
    void set_next_branch(ps_obj* branch) { _nextbr = branch; }
    void set_last_branch(ps_obj* branch) { _lastbr = branch; }
    ps_obj* get_prev_branch() const { return _prevbr; }
    ps_obj* get_next_branch() const { return _nextbr; }
    ps_obj* get_last_branch() const { return _lastbr; }

protected:
    ps_obj*         _prevbr;
    ps_obj*         _nextbr;
    ps_obj*         _lastbr;
};

class ps_comp_loop_statement:
    public ps_comp_statement
{
};

class ps_comp_exit_statement
{
public:
    ps_comp_exit_statement() { _conclusion = 0; }
    void set_conclusion(ps_obj* conclusion) { _conclusion = conclusion; }
    ps_obj* get_conclusion() const { return _conclusion; }

protected:
    ps_obj*         _conclusion;
};

typedef ps_proto<ps_obj::pt_root, true, ps_trait_hold> ps_root;
typedef ps_proto<ps_obj::pt_block, true, ps_trait_hold> ps_block;
typedef ps_proto<ps_obj::pt_value, false, ps_trait_pass, ps_join<ps_obj, ps_comp_value> > ps_value;
typedef ps_proto<ps_obj::pt_expression, true, ps_trait_pass> ps_expression;
typedef ps_proto<ps_obj::pt_operator, true, ps_trait_pass, ps_join<ps_obj, ps_comp_operator> > ps_operator;
typedef ps_proto<ps_obj::pt_function, false, ps_trait_hold, ps_join<ps_obj, ps_comp_function> > ps_function;
typedef ps_proto<ps_obj::pt_calling, true, ps_trait_pass> ps_calling;
typedef ps_proto<ps_obj::pt_go_statement, true, ps_trait_pass> ps_go_statement;
typedef ps_proto<ps_obj::pt_stop_statement, true, ps_trait_pass> ps_stop_statement;
typedef ps_proto<ps_obj::pt_if_statement, true, ps_trait_pass, ps_join<ps_obj, ps_comp_if_statement> > ps_if_statement;
typedef ps_proto<ps_obj::pt_loop_statement, true, ps_trait_pass, ps_join<ps_obj, ps_comp_loop_statement> > ps_loop_statement;
typedef ps_proto<ps_obj::pt_exit_statement, true, ps_trait_pass, ps_join<ps_obj, ps_comp_exit_statement> > ps_exit_statement;
typedef ps_proto<ps_obj::pt_variable, false, ps_trait_pass, ps_join<ps_obj, ps_comp_variable> > ps_variable;
typedef ps_proto<ps_obj::pt_constant, false, ps_trait_pass, ps_join<ps_obj, ps_comp_constant> > ps_constant;

class parser:
    private tree<ps_obj, _treenode_wrapper<ps_obj> >
{
public:
    typedef _treenode_wrapper<ps_obj> wrapper;
    typedef ps_obj value;
    typedef tree<ps_obj, wrapper> superref;
    typedef superref::iterator iterator;
    typedef superref::const_iterator const_iterator;

public:
    parser() { insert<ps_root>(iterator()); }
    const gchar* get_name() const;
    void set_root(const gchar* name);
    template<class _cst>
    iterator insert(iterator i) { return superref::insert_tail<_cst>(i); }
    template<class _cst>
    iterator birth(iterator i) { return superref::birth_tail<_cst>(i); }

public:
    using superref::clear;
    using superref::erase;
    using superref::get_root;
    using superref::for_each;
    using superref::detach;
    using superref::attach;
    using superref::debug_check;

public:
    iterator find(iterator i, const gchar* name);
};

__rathen_end__

#endif
