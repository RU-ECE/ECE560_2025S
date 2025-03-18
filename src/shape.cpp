#include "shape.hh"
#include "log.hh"
#include <GL/glew.h>
#include <memory>

// utility function to dump vertex data to the screen
#if 0
void dump_vert(float* vert, uint32_t resolution, uint32_t c) {
    if (log::level() > log::INFO) return;
    for (uint32_t i = 0; i < c; i+= 5) {
        log::info("(%f, %f, %f)", vert[i], vert[i+1], vert[i+2]);
    }
    log::info("resolution: %d", resolution);
    log::info("predicted num vert components: %d", resolution*5);  
    log::info("actual num vert components: %d", c);
}   

/*
  utility function to dump index data to the screen, 2 indices per
  for triangle_strip
*/ 
void dump_index(uint32_t* indices, uint32_t resolution, uint32_t c) {
    if (log::level() > log::INFO) return;
    for (uint32_t i = 0; i < c; i+=2) {
        log::info("(%d, %d)", indices[i], indices[i+1]);
    }
}
#endif

shape::shape(const float vert[], const uint32_t vert_size,
            const uint32_t indices[], const uint32_t index_size) {
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vert_size*sizeof(float), vert, GL_STATIC_DRAW);
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_size*sizeof(uint32_t), indices, GL_STATIC_DRAW);
    glBindVertexArray(0);
}


/*
    generate a unit sphere (r=1) with lon_res points around the equator and
    lat_res points above and below the equations (2*lat_res+1)
*/
shape shape::gen_sphere(uint32_t lat_res, uint32_t lon_res) {
    const uint32_t yres = 2*lat_res-1;
    const uint32_t xres = lon_res + 2; // includes + 2 for wrapping back to the start
    const uint32_t resolution = yres*xres;
    const double dlon = 2.0*PI / lon_res, dlat = PI / (2*lat_res);
    double lat = -PI/2 + dlat; // latitude in radians
    float* vert = new float[resolution*5];
    uint32_t c = 0;
    for (uint32_t j = 0; j < yres; j++, lat += dlat) {
        //what is the radius of hte circle at that height?
        double rcircle = cos(lat); // size of the circle at this latitude
        double z = sin(lat); // height of each circle
    
//        log::info("rcircle=%f, z=%f", rcircle, z);
        double t = 0;
        for (uint32_t i = 0; i < lon_res; i++, t += dlon) {
            vert[c++] = rcircle * cos(t),
            vert[c++] = rcircle * sin(t);
            vert[c++] = z;
            vert[c++] = t / (2.0 * PI); // Correct u mapping
            vert[c++] = (lat + PI / 2.0) / PI; // Correct v mapping
        }
    }
    // south pole
    vert[c++] = 0;
    vert[c++] = 0;
    vert[c++] = 1;
    vert[c++] = 0.5;
    vert[c++] = 0;

    // north pole
    vert[c++] = 0;
    vert[c++] = 0;
    vert[c++] = +1;
    vert[c++] = 0.5;
    vert[c++] = 1;

//    dump_vert(vert, resolution*5, c);

    const uint32_t indexSize = resolution * 2;// + (2*lat_res-1) + lon_res; 
    //TODO: North and South Poles aren't used
    uint32_t* indices = new uint32_t[indexSize];
    c = 0;
    for (uint32_t j = 0; j < yres-1; j++) {
        uint32_t startrow = j*lon_res;
        for (uint32_t i = 0; i < lon_res; i++) {
            indices[c++] = startrow + i;
            indices[c++] = startrow + lon_res + i;
        }
        indices[c++] = startrow;
        indices[c++] = startrow + lon_res;
        // Add degenerate triangles to connect strips
        indices[c++] = (j + 1) * lon_res + (lon_res - 1);
        indices[c++] = (j + 1) * lon_res;
    }
//    dump_index(indices, indexSize, c);

    shape s(vert, resolution*5, indices, indexSize);
    delete[] vert;
    delete[] indices;
    return s;
}

shape shape::gen_cube() {
    return shape();
}

/*
    create a cylinder with the number of facets around the circumference
*/
shape shape::gen_cylinder(uint32_t res) {
    const float radius = 1.0f; // Unit cylinder
    const float height = 1.0f;
    // Each circle: 1 center + (res+1) circumference (last duplicates the first)
    const uint32_t numVertices = (res + 1) * 2 + 2; // top + bottom circle vertices
    // Top face: res triangles, Bottom face: res triangles, Side faces: res * 2 triangles (6 indices per segment)
    const uint32_t numIndices = res * 3 + res * 3 + res * 6;  

    float* vert = new float[numVertices * 5];
    uint32_t* indices = new uint32_t[numIndices];

    uint32_t c = 0;
    float angleStep = 2.0f * PI / res;

    // Top circle
    // Top center vertex (index 0)
    vert[c++] = 0.0f;                // x
    vert[c++] = height / 2.0f;         // y
    vert[c++] = 0.0f;                // z
    vert[c++] = 0.5f;                // u
    vert[c++] = 0.5f;                // v

    // Top circumference vertices (indices 1 to res+1, with a duplicate for wrapping)
    for (uint32_t i = 0; i <= res; i++) {
        float angle = i * angleStep;
        vert[c++] = radius * cos(angle);
        vert[c++] = height / 2.0f;
        vert[c++] = radius * sin(angle);
        vert[c++] = (cos(angle) + 1.0f) / 2.0f; // UV u
        vert[c++] = (sin(angle) + 1.0f) / 2.0f; // UV v
    }

    // Bottom circle
    // Bottom center vertex (index = res+2)
    vert[c++] = 0.0f;
    vert[c++] = -height / 2.0f;
    vert[c++] = 0.0f;
    vert[c++] = 0.5f;
    vert[c++] = 0.5f;

    // Bottom circumference vertices (indices res+3 to 2*res+3, with a duplicate for wrapping)
    for (uint32_t i = 0; i <= res; i++) {
        float angle = i * angleStep;
        vert[c++] = radius * cos(angle);
        vert[c++] = -height / 2.0f;
        vert[c++] = radius * sin(angle);
        vert[c++] = (cos(angle) + 1.0f) / 2.0f;
        vert[c++] = (sin(angle) + 1.0f) / 2.0f;
    }

    // Now build indices.
    c = 0;
    // Top face: triangle fan using top center (index 0) and top circumference vertices (indices 1 to res+1)
    for (uint32_t i = 0; i < res; i++) {
        indices[c++] = 0;
        indices[c++] = 1 + i;
        indices[c++] = 1 + i + 1;
    }

    // Bottom face: triangle fan using bottom center (index = res+2) and bottom circumference vertices
    uint32_t bottomCenterIndex = res + 2;
    uint32_t bottomStartIndex = res + 3;
    for (uint32_t i = 0; i < res; i++) {
        indices[c++] = bottomCenterIndex;
        // Note: winding order is reversed so the face normal points downward.
        indices[c++] = bottomStartIndex + i + 1;
        indices[c++] = bottomStartIndex + i;
    }

    // Side faces: each segment forms a quad (2 triangles)
    for (uint32_t i = 0; i < res; i++) {
        // Top vertices are at indices 1+i and 1+i+1
        // Bottom vertices are at indices bottomStartIndex+i and bottomStartIndex+i+1
        uint32_t top1 = 1 + i;
        uint32_t top2 = 1 + i + 1;
        uint32_t bot1 = bottomStartIndex + i;
        uint32_t bot2 = bottomStartIndex + i + 1;

        // First triangle of quad
        indices[c++] = top1;
        indices[c++] = top2;
        indices[c++] = bot1;

        // Second triangle of quad
        indices[c++] = bot1;
        indices[c++] = top2;
        indices[c++] = bot2;
    }

    shape s(vert, numVertices * 5, indices, numIndices);
    delete[] vert;
    delete[] indices;
    return s;
}


shape shape::gen_cone(uint32_t h, uint32_t res) {

    const uint32_t numVertices = res + 2; // bottom circle, center point, top point
    const uint32_t numIndices = res * 3 * 3;  // triangle strip  

    /* Allocate memory */
    float vertices[numVertices * 3]; // leave out texture for now
    uint32_t indices[numIndices];

    /* Start with center of bottom */
    uint32_t cur_idx = 0;
    vertices[cur_idx++] = 0.0f;
    vertices[cur_idx++] = 0.0f;
    vertices[cur_idx++] = 0.0f;

    /* Generate bottom circle points */
    for (uint32_t i = 0; i < res; i++) {
        vertices[cur_idx++] = 0.5 * cos(2.0f * PI * i / res);
        vertices[cur_idx++] = 0.0f;
        vertices[cur_idx++] = 0.5 * sin(2.0f * PI * i / res);
    }

    /* Generate top point */
    vertices[cur_idx++] = 0.0f;
    vertices[cur_idx++] = h;
    vertices[cur_idx++] = 0.0f;

    /* Connect bottom circle */
    for (uint32_t i = 0; i < res; i++) {
        indices[cur_idx++] = 0;
        indices[cur_idx++] = i + 1;
        indices[cur_idx++] = i;
    }

    /* Connect circle to top point */
    for (uint32_t i = 0; i < res; i++) {
        indices[cur_idx++] = i + 1;
        indices[cur_idx++] = res + 1;
        indices[cur_idx++] = i;
    }
    
    return shape(vertices, sizeof(vertices)/sizeof(float), indices, sizeof(indices)/sizeof(uint32_t));
}

/*
    create a torus with a major radius of 1.0, broken into ring_res sections
    around the torus, with the tube radius of radius broken into
    tube_res sections
*/
shape shape::gen_torus(float radius, uint32_t ring_res, uint32_t tube_res) {
    // the angle around the torus
    const auto theta_res = 2*PI / ring_res; 
    // the angle around the tube
    const auto phi_res = 2*PI / tube_res;
    std::unique_ptr<float[]> vert(new float[ring_res * tube_res*5]);
    // vertices
    for (auto i = 0; i < ring_res; i++) {
        for (auto j = 0; j < tube_res; j++) {
            auto theta = i * theta_res;
            auto phi = j * phi_res;
            vert[i * tube_res*5 + j*5] = (radius + cos(phi)) * cos(theta);
            vert[i * tube_res*5 + j*5 + 1] = (radius + cos(phi)) * sin(theta);
            vert[i * tube_res*5 + j*5 + 2] = sin(phi);
            vert[i * tube_res*5 + j*5 + 3] = theta / (2*PI);
            vert[i * tube_res*5 + j*5 + 4] = phi / (2*PI);
        }
    }
    // indices
    uint32_t c = 0;
    std::unique_ptr<uint32_t[]> indices(new uint32_t[ring_res * tube_res * 6]);
    for (auto i = 0; i < ring_res; i++) {
        for (auto j = 0; j < tube_res; j++) {
            uint32_t next_i = (i + 1) % ring_res;
            uint32_t next_j = (j + 1) % tube_res;

            /*
            * i,j ---- (next_i, j)
            * |
            * |
            * | 
            * (i, next_j) ---- (next_i, next_j) 
            */
            
            
            // triangle 1
            indices[c++] = i * tube_res + j;
            indices[c++] = next_i * tube_res + j;
            indices[c++] = i * tube_res + next_j;

            // triangle 2
            indices[c++] = next_i * tube_res + j;
            indices[c++] = next_i * tube_res + next_j;
            indices[c++] = i * tube_res + next_j;
        }
    }

    shape s(vert.get(), ring_res * tube_res*5, nullptr, 0);
    return s;
}

shape shape::gen_grid(uint32_t nx, uint32_t ny) {
    return shape();
}

struct xyzrgb{
    float x, y, z, r, g, b;
};

shape shape::gen_circle(uint32_t circle_res) {
    const float radius = 1.0f;
    const uint32_t numVertices = circle_res + 1; // Center + circumference points
    const uint32_t numIndices = circle_res * 3;  // Each triangle has 3 indices

    xyzrgb* vert = new xyzrgb[numVertices]; // Position (x, y, z) + Color (r, g, b)
    uint32_t* indices = new uint32_t[numIndices];

    uint32_t c = 0;
    float angleStep = 2.0f * PI / circle_res;

    // Center vertex
    vert[c++] = {0,0,0,1,0,0};

    // Circumference vertices
    for (uint32_t i = 0; i < circle_res; i++) {
        float angle = i * angleStep;
        vert[c++] = {cos(angle), sin(angle), 0, 0, 0, 1};
    }

    // Generating triangle fan indices
    c = 0;
    for (uint32_t i = 0; i < circle_res; i++) {
        indices[c++] = 0;             // Center vertex
        indices[c++] = i + 1;         // Current vertex
        indices[c++] = (i + 1) % circle_res + 1; // Next vertex (wrapping around)
    }

    shape s((float*) vert, numVertices * 6, indices, numIndices);
    delete[] vert;
    delete[] indices;
    return s;
}


shape shape::gen_octahedron() { // 8 sides, each a triangle
  const float vertices[] = {
    0.f, 1.f, 0.f, // 0
    1.f, 0.f, 0.f, // 1
    -1.f, 0.f, 0.f, // 2
    0.f, 0.f, 1.f, // 3
    0.f, 0.f, -1.f, // 4
    0.f, -1.f, 0.f // 5
  };
  const uint32_t indices[] = {
    3, 1, 
    0, 4, 
    0, 2, 
    3, 5, 
    1, 5, 
    4, 2
  };
  return shape(vertices, sizeof(vertices)/sizeof(float), indices, sizeof(indices)/sizeof(uint32_t));
}

//Authors: Beomseok Park, Shutong Peng
shape shape::gen_tetrahedron() { // 4 sides, each a triangle
    const float vertices[] = {
        // Vertex positions (x, y, z)
        1.0f,  1.0f,  1.0f,  // Vertex 0
       -1.0f, -1.0f,  1.0f,  // Vertex 1
       -1.0f,  1.0f, -1.0f,  // Vertex 2
        1.0f, -1.0f, -1.0f   // Vertex 3
    };

    const uint32_t indices[] = {
        0, 1, 2, //face 1
        0, 3, 1, //face 2
        0, 2, 3, //face 3
        1, 3, 2 //face 4
    };
    return shape(vertices, sizeof(vertices)/sizeof(float), indices, sizeof(indices)/sizeof(uint32_t));
}

shape shape::gen_dodecahedron() { // 12 sides, each pentagon
    return shape();
}

shape shape::gen_icosahedron() { // 20 sides, each a triangle
    return shape();
}

shape shape::gen_rhombicuboctahedron() { // 26 faces (8 triangles and 18 squares), 24 vertices
    const float x = 3*sqrtf(2)/10;

    const float vertices[] = {
        x, 1.f, x,  -x, 1.f, x,  -x, 1.f,-x,   x, 1.f,-x,  // Top 4
        x,-1.f, x,  -x,-1.f, x,  -x,-1.f,-x,   x,-1.f,-x,  // Bottom 4
       -1.f, x, x,  -1.f,-x, x,  -1.f,-x,-x,  -1.f, x,-x,  // Left 4
        1.f, x, x,   1.f,-x, x,   1.f,-x,-x,   1.f, x,-x,  // Right 4
        x, x, 1.f,   x,-x, 1.f,  -x,-x, 1.f,  -x, x, 1.f,  // Front 4
        x, x,-1.f,   x,-x,-1.f,  -x,-x,-1.f,  -x, x,-1.f   // Back 4
    };

    const uint32_t indices[] = {
        // Squares
        0, 2, 1,  0, 3, 2,  // Top
        5, 7, 4,  5, 6, 7,  // Bottom
        8, 10, 9,  8, 11, 10,  // Left
        13, 15, 12,  13, 14, 15,  // Right
        16, 18, 17,  16, 19, 18,  // Front
        21, 23, 20,  21, 22, 23,  // Back

        // Diagonal squares
        16, 1, 19,  16, 0, 1,   19, 9, 18,   19, 8, 9,  
        18, 4, 17,  18, 5, 4,   17, 12, 16,  17, 13, 12,  
        1, 11, 8,   1, 2, 11,   9, 6, 5,     9, 10, 6,  
        4, 14, 13,  4, 7, 14,   12, 3, 0,    12, 15, 3,  
        3, 23, 2,   3, 20, 23,  11, 22, 10,  11, 23, 22,  
        6, 21, 7,   6, 22, 21,  14, 20, 15,  14, 21, 20,  

        // Triangles
        1, 8, 19,   9, 5, 18,   4, 13, 17,  12, 0, 16,  
        2, 23, 11,  10, 22, 6,  7, 21, 14,  15, 20, 3
    };

    return shape(vertices, sizeof(vertices)/sizeof(float), indices, sizeof(indices)/sizeof(uint32_t));
}

//Authors: Mayank Barad, Nabhan Zaman
shape shape::gen_moebius(float w, int ring_res) {
    // Allocate memory for vertices and indices
    float* vertices = new float[2*ring_res * 3];
    uint32_t* indices = new uint32_t[2*ring_res * 3];

    float r = 1.0f;

    for (uint32_t dir = 0; dir < ring_res; dir++) { // Loop through each direction
        float theta1 = (dir*2*PI)/ring_res;
        float theta2 = ((dir+ring_res)*2*PI)/ring_res;

        float x1 = (r + w*cos(theta1/2) ) * cos(theta1);
        float y1 = (r + w*sin(theta1/2) ) * sin(theta1);
        float z1 = w*sin(theta1/2);

        float x2 = (r + w*cos(theta2/2) ) * cos(theta2);
        float y2 = (r + w*sin(theta2/2) ) * sin(theta2);
        float z2 = w*sin(theta2/2);

        uint32_t idx = dir * 6;
        vertices[idx] = x1;
        vertices[idx+1] = y1;
        vertices[idx+2] = z1;

        vertices[idx+3] = x2;
        vertices[idx+4] = y2;
        vertices[idx+5] = z2;
    }

    for (uint32_t tri = 0; tri < ring_res-1; tri++) { // Triangle indices for surface
        uint32_t idx = tri * 6;

        indices[idx] =   2*tri;
        indices[idx+1] = 2*tri + 1;
        indices[idx+2] = 2*tri + 2;

        indices[idx+3] = 2*tri + 2;
        indices[idx+4] = 2*tri + 1;
        indices[idx+5] = 2*tri + 3;
    }
    
    // Create and return the shape object with vertices and indices
    shape s(vertices, 2*3*ring_res, indices, 2*3*ring_res);
    delete[] vertices;
    delete[] indices;
    return s;
}
