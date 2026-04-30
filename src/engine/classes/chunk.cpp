#include "chunk.hpp"

namespace ec {

void ec::Chunk::create_vertices() {
    vertices[0] = vertex{ {0, 0, 0}, {0, 0} };
    vertices[1] = vertex{ {1, 0, 0}, {1, 0} };
    vertices[2] = vertex{ {0, 1, 0}, {0, 1} };
    vertices[3] = vertex{ {1, 1, 0}, {1, 1} };

    indices[0] = 0;
    indices[1] = 1;
    indices[2] = 2;
    indices[3] = 2;
    indices[4] = 1;
    indices[5] = 3;
}


}