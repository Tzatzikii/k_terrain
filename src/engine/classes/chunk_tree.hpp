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

class ChunkTree {
private:
    std::unique_ptr<ChunkTree> subdivide[4] = {nullptr};
    int64_t max = INT64_MAX;
    int64_t min = INT64_MIN;
    static const int64_t recursion_limit = 20;
    static const int64_t min_size = 16;
    int64_t level = 0;
    int64_t cx, cy;
    uint64_t size;
    uint32_t id;
    static uint64_t id_tracker;
    uint64_t tree_size = 0;
    Texture noise_texture;
    static const uint64_t seed = 727;
    
public:
    ChunkTree( int64_t _cx, int64_t _cy, uint64_t _size = INT64_MAX*2, int64_t _level = 0, uint32_t _id = 0 );
    glm::vec2 get_center() { return glm::vec2( static_cast<float>(cx), static_cast<float>(cy) ); }
    void get_geometry( std::vector<vertex>& _vertices, std::vector<uint32_t>& _indices, uint32_t _n );
    uint64_t get_tree_size() { return tree_size; }
    void get_noise_views( std::vector<vk::ImageView>& _views );
    void set_textures( Texture& texture );
    void generate_noise_textures(
        BaseApp* _current_app
    );
    void clean();
};

}


#endif // RES_SRC_ENGINE_CLASSES_CHUNKTREE_HPP