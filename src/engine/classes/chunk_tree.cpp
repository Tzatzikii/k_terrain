#include "chunk_tree.hpp"
#include "../../../libs/FastNoise.h"
#include "../vulkan/engine_core.hpp"
#include <iostream>

namespace ec {

void QuadTree::generate( glm::vec3 _eye_pos ) {
    top_node = std::make_shared<QuadTree::Node>(0, 0, 65536);
    update_node( _eye_pos, top_node );
    
}

void QuadTree::update( glm::vec3 _eye_pos ) {
    // for( int i = 0; i < leaves.size(); i++ ) {
    //     auto leaf = leaves[i];
    //     std::cout << i << std::endl;
    //     update_node( _eye_pos, leaf );
    // }
    update_node(_eye_pos, top_node);
}

void QuadTree::update_node( glm::vec3 _eye_pos, std::shared_ptr<QuadTree::Node> _node ) {
    float dx = _eye_pos.x - _node->cx;
    float dy = _eye_pos.y - _node->cy;
    float dist = std::sqrt(dx*dx + dy*dy);
    if( _node->is_divisible( dist ) && !_node->dirty ) {
        _node->subdivide( _eye_pos );
        for( auto child : _node->children ) {
            update_node( _eye_pos, child );
        }
    }
    else if( !_node->is_leaf ){
        _node->collapse(index_pool);
        _node->index = index_pool.get();
        
        if(std::find(leaves.begin(), leaves.end(), _node) == leaves.end()) {
            leaves.push_back(_node);
        }
        
    }

}

void QuadTree::flush_dirty() {
    for( int i = leaves.size() - 1; i >= 0; i--) {
        if(leaves[i]->dirty) {
            index_pool.put(leaves[i]->index);
            leaves[i] = nullptr;
            leaves.erase(leaves.begin() + i);
        }
    }
}

std::vector<ec::QuadTree::LeafInfo> QuadTree::get_leaf_infos() {
    std::vector<ec::QuadTree::LeafInfo> leaf_infos;
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


int QuadTree::is_edge( std::shared_ptr<Node> _node, int _side_index ) {
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

void QuadTree::Node::subdivide( glm::vec3 _eye_pos ) {
    is_leaf = false;

    if( this->has_children() ) {
        this->children[0]->dirty = false;
        this->children[1]->dirty = false;
        this->children[2]->dirty = false;
        this->children[3]->dirty = false;
        return;
    }
    int iter = 0;
    for( int i = -1; i <= 1; i+=2 ) {

        for( int j = -1; j <= 1; j+=2 ) {
            //auto next = create_child(i, j);
            int64_t next_cx = cx+i*static_cast<int64_t>(size/4);
            int64_t next_cy = cy+j*static_cast<int64_t>(size/4);
            int64_t next_size = size/2;
            std::shared_ptr<QuadTree::Node> next = std::make_shared<QuadTree::Node>( next_cx, next_cy, next_size, level+1 );
            children[iter++] = next;
        }
        
    } 
    
}

void QuadTree::Node::collapse( IndexPool& _index_pool ) {
    collapse_branch( _index_pool );
    this->dirty = false; // un-dirt the top of the branch
    this->is_leaf = true;
   
}

void QuadTree::Node::collapse_branch( IndexPool& _index_pool ) {
    if( this->has_children() ) {
        this->children[0]->collapse_branch( _index_pool );
        this->children[1]->collapse_branch( _index_pool );
        this->children[2]->collapse_branch( _index_pool );
        this->children[3]->collapse_branch( _index_pool );
    }
    if(!dirty) {
        dirty = true;
        //_index_pool.put(index);
    }

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
