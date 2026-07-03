#ifndef SRC_ENGINE_CLASSES_TERRAIN_HPP
#define SRC_ENGINE_CLASSES_TERRAIN_HPP

#include <stdint.h>
#include <array>
#include "../../../libs/FastNoise.h"
#include "chunk.hpp"

namespace ec {

template<uint32_t size> class Terrain {
public:
    Terrain(){}
    void generate() {
        generate_chunks();
    }
    void get_model( std::vector<ec::vertex>& ret_vertices, std::vector<uint32_t>& ret_indices ) {

        for( int32_t i = 0; i < chunks.size(); i++ ) {
            Chunk& chunk = chunks[i];
            std::array<ec::vertex, 4> vertices = chunk.get_vertices();
            std::array<uint32_t, 6> indices = chunk.get_indices();
            
            for( vertex v : vertices ) {
                ret_vertices.push_back(
                    vertex{ 
                        (v.pos + glm::vec3( chunk.get_pos().x, chunk.get_pos().y, 0 )) * chunk_size,
                        v.tex_coord
                    }
                );
            }
            for( uint32_t index : indices ) {
                ret_indices.push_back( index + 4*chunk.get_index() );
            }

        }
    }

private:
    float chunk_size = 10;
    std::array<Chunk, size * size> chunks;
    void generate_chunks() {
        for( int32_t i = 0; i < size*size; i++ ) {
            Chunk chunk( glm::vec2{ i/size, i%size }, static_cast<float>(size), i );
            
            this->chunks[i] = chunk;

        }
    }

};

} // namespace ec

#endif // SRC_ENGINE_CLASSES_TERRAIN_HPP