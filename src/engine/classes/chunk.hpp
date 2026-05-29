#ifndef SRC_ENGINE_CLASSES_CHUNK_HPP
#define SRC_ENGINE_CLASSES_CHUNK_HPP

#include <array>
#include "../classes/vertex.hpp"

namespace ec {

class Chunk {
public:
    Chunk( glm::vec2 rel_pos = glm::vec2(0, 0), float divider = 1, uint32_t rel_index = 0 ) : rel_pos( rel_pos ), rel_index( rel_index ){
        create_vertices( divider );
    }    
    glm::vec2 get_pos() { return rel_pos; }
    uint32_t get_index() { return rel_index; }
    std::array<ec::vertex, 4> get_vertices() { return std::array<ec::vertex, 4>(vertices); }
    std::array<uint32_t, 6> get_indices() { return std::array<uint32_t, 6>(indices); }
    Chunk operator=( Chunk& other ) {
        rel_pos = other.rel_pos;
        rel_index = other.rel_index;
        this->vertices = other.vertices;
        this->indices = other.indices;
        return *this;
    }

private:
    glm::vec2 rel_pos;
    uint32_t rel_index;
    std::array<ec::vertex, 4> vertices;
    std::array<uint32_t, 6> indices;
    void create_vertices( float divider );
};

} // namespace ec

#endif // SRC_ENGINE_CLASSES_CHUNK_HPP