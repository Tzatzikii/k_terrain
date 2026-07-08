#include "quadtree.hpp"
#include "../../../libs/FastNoise.h"
#include "../vulkan/engine_core.hpp"
#include <iostream>

namespace ec {

void Quadtree::generate( glm::vec3 _eye_pos ) {

    top_node = std::make_shared<Quadtree::Node>(0, 0, 65536);
    update_node( _eye_pos, top_node );
    
}

void Quadtree::update( glm::vec3 _eye_pos ) {
    update_node(_eye_pos, top_node);
}

void Quadtree::update_node( glm::vec3 _eye_pos, std::shared_ptr<Quadtree::Node> _node ) {

    float dx = _eye_pos.x - _node->cx;
    float dy = _eye_pos.y - _node->cy;
    float dist = std::sqrt(dx*dx + dy*dy);

    if( _node->is_divisible( dist ) ) {

        _node->subdivide( _eye_pos );

        for( auto child : _node->children ) {
            update_node( _eye_pos, child );
        }

    }

    else if( _node->leaf ){

        _node->collapse(index_pool);

        if(_node->index == -1) {
            _node->index = index_pool.get();
        }
        
        if(std::find(cache.begin(), cache.end(), _node) == cache.end()) {
            cache.push_back(_node);
        }
        
    }

}

void Quadtree::clear_cache() {

    for( int32_t i = cache.size() - 1; i >= 0; i--) {

        if(cache[i]->leaf) {

            index_pool.put(cache[i]->index);
            cache[i] = nullptr;
            cache.erase(cache.begin() + i);

        }

    }

}

std::vector<ec::Quadtree::LeafInfo> Quadtree::get_leaf_infos() {

    std::vector<ec::Quadtree::LeafInfo> leaf_infos;

    for( auto node : cache ) {

        if( !node->leaf ){ continue; }
        LeafInfo info;
        info.pos = glm::vec2( node->cx, node->cy );
        info.size = node->size;
        info.index = node->index;

        for( int32_t i = 0; i < 4; i++ ) {
            info.tess_edges[i] = is_edge(node, i);    
        }

        leaf_infos.push_back(info);
    }

    static auto log = leaf_infos.size();

    if( leaf_infos.size() != log ) {
        //std::cout << leaf_infos.size() << " / " << leaves.size() << std::endl;
        log = leaf_infos.size();
    }

    if( cache.size() > leaf_infos.size() * 4 ) {
        clear_cache();
    }

    return leaf_infos;

}

// Returns the tessellation factor relative to _node for all neighboring nodes 
int32_t Quadtree::is_edge( std::shared_ptr<Node> _node, int32_t _side_index ) {

    auto closest = cache[0];
    glm::vec2 closest_pos = { closest->cx, closest->cy };
    glm::vec2 node_mod = {0, 0};
    glm::vec2 cmp_mod = {0, 0};
    
    switch(_side_index) {
        case 0: {
            node_mod.x=-1;
            cmp_mod.x=1;
            break;
        }
        case 1: {
            node_mod.y+=-1;
            cmp_mod.y=1;
            break;
        }
        case 2: {
            node_mod.x=1;
            cmp_mod.x=-1;
            break;
        }
        case 3: {
            node_mod.y=1;
            cmp_mod.y=-1;
            break;
        }
    }
    // lets pray to heavens this somehow doesnt get larger than INT64_MAX, however i can hardly imagine that
    glm::vec2 node_sc = glm::vec2(_node->cx, _node->cy) + node_mod * static_cast<float>(_node->size/2); // node side-center
    glm::vec2 cmp_sc = closest_pos + cmp_mod * static_cast<float>(closest->size/2); // node compare side-center
    glm::vec2 diff = node_sc - cmp_sc;
    float closest_dist = glm::length(diff);
    
    // side indices are based on the vulkan tessellation indexing
    // https://docs.vulkan.org/spec/latest/chapters/tessellation.html
    for( auto node : cache ) {
        if(node == _node || !node->leaf) { continue; }
        glm::vec2 pos = {node->cx, node->cy};
        cmp_sc = pos + cmp_mod * static_cast<float>(node->size/2);
        diff = node_sc - cmp_sc;
        float dist = glm::length(diff);
        if( dist < closest_dist ) {
            closest = node;
            closest_dist = dist;
        }
    }
    return std::max(1.0f, (float)_node->size / (float)closest->size);
}

void Quadtree::Node::subdivide( glm::vec3 _eye_pos ) {
    leaf = false;

    if( this->has_children() ) {
        this->children[1]->leaf = true;
        this->children[2]->leaf = true;
        this->children[3]->leaf = true;
        this->children[0]->leaf = true;
        return;
    }
    int32_t iter = 0;
    for( int32_t j = -1; j <= 1; j+=2 ) {
    for( int32_t i = -1; i <= 1; i+=2 ) {

            //auto next = create_child(i, j);
            int64_t next_cx = cx+i*static_cast<int64_t>(size/4);
            int64_t next_cy = cy+j*static_cast<int64_t>(size/4);
            int64_t next_size = size/2;
            std::shared_ptr<Quadtree::Node> next = std::make_shared<Quadtree::Node>( next_cx, next_cy, next_size, level+1 );
            children[iter++] = next;
        }
        
    } 
    
}

void Quadtree::Node::collapse( IndexPool& _index_pool ) {
    collapse_branch( _index_pool );
    leaf = true;
}

void Quadtree::Node::collapse_branch( IndexPool& _index_pool ) {
    if( this->has_children() ) {
        this->children[0]->collapse_branch( _index_pool );
        this->children[1]->collapse_branch( _index_pool );
        this->children[2]->collapse_branch( _index_pool );
        this->children[3]->collapse_branch( _index_pool );
    }
    leaf = false;
}

float debug_pattern(int32_t x, int32_t y,
                    int32_t width,
                    int32_t height)
{
    float nx =
        (float)x / width;

    float ny =
        (float)y / height;

    float cx = nx * 2.0f - 1.0f;
    float cy = ny * 2.0f - 1.0f;

    float dist =
        std::sqrt(cx * cx + cy * cy);

    float radial =
        std::sin(dist * 50.0f);

    float gradientX = nx;
    float gradientY = ny;

    float result =
        radial * 0.5f +
        gradientX * 0.25f +
        gradientY * 0.25f;

    return std::clamp(result, 0.0f, 1.0f);
}

void Quadtree::Node::calculate_noise( uint8_t* _dest ) {
    size_t noise_size = 65;
    FastNoise noise;

    noise.SetNoiseType( FastNoise::NoiseType::PerlinFractal );
    noise.SetFractalType( FastNoise::FractalType::Billow );
    noise.SetFractalOctaves(9.0f);
    noise.SetFractalLacunarity(1.8f);
    noise.SetFractalGain(0.5f);
    noise.SetSeed( Quadtree::Node::seed );

    float center_x = static_cast<float>(cx);
    float center_y = static_cast<float>(cy);
    float size_f = static_cast<float>(size);
    float frequency = 0.1f;

    noise.SetFrequency(frequency);

    float scale = 2.0/255.0; // map [-1;1] to [0;255]
    float ratio = static_cast<float>(this->size)/static_cast<float>(noise_size);
    
    uint32_t pixel_index = 0;
    for( uint32_t i = 0; i < noise_size; i++ ) {
            for( uint32_t j = 0; j < noise_size; j++ ) {
                float u = (j) / static_cast<float>(noise_size-1);
                float v = (i) / static_cast<float>(noise_size-1);

                float world_x = center_x + (u - 0.5f) * size;
                float world_y = center_y + (v - 0.5f) * size;

                float noise_value = noise.GetNoise(
                    world_x * frequency,
                    world_y * frequency
                );
               // float noise_value = std::sqrt(world_x*world_x+world_y*world_y)/2048;
                uint8_t pixel = static_cast<uint8_t>(std::clamp( (noise_value+1.0f)/scale, 0.0f, 255.0f ));
                assert(pixel >= 0 && pixel <= 255);
                _dest[pixel_index++] = pixel;
                _dest[pixel_index++] = pixel;
                _dest[pixel_index++] = pixel;
                _dest[pixel_index++] = 255;
        }
    }
    ////std::cout << "color:" << ((static_cast<float>(index)/(255.0))) << std::endl;
    //std::memset( _dest, static_cast<char>((static_cast<float>(index))), noise_size * noise_size * 4);
    //std::memset( _dest, static_cast<uint8_t>(100), noise_size * noise_size * 4 );
}


Texture Quadtree::create_noise_texture( BaseApp* _current_app ) {
    size_t noise_size = 65;
    size_t texture_size = noise_size * noise_size * 4;
    uint8_t* pixels = new uint8_t[texture_size * 4096]; // ~7 megabytes

    Texture noise_texture = Texture( _current_app, pixels, noise_size, noise_size, 2048 );
    
    delete[] pixels;
    return noise_texture;
}


void Quadtree::update_noise_texture( BaseApp* _current_app, Texture& _noise_texture ) {
    size_t noise_size = 65;
    size_t texture_size = noise_size * noise_size * 4;
    uint8_t* noise = new uint8_t[texture_size];

    auto b = _noise_texture.begin_write();
    for( auto node : cache ) {
        if( node->has_noise ) {continue;}
        node->calculate_noise( noise );
        _noise_texture.write( b, node->index*texture_size, noise, texture_size );
        node->has_noise = true;
    }
    _noise_texture.end_write(b);
    delete[] noise;

}

    
} // namespace ec
