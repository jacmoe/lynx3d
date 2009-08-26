#pragma once

#include "lynx.h"

// #pragma pack(push, 1) // manual padding

struct bspbin_header_t
{
    int magic;
    int version;
};

struct bspbin_direntry_t
{
    DWORD offset;
    DWORD length;
};

struct bspbin_plane_t
{
    plane_t p;
};

struct bspbin_texture_t
{
    char name[64];
};

struct bspbin_node_t
{
    int plane;
    int children[2];
    float radius;
    vec3_t sphere_origin;
};

struct bspbin_leaf_t
{
    int firstpoly;
    int polycount;
};

struct bspbin_poly_t
{
    int tex;
    int firstvertex;
    int vertexcount;
};

struct bspbin_vertex_t
{
    vec3_t v;
    vec3_t n;
    float tu, tv;
};

// #pragma pack(pop)