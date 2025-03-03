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
    const uint32_t numVertices = (res + 1) * 2 + 2; // Top and bottom circles + center points
    const uint32_t numIndices = res * 6 + res * 6;  // Side faces + top/bottom faces

    float* vert = new float[numVertices * 5];
    uint32_t* indices = new uint32_t[numIndices];

    uint32_t c = 0;
    float angleStep = 2.0f * PI / res;

    // Generate top circle
    vert[c++] = 0.0f;  // Center vertex (x)
    vert[c++] = height / 2.0f; // Center vertex (y)
    vert[c++] = 0.0f;  // Center vertex (z)
    vert[c++] = 0.5f;  
    vert[c++] = 0.5f;  

    for (uint32_t i = 0; i <= res; i++) {
        float angle = i * angleStep;
        vert[c++] = radius * cos(angle);
        vert[c++] = height / 2.0f;
        vert[c++] = radius * sin(angle);
        vert[c++] = (cos(angle) + 1.0f) / 2.0f; // UV u
        vert[c++] = (sin(angle) + 1.0f) / 2.0f; // UV v
    }

    // Generate bottom circle
    vert[c++] = 0.0f;
    vert[c++] = -height / 2.0f;
    vert[c++] = 0.0f;
    vert[c++] = 0.5f;
    vert[c++] = 0.5f;

    for (uint32_t i = 0; i <= res; i++) {
        float angle = i * angleStep;
        vert[c++] = radius * cos(angle);
        vert[c++] = -height / 2.0f;
        vert[c++] = radius * sin(angle);
        vert[c++] = (cos(angle) + 1.0f) / 2.0f;
        vert[c++] = (sin(angle) + 1.0f) / 2.0f;
    }

    // Generate indices for top face
    c = 0;
    for (uint32_t i = 1; i <= res; i++) {
        indices[c++] = 0;
        indices[c++] = i;
        indices[c++] = i + 1;
    }

    // Generate indices for bottom face
    uint32_t bottomCenterIndex = res + 2;
    uint32_t bottomStartIndex = bottomCenterIndex + 1;
    for (uint32_t i = 0; i < res; i++) {
        indices[c++] = bottomCenterIndex;
        indices[c++] = bottomStartIndex + i + 1;
        indices[c++] = bottomStartIndex + i;
    }

    // Generate indices for side faces (triangle strip)
    for (uint32_t i = 1; i <= res; i++) {
        indices[c++] = i;
        indices[c++] = i + 1;
        indices[c++] = bottomStartIndex + i;

        indices[c++] = bottomStartIndex + i;
        indices[c++] = i + 1;
        indices[c++] = bottomStartIndex + i + 1;
    }

    shape s(vert, numVertices * 5, indices, numIndices);
    delete[] vert;
    delete[] indices;
    return s;
}


shape shape::gen_cone(uint32_t res) {
    return shape();
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

shape shape::gen_circle(uint32_t circle_res) {
    return shape();
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

shape shape::gen_tetrahedron() { // 4 sides, each a triangle
    return shape();
}

shape shape::gen_dodecahedron() { // 12 sides, each pentagon
    return shape();
}

shape shape::gen_icosahedron() { // 20 sides, each a triangle
    return shape();
}