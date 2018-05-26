#ifndef basis_dcab07d2_8e6a_4150_b6fb_1eb09f015f76_h
#define basis_dcab07d2_8e6a_4150_b6fb_1eb09f015f76_h

#include <gslib\error.h>
//#include <gslib\string.h>
#include <gslib\std.h>
#include <rathen\config.h>
#include <rathen\buildin.h>

__rathen_begin__

class __gs_novtable type abstract
{
public:
    virtual ~type() {}
    virtual const gchar* get_name() const = 0;
    virtual int get_size() const = 0;
    virtual type* get_oprret(const oprinfo& opr) = 0;
    /*
     * Commutative attribute, if an operator was commutative, then a series of transformation could be done
     * in the expression optimization, for example,
     * it would always prefer to put the variables to the left side of the operator, and the constants to the
     * right side, so that we could generate the code try to put the constants within the instructions.
     */
    virtual bool is_comutative(const oprinfo& opr) const
    {
        if (opr.opr == _t("="))
            return false;
        return true;
    }
    /* Complex attribute, a complex type always has a construction/destruction operation. */
    virtual bool is_complex() const { return false; }
};

class __gs_novtable com_type abstract:
    public type
{
public:
    virtual bool is_complex() const { return true; }
};

class type_manager
{
public:
    typedef std::pair<string, type*> type_pair;
    typedef unordered_map<string, type*> type_map;
    typedef type_map::iterator iterator;
    typedef type_map::const_iterator const_iterator;

public:
    static type_manager* get_singleton_ptr()
    {
        static type_manager inst;
        return &inst;
    }
    ~type_manager()
    {
        std::for_each(_tpmap.begin(), _tpmap.end(), [](const type_pair& tp) { delete tp.second; });
        _tpmap.clear();
    }
    type* find_type(const gchar* name) { return find_type(string(name)); }
    type* find_type(const gchar* name, int len) { return find_type(string(name, len)); }
    type* find_type(const string& name)
    {
        auto i = _tpmap.find(name);
        return i == _tpmap.end() ? 0 : i->second;
    }

private:
    type_map    _tpmap;

public:
    template<class _cst>
    bool regtype()
    {
        type* t = gs_new(_cst);
        if(!_reginner(t)) { gs_del(type, t); return false; }
        return true;
    }

private:
    type_manager();
    bool _reginner(type* t)
    {
        assert(t);
        if(find_type(t->get_name()))
            return false;
        _tpmap.insert(std::make_pair(string(t->get_name()), t));
        return true;
    }
};

#define _type_manager type_manager::get_singleton_ptr()

class int_type:
    public type
{
public:
    virtual const gchar* get_name() const { return _t("int"); }
    virtual int get_size() const { return 4; }
    virtual type* get_oprret(const oprinfo& opr) { return this; }   // TODO
};

class bool_type:
    public type
{
public:
    virtual const gchar* get_name() const { return _t("bool"); }
    virtual int get_size() const { return 4; }
    virtual type* get_oprret(const oprinfo& opr) { return this; }   // TODO
};

class void_type:
    public type
{
public:
    virtual const gchar* get_name() const { return _t("void"); }
    virtual int get_size() const { return 0; }
    virtual type* get_oprret(const oprinfo& opr) { return 0; }
};

class string_type:
    public com_type
{
public:
    virtual const gchar* get_name() const { return _t("string"); }
    virtual int get_size() const { return (int)sizeof(gs::string); }
    virtual type* get_oprret(const oprinfo& opr) { return this; }   // TODO
};

class __gs_novtable object abstract
{
public:
    enum
    {
        tag_data,
        tag_const,
        tag_block,
        tag_node,
        tag_scope,
        //tag_
        //...
    };

protected:
    string      _name;

public:
    object() {}
    virtual ~object() {}
    virtual void set_name(const gchar* name) { _name = name; }
    virtual uint get_tag() const = 0;
    virtual bool is_holder() const = 0;
    virtual const string& get_name() const { return _name; }
};

struct unikey
{
public:
    typedef gchar* vckey;
    typedef const gchar* cckey;

protected:
    cckey       _key;

public:
    unikey(): _key(0) {}
    unikey(const gchar* k): _key(k) {}
    unikey(const string& k): _key(k.c_str()) {}
    unikey(const object* obj): _key(obj->get_name().c_str()) {}
    const gchar* get_key() const { return _key; }
};

struct indexing
{
public:
    struct hash
    {
        size_t operator()(const unikey& k) const
        { return string_hash(k.get_key()); }
    };
    struct equal
    {
        bool operator()(const unikey& k1, const unikey& k2) const
        { return string_hash(k1.get_key()) == string_hash(k2.get_key()); }
    };
    typedef unordered_map<unikey, object*, hash, equal> unimap;
    typedef unimap::iterator iterator;
    typedef unimap::const_iterator const_iterator;

protected:
    unimap      _pairs;

public:
    indexing() {}
    ~indexing() {}
    bool add_value(object* ptr)
    {
        assert(ptr);
        if(_pairs.insert(std::make_pair(unikey(ptr), ptr)).second) {
            set_error(_t("udt insert dp failed, maybe an error in name mangling."));
            return false;
        }
        return true;
    }
    object* find_value(const gchar* name)
    {
        assert(name);
        iterator i = _pairs.find(unikey(name));
        return i == _pairs.end() ? 0 :i->second;
    }
    const object* find_value(const gchar* name) const
    {
        assert(name);
        const_iterator i = _pairs.find(unikey(name));
        return i == _pairs.end() ? 0 : i->second;
    }
};

// class __gnvt pass_object:
//     virtual public object
// {
// public:
//     virtual bool is_holder() const { return false; }
// };
// 
// class __gnvt hold_object:
//     virtual public object
// {
// public:
//     virtual bool is_holder() const { return true; }
//     virtual void add_indexing(object* obj) { _indexing.add_value(obj); }
//     virtual indexing& get_indexing() { return _indexing; }
// 
// protected:
//     indexing    _indexing;
// 
// public:
//     object* find_object(const gchar* name) { return _indexing.find_value(name); }
//     const object* find_object(const gchar* name) const { return _indexing.find_value(name); }
// };

// class block:
//     public object
// {
// protected:
//     indexing    _indexing;
// 
// public:
//     virtual void add_indexing(object* obj) { _indexing.add_value(obj); }
//     virtual indexing& get_indexing() { return _indexing; }
// 
// public:
//     object* find_object(const gchar* name) { return _indexing.find_value(name); }
//     const object* find_object(const gchar* name) const { return _indexing.find_value(name); }
// };
// 
// class data:
//     public object
// {
// protected:
//     type*       _type;
// 
// public:
//     virtual tag get_tag() const { return tag_data; }
//     virtual void set_type(type* t) { _type = t; }
//     virtual type* get_type() const { return _type; }
// };
// 
// class reference:
//     public object
// {
// protected:
//     string      _origin;
// 
// public:
//     virtual tag get_tag() const { return tag_const; }
//     virtual void set_origin(const gchar* str, int len) { _origin.assign(str, len); }
//     virtual const gchar* get_origin() const { return _origin.c_str(); }
// 
// public:
//     void set_origin(const gchar* str) { set_origin(str, strtool::length(str)); }
// };

__rathen_end__

#endif
