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

#include <assert.h>
#include <ariel/scenegraph.h>

__ariel_begin__

/* sntype entities */
#define declare_sntype(snt) \
    scene_node::sntype scene_node::snt = 0;
#include <ariel\snt.h>
#undef declare_sntype

void scene_node::register_sntypes()
{
    /* setup this only once */
    static bool setup = false;
    if(setup)
        return;
    setup = true;
    
    /* sntype setups */
    typedef string sntype_entity;
#define declare_sntype(snt) \
    static sntype_entity snt##ent(_t(#snt)); \
    snt = &snt##ent;
#include <ariel\snt.h>
#undef declare_sntype
}

scene_graph::scene_graph()
{
    /* try to setup the scene nodes. */
    scene_node::register_sntypes();
}

scene_graph::sntree* scene_graph::get_node_tree(const scene_node* node)
{
    if(!node)
        return 0;
    const_iterator i = node_to_iter(node);
    i = i.root();
    if(i == _sntree.get_root())
        return &_sntree;
    if(i ==  _miscs.get_root())
        return &_miscs;
    return 0;
}

const scene_graph::sntree* scene_graph::get_node_tree(const scene_node* node) const
{
    if(!node)
        return 0;
    const_iterator i = node_to_iter(node);
    i = i.root();
    if(i == _sntree.get_root())
        return &_sntree;
    if(i ==  _miscs.get_root())
        return &_miscs;
    return 0;
}

void scene_graph::fix_node(scene_node* old, scene_node* new1)
{
    assert(old && new1);
    if(!old->get_name()[0])
        return;
    assert(old->get_key() == new1->get_key());
    sntable::iterator i = _sntable.find(scene_key(old->get_name()));
    const_cast<scene_key&>(i->first) = scene_key(new1);
    i->second = new1;
}

void scene_graph::map_node(scene_node* node, const gchar* name)
{
    assert(node && name);
    node->set_key(name);
    _sntable.insert(std::make_pair(scene_key(node), node));
}

void scene_graph::unmap_node(scene_node* node)
{
    assert(node);
    if(node && node->get_name()[0])
        _sntable.erase(scene_key(node->get_name()));
}

scene_node* scene_graph::create_node(sntype tag, snptr ptr, fndestroy del)
{
    iterator i = _miscs.insert(_miscs.get_root());
    scene_node* node = i.get_ptr();
    assert(node);
    node->bind(tag, ptr, del);
    return node;
}

scene_node* scene_graph::create_node(const gchar* name, sntype tag, snptr ptr, fndestroy del)
{
    assert(name);
    sntable::iterator i = _sntable.find(scene_key(name));
    if(i != _sntable.end()) {
        assert(!"node already exited.");
        return 0;
    }
    scene_node* node = create_node(tag, ptr, del);
    assert(node);
    map_node(node, name);
    return node;
}

scene_node* scene_graph::attach_node(scene_node* parent, scene_node* node)
{
    assert(node);
    iterator p = parent ? node_to_iter(parent) : _sntree.get_root();
    iterator i = node_to_iter(node);
    sntree* pholder = get_node_tree(parent);
    sntree* holder = get_node_tree(node);
    if(!pholder || !holder) {
        assert(!"one or more holders could not be found.");
        return 0;
    }
    sntree detach;
    holder->detach(detach, i);
    i = pholder->birth(p);
    pholder->attach(detach, i);
    scene_node* new1 = iter_to_node(i);
    assert(new1);
    fix_node(node, new1);
    return new1;
}

scene_node* scene_graph::detach_node(scene_node* node)
{
    assert(node);
    if(!is_node_attached(node)) {
        assert(!"node was unattached.");
        return 0;
    }
    iterator i = node_to_iter(node);
    sntree detach;
    _sntree.detach(detach, i);
    i = _miscs.insert(_miscs.get_root());
    _miscs.attach(detach, i);
    scene_node* new1 = iter_to_node(i);
    assert(new1);
    fix_node(node, new1);
    return new1;
}

void scene_graph::destroy_node(scene_node* node)
{
    assert(node);
    sntree* holder = get_node_tree(node);
    assert(holder && "failed to find holder.");
    if(node->get_name()[0])
        _sntable.erase(scene_key(node->get_name()));
    holder->erase(node_to_iter(node));
}

static void unmap_sntable(scene_graph::iterator i, scene_graph* sg)
{
    if(i->get_name()[0])
        sg->unmap_node(i.get_ptr());
}

void scene_graph::clear_miscs()
{
    _miscs.postorder_traversal([&](scene_graph::wrapper* w) {
        assert(w);
        unmap_sntable(scene_graph::iterator(w), this);
    });
    _miscs.clear();
}

__ariel_end__
