#ifndef RES_SRC_ENGINE_CLASSES_CHUNKTREE_HPP
#define RES_SRC_ENGINE_CLASSES_CHUNKTREE_HPP

#include <memory>
#include <stdint.h>
#include <vector>
#include "../math/vertex.hpp"
#include "../etc/header_libs.hpp"
#include "../vulkan/texture.hpp"

namespace ec {

class BaseApp;    

class QuadTree {
private:
    std::unique_ptr<QuadTree> children[4] = { nullptr };
    static const int64_t recursion_limit = 20;
    static const int64_t min_size = 16;
    static const uint64_t seed = 727;
    static uint64_t id_tracker;
    int64_t level = 0;
    int64_t cx, cy;
    uint64_t size;
    uint32_t id;
    uint64_t leaf_count = 0;
    Texture noise_texture;
    bool updated = false;
    
public:
    QuadTree( int64_t _cx, int64_t _cy, uint64_t _size = INT64_MAX*2, int64_t _level = 0, uint32_t _id = 0 );
    glm::vec2 get_center() { return glm::vec2( static_cast<float>(cx), static_cast<float>(cy) ); }
    void get_geometry( std::vector<vertex>& _vertices, std::vector<uint32_t>& _indices, uint32_t _n );

    uint64_t get_leaf_count() { return leaf_count; }

    void get_noise_views( std::vector<vk::ImageView>& _views );
    void set_textures( Texture& texture );
    void generate_noise_textures(
        BaseApp* _current_app
    );
    void update( glm::vec3 _eye_pos );
    void subdivide( glm::vec3 _eye_pos );
    void clean();

    float divide_threshold() { 
        return size; 
    }

    bool is_divisible( float dist_from_eye ) {
        return !( 
            ( this->level >= QuadTree::recursion_limit )   || 
            ( this->size <= QuadTree::min_size )           || 
            ( dist_from_eye > divide_threshold() )
        );
    }

    void delete_children();

    void update_tree_size();

    void update_children( glm::vec3 _eye_pos );

    void make_leaf();

    bool is_leaf() {
        return children[0] == nullptr;
    }

    bool has_children() { 
        return children[0] != nullptr; 
    }
    std::unique_ptr<QuadTree> create_child( int local_i, int local_j );
};

}


#endif // RES_SRC_ENGINE_CLASSES_CHUNKTREE_HPP