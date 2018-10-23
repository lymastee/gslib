#include <gslib/error.h>
#include <ariel/mesh.h>

__ariel_begin__

#define mesh_blanks _t(" \t\v\r\n\f")

static bool bad_eof(const string& src, int curr)
{
    if(curr < 0) {
        set_error(_t("An error was already existed.\n"));
        return true;
    }
    if(curr >= src.length()) {
        set_error(_t("Unexpected end of file.\n"));
        return true;
    }
    return false;
}

static int skip_blank_charactors(const string& src, int start)
{
    int p = (int)src.find_first_not_of(mesh_blanks, start);
    if(p == string::npos)
        return src.length();
    return p;
}

static int preread_section_name(const string& src, string& name, int start)
{
    int p = (int)src.find_first_of(mesh_blanks "{", start);
    if(p == string::npos)
        return src.length();
    if(p)
        name.assign(src.c_str() + start, p - start);
    return p;
}

static int enter_section(const string& src, int start)
{
    assert(start < src.length());
    if(src.at(start) == _t('{'))
        return ++ start;
    int next = skip_blank_charactors(src, start);
    if(bad_eof(src, next) || src.at(next) != _t('{'))
        return -1;
    return ++ next;
}

static int read_line_of_section(const string& src, string& line, int start)
{
    assert(start < src.length());
    int p = (int)src.find_first_of(_t("\r\n}"), start);
    if(bad_eof(src, p))
        return -1;
    if(src.at(p) == _t('}')) {
        set_error(_t("Unexpected end of section.\n"));
        return -1;
    }
    line.assign(src, start, p - start);
    return (int)src.find_first_not_of(_t("\r\n"), p);
}

mesh_point_data::mesh_point_data()
{
    _index = mesh_invalid_index;
    _userdata = nullptr;
}

mesh_point_data::~mesh_point_data()
{
    _index = mesh_invalid_index;
    _userdata = nullptr;
}

mesh::mesh()
{
}

mesh::~mesh()
{
    destroy_point_table();
    _triangles.clear();
}

int mesh::load_from_text(const string& src, int start)
{
    assert(start >= 0 && start < src.length());
    int next = enter_section(src, start);
    if(bad_eof(src, next))
        return -1;
    next = skip_blank_charactors(src, start = next);
    if(bad_eof(src, next))
        return -1;
    uint fulfilled = 0;
    enum
    {
        point_section_fulfilled = 0x01,
        face_section_fulfilled = 0x02,
        local_section_fulfilled = 0x04,
        necessary_section_fulfilled = point_section_fulfilled | face_section_fulfilled,
    };
    while(src.at(next) != _t('}')) {
        string prename;
        next = preread_section_name(src, prename, start = next);
        if(bad_eof(src, next))
            return -1;
        /* format start with .Points */
        if(prename == _t(".Points")) {
            next = load_point_section_from_text(src, start = next);
            if(bad_eof(src, next))
                return -1;
            fulfilled |= point_section_fulfilled;
        }
        /* format start with .Faces */
        else if(prename == _t(".Faces")) {
            next = load_face_section_from_text(src, start = next);
            if(bad_eof(src, next))
                return -1;
            fulfilled |= face_section_fulfilled;
        }
        /* format start with .Local */
        else if(prename == _t(".Local")) {
            next = load_local_section_from_text(src, start = next);
            if(bad_eof(src, next))
                return -1;
            fulfilled |= local_section_fulfilled;
        }
        else {
            set_error(_t("Unexpected section.\n"));
            return -1;
        }
        next = skip_blank_charactors(src, start = next);
        if(bad_eof(src, next))
            return -1;
    }
    if((fulfilled & necessary_section_fulfilled) != necessary_section_fulfilled)
        return -1;
    calculate_normals();
    if(!(fulfilled & local_section_fulfilled)) {
        /* setup a default matrix */
        _localmat.identity();
    }
    assert(src.at(next) == _t('}'));
    return ++ next;
}

int mesh::load_local_section_from_text(const string& src, int start)
{
    assert(start >= 0 && start < src.length());
    int next = enter_section(src, start);
    if(bad_eof(src, next))
        return -1;
    next = skip_blank_charactors(src, start = next);
    if(bad_eof(src, next))
        return -1;
    vector<float> read_floats;
    while(src.at(next) != _t('}')) {
        string line;
        next = read_line_of_section(src, line, start = next);
        if(bad_eof(src, next))
            return -1;
        float a, b, c, d;
        int ct = strtool::sscanf(line.c_str(), _t("%f %f %f %f"), &a, &b, &c, &d);
        if(ct != 4) {
            set_error(_t("Bad format about line in section...\n"));
            return -1;
        }
        read_floats.push_back(a);
        read_floats.push_back(b);
        read_floats.push_back(c);
        read_floats.push_back(d);
        /* step on */
        next = skip_blank_charactors(src, start = next);
        if(bad_eof(src, next))
            return -1;
    }
    if(read_floats.size() != 16)
        return -1;
    for(int i = 0, k = 0; i < 4; i ++) {
        for(int j = 0; j < 4; j ++, k ++)
            _localmat.m[i][j] = read_floats.at(k);
    }
    assert(src.at(next) == _t('}'));
    return ++ next;
}

int mesh::load_point_section_from_text(const string& src, int start)
{
    assert(start >= 0 && start < src.length());
    assert(_point_table.empty());
    int next = enter_section(src, start);
    if(bad_eof(src, next))
        return -1;
    next = skip_blank_charactors(src, start = next);
    if(bad_eof(src, next))
        return -1;
    while(src.at(next) != _t('}')) {
        /* the format was: float float float[optional](index)\r\n */
        string line;
        next = read_line_of_section(src, line, start = next);
        if(bad_eof(src, next))
            return -1;
        float x, y, z;
        int c = strtool::sscanf(line.c_str(), _t("%f %f %f"), &x, &y, &z);
        if(c != 3) {
            set_error(_t("Bad format about line in section...\n"));
            return -1;
        }
        /* add to point table */
        auto* v = new mesh_point_data();
        assert(v);
        v->set_point(vec3(x, y, z));
        _point_table.push_back(v);
        /* step on */
        next = skip_blank_charactors(src, start = next);
        if(bad_eof(src, next))
            return -1;
    }
    _point_table.shrink_to_fit();
    arrange_point_indices();
    assert(src.at(next) == _t('}'));
    return ++ next;
}

int mesh::load_face_section_from_text(const string& src, int start)
{
    assert(start >= 0 && start < src.length());
    int next = enter_section(src, start);
    if(bad_eof(src, next))
        return -1;
    next = skip_blank_charactors(src, start = next);
    if(bad_eof(src, next))
        return -1;
    while(src.at(next) != _t('}')) {
        string line;
        next = read_line_of_section(src, line, start = next);
        if(bad_eof(src, next))
            return -1;
        int i, j, k;
        int c = strtool::sscanf(line.c_str(), _t("%d %d %d"), &i, &j, &k);
        if(c != 3) {
            set_error(_t("Bad format about line in section...\n"));
            return -1;
        }
        /* write index */
        _triangles.push_back(i);
        _triangles.push_back(j);
        _triangles.push_back(k);
        /* step on */
        next = skip_blank_charactors(src, start = next);
        if(bad_eof(src, next))
            return -1;
    }
    assert(src.at(next) == _t('}'));
    return ++ next;
}

void mesh::arrange_point_indices()
{
    int size = (int)_point_table.size();
    for(int i = 0; i < size; i ++)
        _point_table.at(i)->set_index(i);
}

void mesh::destroy_point_table()
{
    for(auto* p : _point_table)
        delete p;
    _point_table.clear();
}

void mesh::calculate_normals()
{
    /* set all the normals to 0 */
    for(auto* v : _point_table)
        v->set_normal(vec3(0.f, 0.f, 0.f));
    /* calculate normals for each face, normalize it, add it up to the normals of each point */
    assert(_triangles.size() % 3 == 0);
    for(int m = 0; m < (int)_triangles.size(); m += 3) {
        auto i = _triangles.at(m);
        auto j = _triangles.at(m + 1);
        auto k = _triangles.at(m + 2);
        auto* v1 = _point_table.at(i);
        auto* v2 = _point_table.at(j);
        auto* v3 = _point_table.at(k);
        vec3 normal;
        normal.cross(vec3().sub(v2->get_point(), v1->get_point()), vec3().sub(v3->get_point(), v2->get_point())).normalize();
        v2->acc_normal(normal);
        v1->acc_normal(normal);
        v3->acc_normal(normal);
    }
    /* normalize each of the normals */
    for(auto* v : _point_table)
        v->normalize_normal();
}

bool mesh_loader_for_test::load_from_text(const string& src)
{
    int start = skip_blank_charactors(src, 0);
    if(bad_eof(src, start))
        return false;
    string prename;
    int next = preread_section_name(src, prename, start);
    if(bad_eof(src, next))
        return false;
    do {
        if(prename == _t("Polygon")) {
            mesh* ptr = new mesh;
            next = ptr->load_from_text(src, next);
            if(next == -1) {
                delete ptr;
                return false;
            }
            on_add_mesh(ptr);
        }
        else {
            set_error(_t("Unknown section name.\n"));
            return false;
        }
        next = skip_blank_charactors(src, next);
        if(next >= src.length())
            break;
        next = preread_section_name(src, prename, start = next);
    }
    while(next < src.length());
    return true;
}

__ariel_end__
