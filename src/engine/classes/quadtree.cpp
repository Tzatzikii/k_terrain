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

    if( _node->is_divisible( dist ) && !_node->dirty ) {

        _node->subdivide( _eye_pos );

        for( auto child : _node->children ) {
            update_node( _eye_pos, child );
        }

    }

    else if( !_node->is_leaf && !_node->dirty ){

        _node->collapse(index_pool);

        if(_node->index == -1) {
            _node->index = index_pool.get();
        }
        
        if(std::find(leaves.begin(), leaves.end(), _node) == leaves.end()) {
            leaves.push_back(_node);
        }
        
    }

}

void Quadtree::flush_dirty() {

    for( int i = leaves.size() - 1; i >= 0; i--) {

        if(leaves[i]->dirty) {

            index_pool.put(leaves[i]->index);
            leaves[i] = nullptr;
            leaves.erase(leaves.begin() + i);

        }

    }

}

std::vector<ec::Quadtree::LeafInfo> Quadtree::get_leaf_infos() {

    std::vector<ec::Quadtree::LeafInfo> leaf_infos;

    for( auto leaf : leaves ) {

        if(leaf->dirty || !leaf->is_leaf) continue; // temporary
        LeafInfo info;
        info.pos = glm::vec2( leaf->cx, leaf->cy );
        info.size = leaf->size;
        info.index = leaf->index;

        for( int i = 0; i < 4; i++ ) {
            info.tess_edges[i] = is_edge(leaf, i);    
        }

        leaf_infos.push_back(info);
    }

    static auto log = leaf_infos.size();

    if( leaf_infos.size() != log ) {
        std::cout << leaf_infos.size() << " / " << leaves.size() << std::endl;
        log = leaf_infos.size();
    }

    if( leaves.size() > leaf_infos.size() * 4 ) {
        flush_dirty();
    }

    return leaf_infos;

}


int Quadtree::is_edge( std::shared_ptr<Node> _node, int _side_index ) {

    auto closest = leaves[0];
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
            node_mod.y+=1;
            cmp_mod.y=-1;
            break;
        }
        case 2: {
            node_mod.x=1;
            cmp_mod.x=-1;
            break;
        }
        case 3: {
            node_mod.y=-1;
            cmp_mod.y=1;
            break;
        }
    }
    // lets pray to heavens this somehow doesnt get larger than INT64_MAX, however i can hardly imagine that
    glm::vec2 node_sc = glm::vec2(_node->cx, _node->cy) + node_mod * static_cast<float>(_node->size/2);
    glm::vec2 cmp_sc = closest_pos + cmp_mod * static_cast<float>(closest->size/2);
    glm::vec2 diff = node_sc - cmp_sc;
    float closest_dist = glm::length(diff);
    
    // side indices are based on the vulkan tessellation indexing
    // https://docs.vulkan.org/spec/latest/chapters/tessellation.html
    for( auto node : leaves ) {
        if(node == _node || !_node->is_leaf || _node->dirty) continue;
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
    is_leaf = false;

    if( this->has_children() ) {
        this->children[0]->dirty = false;
        this->children[1]->dirty = false;
        this->children[2]->dirty = false;
        this->children[3]->dirty = false;
        return;
    }
    int iter = 0;
    for( int j = -1; j <= 1; j+=2 ) {
    for( int i = -1; i <= 1; i+=2 ) {

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
    this->dirty = false; // un-dirt the top of the branch
    this->is_leaf = true;
   
}

void Quadtree::Node::collapse_branch( IndexPool& _index_pool ) {
    if( this->has_children() ) {
        this->children[0]->collapse_branch( _index_pool );
        this->children[1]->collapse_branch( _index_pool );
        this->children[2]->collapse_branch( _index_pool );
        this->children[3]->collapse_branch( _index_pool );
    }
    if(!dirty) {
        dirty = true;
    }

}

float debug_pattern(int x, int y,
                    int width,
                    int height)
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

void QuadTree::Node::calculate_noise( uint8_t* _dest ) {
    size_t noise_size = 64;
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
                float u = (j + 0.5f) / static_cast<float>(noise_size);
                float v = (i + 0.5f) / static_cast<float>(noise_size);

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
    //std::cout << "color:" << ((static_cast<float>(index)/(255.0))) << std::endl;
    //std::memset( _dest, static_cast<char>((static_cast<float>(index))), noise_size * noise_size * 4);
}


Texture QuadTree::create_noise_texture( BaseApp* _current_app ) {
    uint8_t* pixels = new uint8_t[64 * 64 * 4 * 4096]; // ~7 megabytes

    Texture noise_texture = Texture( _current_app, pixels, 64, 64, 2048 );
    
    delete[] pixels;
    return noise_texture;
}


void QuadTree::update_noise_texture( BaseApp* _current_app, Texture& _noise_texture ) {
    uint8_t* noise = new uint8_t[64 * 64 * 4];

    auto b = _noise_texture.begin_write();
    for( auto node : leaves ) {
        if( node->has_noise ) {continue;}
        node->calculate_noise( noise );
        _noise_texture.write( b, node->index*64*64*4, noise, 64*64*4);
        node->has_noise = true;
    }
    _noise_texture.end_write(b);
    delete[] noise;

}

    
} // namespace ec
