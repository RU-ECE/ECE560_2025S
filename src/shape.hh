#pragma once
#include <cstdint>
#define _USE_MATH_DEFINES
#include <cmath>

constexpr double PI = M_PI; 
/**
* Shape class
* static methods to create shapes and return the integer handle to the 
* vertexes and indices
*/

class shape {
public:
    uint32_t vao;
    uint32_t vbo;
    uint32_t ibo;
    uint32_t indexSize;
    static shape gen_sphere(uint32_t lat_res, uint32_t lon_res);
    static shape gen_octahedron(); // 8 sides, each a triangle
    static shape gen_cube();
    static shape gen_tetrahedron(); // 4 sides, each a triangle
    static shape gen_dodecahedron(); // 12 sides, each pentagon
    static shape gen_icosahedron(); // 20 sides, each a triangle
    static shape gen_cylinder(uint32_t ring_res);
    static shape gen_cone(uint32_t ring_res);
    static shape gen_torus(float tube_radius, uint32_t ring_res, uint32_t tube_resolution);
    static shape gen_grid(uint32_t gridX, uint32_t gridY);
    static shape gen_circle(uint32_t circle_res); // filled circle

    shape() : vao(0), vbo(0), ibo(0) {}
    shape(const float vert[], const uint32_t vert_size,
            const uint32_t indices[], const uint32_t index_size);
// each render method should have an associated precomputed shader program
// should not be needed by the caller
    void render_textured(uint32_t texture_id);
    void render_colored();
};
