#include "chunk_tree.hpp"
#include "../../../libs/FastNoise.h"
#include "../vulkan/engine_core.hpp"
#include <iostream>

namespace ec {

uint64_t ChunkTree::id_tracker = 0;

ChunkTree::ChunkTree( int64_t _cx, int64_t _cy, uint64_t _size, int64_t _level, uint32_t _id ) 
    : cx(_cx), cy(_cy), size(_size), level(_level), id(_id)
{

    // Temporary magic number just to try out. 0 will be replaced with the player's position
    int64_t posx = 0;
    int64_t posy = 0;
    int64_t dx = std::abs(posx - cx);
    int64_t dy = std::abs(posy - cy);
    int64_t dist = std::sqrt( dx*dx + dy*dy );
    //int64_t subdivide_dist = 65536/(std::pow(2,level)); == size
    if(_level >= ChunkTree::recursion_limit || _size <= ChunkTree::min_size || ( dist > size ) ) {
        id = id_tracker++;
        tree_size = 1;
        return;
    }
    int index = 0;

    for( int i = -1; i <= 1; i+=2 ) {
        for( int j = -1; j <= 1; j+=2 ) {
            int64_t next_cx = cx+i*static_cast<int64_t>(size/4);
            int64_t next_cy = cy+j*static_cast<int64_t>(size/4);
            int64_t next_size = size/2;
            std::unique_ptr<ChunkTree> next = std::make_unique<ChunkTree>( next_cx, next_cy, next_size, level+1 );
            tree_size += next->tree_size;
            std::swap( subdivide[index++], next );
        }
    }
    
}   

void ChunkTree::get_geometry( std::vector<vertex>& _vertices, std::vector<uint32_t>& _indices, uint32_t _n ) {
    if( this->subdivide[0] == nullptr ) {

        glm::vec3 center = glm::vec3( static_cast<float>( this->cx ), static_cast<float>( this->cy), 0 );
        float size = static_cast<float>( this->size );

        uint32_t id = this->id;

        _vertices.push_back( {center + glm::vec3( -size/2, -size/2, 0 ), {0, 0}, id} );
        _vertices.push_back( {center + glm::vec3( size/2, -size/2, 0 ), {0, 1}, id } );
        _vertices.push_back( {center + glm::vec3( -size/2, size/2, 0 ), {1, 0}, id } );
        _vertices.push_back( {center + glm::vec3( size/2, size/2, 0  ), {1, 1}, id } );

        _indices.push_back( id*4 + 0 );
        _indices.push_back( id*4 + 1 );
        _indices.push_back( id*4 + 2 );
        _indices.push_back( id*4 + 2 );
        _indices.push_back( id*4 + 1 );
        _indices.push_back( id*4 + 3 );

        return;
    }
    else {
        subdivide[0]->get_geometry(_vertices, _indices, ++_n );
        subdivide[1]->get_geometry(_vertices, _indices, ++_n );
        subdivide[2]->get_geometry(_vertices, _indices, ++_n );
        subdivide[3]->get_geometry(_vertices, _indices, ++_n );
    }

}

void ChunkTree::get_noise_views( std::vector<vk::ImageView>& _views ) {
    if( this->subdivide[0] == nullptr ) {
        _views.push_back( noise_texture.get_view() );
    }
    else {
        this->subdivide[0]->get_noise_views(_views);
        this->subdivide[1]->get_noise_views(_views);
        this->subdivide[2]->get_noise_views(_views);
        this->subdivide[3]->get_noise_views(_views);
    }
}

void ChunkTree::set_textures( Texture& _texture ) {
    if( this->subdivide[0] == nullptr ) {
        this->noise_texture = _texture;
        this->noise_texture.create_view( vk::ImageAspectFlagBits::eColor );
    }
    else {
        this->subdivide[0]->set_textures(_texture);
        this->subdivide[1]->set_textures(_texture);
        this->subdivide[2]->set_textures(_texture);
        this->subdivide[3]->set_textures(_texture);
    }
}

void ChunkTree::generate_noise_textures( 
        ec::BaseApp* _current_app
) { 
    if( subdivide[0] != nullptr) {
        this->subdivide[0]->generate_noise_textures( _current_app );
        this->subdivide[1]->generate_noise_textures( _current_app );
        this->subdivide[2]->generate_noise_textures( _current_app );
        this->subdivide[3]->generate_noise_textures( _current_app );
        return;
    }

    size_t noise_size = 128;
    uint index = 0;
    u_char* pixels = new u_char[noise_size * noise_size * 4];
    FastNoise noise;
    noise.SetNoiseType( FastNoise::NoiseType::PerlinFractal );
    noise.SetFractalType( FastNoise::FractalType::Billow );
    noise.SetFractalOctaves(10.0f);
    noise.SetFractalLacunarity(1.8f);
    noise.SetFractalGain(0.5f);
    noise.SetSeed( ChunkTree::seed );
    float center_x = static_cast<float>(cx);
    float center_y = static_cast<float>(cy);
    float size_f = static_cast<float>(size);
    float frequency = 0.1f;
    noise.SetFrequency(frequency);
    float scale = 2.0/255.0; // map [-1;1] to [0;255]
    float ratio = static_cast<float>(this->size)/static_cast<float>(noise_size);
    for( uint i = 0; i < noise_size; i++ ) {
            for( uint j = 0; j < noise_size; j++ ) {
                float u = (i + 0.5f) / 128.0f;
                float v = (j + 0.5f) / 128.0f;

                float world_x = center_x + (u - 0.5f) * size;
                float world_y = center_y + (v - 0.5f) * size;

                float noise_value = noise.GetNoise(
                    world_x * frequency,
                    world_y * frequency
                );
                u_char pixel = static_cast<u_char>(std::clamp( (noise_value+1.0f)/scale, 0.0f, 255.0f ));
                pixels[index++] = pixel;
                pixels[index++] = pixel;
                pixels[index++] = pixel;
                pixels[index++] = 255;
        }
    }
    //std::memset( pixels, 255, size * size * 4);

    noise_texture = Texture( _current_app, pixels, noise_size, noise_size );
    noise_texture.create_view( vk::ImageAspectFlagBits::eColor );

    delete[] pixels;


}

void ChunkTree::clean() {
    if( this->subdivide[0] == nullptr ) {
        noise_texture.clean();
    }
    else {
        this->subdivide[0]->clean();
        this->subdivide[1]->clean();
        this->subdivide[2]->clean();
        this->subdivide[3]->clean();
    }
}
    
} // namespace ec
