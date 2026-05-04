#include "chunk.hpp"

namespace ec {

void ec::Chunk::create_vertices( float divider ) {
    vertices[0] = vertex{ {0, 0, 0}, (glm::vec2(0, 0) + rel_pos)/divider };
    vertices[1] = vertex{ {1, 0, 0}, (glm::vec2(1, 0) + rel_pos)/divider  };
    vertices[2] = vertex{ {0, 1, 0}, (glm::vec2(0, 1) + rel_pos)/divider  };
    vertices[3] = vertex{ {1, 1, 0}, (glm::vec2(1, 1) + rel_pos)/divider };

    indices[0] = 0;
    indices[1] = 1;
    indices[2] = 2;
    indices[3] = 2;
    indices[4] = 1;
    indices[5] = 3;
}


}