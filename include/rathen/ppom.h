#ifndef ppom_57c54fa0_d309_4aad_9678_16fa29f7af42_h
#define ppom_57c54fa0_d309_4aad_9678_16fa29f7af42_h

#include <rathen\config.h>
#include <gslib\tree.h>
#include <rathen\basis.h>

__rathen_begin__

class ppom
{
public:
    typedef tree<object, _treenode_wrapper<object> > ppom_tree;
    typedef ppom_tree::iterator iterator;
    typedef ppom_tree::const_iterator const_iterator;
    //typedef ;
    
public:
    ppom() {}

public:
    template<class _cst>
    iterator insert(iterator i) { return _tree.insert_tail<_cst>(i); }
    template<class _cst>
    iterator birth(iterator i) { return _tree.birth_tail<_cst>(i); }

protected:
    ppom_tree       _tree;
};

__rathen_end__

#endif
