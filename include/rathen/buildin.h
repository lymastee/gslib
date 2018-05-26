#ifndef buildin_40bf10d0_ba81_4f06_accf_8020d3698607_h
#define buildin_40bf10d0_ba81_4f06_accf_8020d3698607_h

#include <gslib\type.h>
#include <gslib\string.h>
#include <rathen\config.h>

__rathen_begin__

#define max_opr 4

struct oprnode
{
    int     prior;
    gchar   opr[max_opr];
    bool    unary;
};

struct oprinfo
{
    string      opr;
    bool        unary;

    oprinfo(const gchar* o, bool u): opr(o), unary(u) {}
    oprinfo(const string& s, bool u): opr(s), unary(u) {}
};

struct unary_oprinfo:
    public oprinfo
{
    unary_oprinfo(const gchar* o): oprinfo(o, true) {}
    unary_oprinfo(const string& s): oprinfo(s, true) {}
};

struct binary_oprinfo:
    public oprinfo
{
    binary_oprinfo(const gchar* o): oprinfo(o, false) {}
    binary_oprinfo(const string& s): oprinfo(s, false) {}
};

class opr_manager
{
public:
    /*
     * the operators:
       ( )
       ++ --
       - ! ~
       += -= *= /= %=
       * / %
       + -
       << >>
       < > <= >= == !=
       & | ^ && ||
       =
     */
    static const oprnode opr_list[];

private:
    opr_manager() {}

public:
    static opr_manager* get_singleton_ptr()
    {
        static opr_manager inst;
        return &inst;
    }
    const oprnode* get_unary_operator(const gchar* src) const;
    const oprnode* get_binary_operator(const gchar* src) const;
    const oprnode* get_unary_operator(const string& src) const { return get_unary_operator(src.c_str()); }
    const oprnode* get_binary_operator(const string& src) const { return get_binary_operator(src.c_str()); }
    const oprnode* get_operator(const oprinfo& info) const
    {
        return info.unary ? get_unary_operator(info.opr) :
            get_binary_operator(info.opr);
    }

    /*
     * Currently, only priority '1' was unary.
     * pass the 'opr' to this function, in case of any proper changes in the future.
     */
    bool is_unary(int prior, const gchar* opr) const { return prior == 1; }
    bool is_unary(int prior, const string& opr) const { return prior == 1; }
};

#define _opr_manager    opr_manager::get_singleton_ptr()

#define max_key 8

struct key_node
{
    int     pattern;
    gchar   key[max_key];
};

class key_manager
{
public:
    static const key_node key_list[];
    enum
    {
        /*
         * Respectively like:
         * stop;
         * exit(...);
         * else {...}
         * if(...) {...}
         */
        pat_err     = 0x00,
        pat_raw     = 0x01,
        pat_arg     = 0x02,
        pat_seg     = 0x04,
        pat_com     = 0x08,
    };

private:
    key_manager() {}

public:
    static key_manager* get_singleton_ptr()
    {
        static key_manager inst;
        return &inst;
    }
    const key_node* get_key(const gchar* src) const;
    const key_node* get_key(const string& src) const { return get_key(src.c_str()); }
    int get_key_pattern(const gchar* src) const;
    int get_key_pattern(const string& src) const { return get_key_pattern(src.c_str()); }
};

#define _key_manager    key_manager::get_singleton_ptr()

__rathen_end__

#endif