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

#ifndef mesh_7c5c4ae5_0f3e_4a2d_936e_0447239ac3a7_h
#define mesh_7c5c4ae5_0f3e_4a2d_936e_0447239ac3a7_h

#include <ariel/config.h>
#include <gslib/type.h>
#include <gslib/std.h>

__ariel_begin__

template<class pt_type>
struct mesh_joint;
template<class pt_type>
struct mesh_edge;
template<class pt_type>
struct mesh_face;

template<class pt_type>
using mesh_joint_list = vector<mesh_joint<pt_type> >;
template<class pt_type>
using mesh_edge_list = vector<mesh_edge<pt_type> >;
template<class pt_type>
using mesh_face_list = vector<mesh_face<pt_type> >;

enum mesh_joint_type
{
    jt_turn,
    jt_fork,
};

template<class pt_type>
struct mesh_common_namespace
{
public:
    typedef pt_type point_type;
    typedef mesh_joint<pt_type> joint_type;
    typedef mesh_edge<pt_type> edge_type;
    typedef mesh_face<pt_type> face_type;
    typedef mesh_joint_list<pt_type> joint_list;
    typedef mesh_edge_list<pt_type> edge_list;
    typedef mesh_face_list<pt_type> face_list;
};

template<class pt_type>
struct mesh_joint:
    public mesh_common_namespace<pt_type>
{
public:
    mesh_joint(const pt_type& p) { set_point(p); }
    virtual ~mesh_joint() {}
    virtual mesh_joint_type get_type() const = 0;
    virtual int get_edge_count() const = 0;
    virtual edge_type* get_edge(int i) const = 0;

public:
    const pt_type& get_point() const { return _point; }
    void set_point(const pt_type& p) { _point = p; }

protected:
    pt_type             _point;
};

template<class pt_type>
struct mesh_turn_joint:
    public mesh_joint<pt_type>
{
public:
    mesh_turn_joint(const pt_type& p): mesh_joint(p) { _edges[0] = _edges[1] = 0; }
    virtual mesh_joint_type get_type() const override { return jt_turn; }
    virtual int get_edge_count() const override { return 2; }
    virtual edge_type* get_edge(int i) const override
    {
        assert(i < 2);
        return _edges[i];
    }

protected:
    edge_type*          _edges[2];

public:
    edge_type* get_prev_edge() const;
    edge_type* get_next_edge() const;
};

template<class pt_type>
struct mesh_fork_joint:
    public mesh_joint<pt_type>
{
public:
    mesh_fork_joint(const pt_type& p): mesh_joint(p) {}
    virtual mesh_joint_type get_type() const override { return jt_fork; }
    virtual int get_edge_count() const override { return jt_fork; }
    virtual edge_type* get_edge(int i) const override { return _edges.at(i); }

protected:
    edge_list           _edges;
};

template<class pt_type>
struct mesh_edge:
    public mesh_common_namespace<pt_type>
{
public:
    mesh_edge() { _joints[0] = _joints[1] = 0; }
    mesh_edge(joint_type* j1, joint_type* j2)
    {
        _joints[0] = j1;
        _joints[1] = j2;
    }
    joint_type* get_joint(int i) const
    {
        assert(i >= 0 && i < 2);
        return _joints[i];
    }
    void set_joint(int i, joint_type* p)
    {
        assert(i >= 0 && i < 2);
        _joints[i] = p;
    }
    joint_type* get_prev_joint() const { return _joints[0]; }
    joint_type* get_next_joint() const { return _joints[1]; }
    const pt_type& get_prev_point() const;
    const pt_type& get_next_point() const;

protected:
    joint_type*         _joints[2];

private:
    static const pt_type nonsense_point;
};

template<class pt_type>
struct mesh_face:
    public mesh_common_namespace<pt_type>
{
public:
    void add_joint(joint_type* p) { _joints.push_back(p); }


protected:
    joint_list          _joints;
};

template<class pt_type>
class mesh:
    public mesh_common_namespace<pt_type>
{
public:
    mesh();
    virtual ~mesh();

protected:
    joint_list          _joint_holdings;
    edge_list           _edge_holdings;
    face_list           _faces;
};

__ariel_end__

#include <ariel/mesh.hpp>

#endif