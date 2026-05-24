#include "chunk_tree.hpp"
#include "../../../libs/FastNoise.h"
#include "../vulkan/engine_core.hpp"
#include <iostream>

namespace ec {




// QuadTree::QuadTree( int64_t _cx, int64_t _cy, uint64_t _size, int64_t _level, uint32_t _id ) 
//     : cx(_cx), cy(_cy), size(_size), level(_level), index(_id)
// {
    
// }  

void QuadTree::Node::subdivide( glm::vec3 _eye_pos ) {

    int index = 0;

    for( int i = -1; i <= 1; i+=2 ) {

        for( int j = -1; j <= 1; j+=2 ) {
            //auto next = create_child(i, j);
            int64_t next_cx = cx+i*static_cast<int64_t>(size/4);
            int64_t next_cy = cy+j*static_cast<int64_t>(size/4);
            int64_t next_size = size/2;
            std::unique_ptr<QuadTree::Node> next = std::make_unique<QuadTree::Node>( next_cx, next_cy, next_size, level+1 );
            std::swap( children[index++], next );
        }
        
    } 
    
}

// std::unique_ptr<QuadTree> QuadTree::create_child( int _local_i, int _local_j ) {

//     int64_t next_cx = cx+_local_i*static_cast<int64_t>(size/4);
//     int64_t next_cy = cy+_local_j*static_cast<int64_t>(size/4);
//     int64_t next_size = size/2;
//     std::unique_ptr<QuadTree> next = std::make_unique<QuadTree>( next_cx, next_cy, next_size, level+1 );
//     return next;
    
// }   


// void QuadTree::get_noise_views( std::vector<vk::ImageView>& _views ) {
//     if( this->is_leaf() ) {
//         _views.push_back( noise_texture.get_view() );
//     }
//     else {
//         this->children[0]->get_noise_views(_views);
//         this->children[1]->get_noise_views(_views);
//         this->children[2]->get_noise_views(_views);
//         this->children[3]->get_noise_views(_views);
//     }
// }

// void QuadTree::set_textures( Texture& _texture ) {
//     if( this->is_leaf() ) {
//         this->noise_texture = _texture;
//         this->noise_texture.create_view( vk::ImageAspectFlagBits::eColor );
//     }
//     else {
//         this->children[0]->set_textures(_texture);
//         this->children[1]->set_textures(_texture);
//         this->children[2]->set_textures(_texture);
//         this->children[3]->set_textures(_texture);
//     }
// }

// void QuadTree::update( glm::vec3 _eye_pos ) {
//     bool changed = false;
//     glm::vec3 pos = _eye_pos;
 
//     float dx = std::abs(pos.x - cx);
//     float dy = std::abs(pos.y - cy);
//     float dist = std::sqrt( dx*dx + dy*dy );
//    // float threshold = size * 4;

//     if( !this->is_divisible( dist ) ) {
//         this->make_leaf();
        
//         return;
//     }
//     this->subdivide( pos );
//     this->update_children( pos );
//     this->update_tree_size();
// }

// void QuadTree::update_tree_size() {
//     if( this->has_children() ) {
//         leaf_count = 0;
//         for( auto& child : children ) {
//             child->update_tree_size();
//             leaf_count += child->leaf_count;
//         }
//     }
//     // if a chunk doesn't have children it is a leaf, in
//     // which case the ChunkTree::make_leaf function assigns
//     // a leaf_count of 1, so no need to handle that again
// }

// void QuadTree::update_children( glm::vec3 _eye_pos ) {
//     if( this->is_leaf() ) {
//         return;
//     }
//     for( auto& child : children ) {
//         child->update( _eye_pos );
//     }
//     std::cout << "made leaf, coords: " << cx << " : " << cy << std::endl;
// }

// void QuadTree::delete_children() {

//     if( !this->has_children() ) {
//         return;
//     }

//     for( int i = 0; i < 4; i++ ) {
//         children[i]->clean();
//         children[i].release();
//     } 

// }

void QuadTree::Node::make_leaf( uint32_t _new_index ) {

    //this->delete_children();
    this->dirty = true;
    index = _new_index;
    //leaf_count = 1;
}

void QuadTree::Node::calculate_noise( u_char* _dest ) {
    size_t noise_size = 128;
    FastNoise noise;

    noise.SetNoiseType( FastNoise::NoiseType::PerlinFractal );
    noise.SetFractalType( FastNoise::FractalType::Billow );
    noise.SetFractalOctaves(9.0f);
    noise.SetFractalLacunarity(1.8f);
    noise.SetFractalGain(0.5f);
    noise.SetSeed( QuadTree::Node::seed );

    float center_x = static_cast<float>(cx);
    float center_y = static_cast<float>(cy);
    float size_f = static_cast<float>(size);
    float frequency = 0.1f;

    noise.SetFrequency(frequency);

    float scale = 2.0/255.0; // map [-1;1] to [0;255]
    float ratio = static_cast<float>(this->size)/static_cast<float>(noise_size);
    
    uint index = 0;
    for( uint i = 0; i < noise_size; i++ ) {
            for( uint j = 0; j < noise_size; j++ ) {
                float u = (i + 0.5f) / static_cast<float>(noise_size);
                float v = (j + 0.5f) / static_cast<float>(noise_size);

                float world_x = center_x + (u - 0.5f) * size;
                float world_y = center_y + (v - 0.5f) * size;

                float noise_value = noise.GetNoise(
                    world_x * frequency,
                    world_y * frequency
                );
                u_char pixel = static_cast<u_char>(std::clamp( (noise_value+1.0f)/scale, 0.0f, 255.0f ));
                assert(pixel >= 0 && pixel <= 255);
                _dest[index++] = pixel;
                _dest[index++] = pixel;
                _dest[index++] = pixel;
                _dest[index++] = 255;
        }
    }
    std::memset( _dest, static_cast<char>(0), noise_size * noise_size * 4);
}

Texture QuadTree::generate_noise_texture( BaseApp* _current_app ) {
    u_char* pixels = new u_char[leaf_count * 4];


    
    delete[] pixels;
}

    
} // namespace ec
