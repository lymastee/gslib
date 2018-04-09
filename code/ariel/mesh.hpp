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

/*
 * This file was the implementation of mesh.h
 * DONOT include this file outside.
 * include mesh.h instead.
 */

__ariel_begin__

template<class pt_type>
typename mesh_turn_joint<pt_type>::edge_type* mesh_turn_joint<pt_type>::get_prev_edge() const
{
    if(_edges[0]) {
        assert(_edges[0]->get_next_point() == get_point());
        return _edges[0];
    }
    else {
        assert(!"unexpected.");
        return nullptr;
    }
}

template<class pt_type>
typename mesh_turn_joint<pt_type>::edge_type* mesh_turn_joint<pt_type>::get_next_edge() const
{
    if(_edges[1]) {
        assert(_edges[1]->get_prev_point() == get_point());
        return _edges[1];
    }
    else {
        assert(!"unexpected.");
        return nullptr;
    }
}

template<class pt_type>
const pt_type& mesh_edge<pt_type>::get_prev_point() const
{
    return _joints[0] ? _joints[0]->get_point() : nonsense_point;
}

template<class pt_type>
const pt_type& mesh_edge<pt_type>::get_next_point() const
{
    return _joints[1] ? _joints[1]->get_point() : nonsense_point;
}

__ariel_end__
