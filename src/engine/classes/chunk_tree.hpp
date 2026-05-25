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

    struct Node {

        std::shared_ptr<QuadTree::Node> children[4] = { nullptr };

        int64_t level = 0;

        int64_t cx, cy;

        uint64_t size;

        uint32_t index;

        bool dirty;

        const int64_t   recursion_limit  = 20;
        const int64_t   min_size         = 16;
        const uint64_t  seed             = 727;

        Node( int64_t _cx, int64_t _cy, uint64_t _size = INT64_MAX*2, int64_t _level = 0, uint32_t _index = -1 )
            : cx(_cx), cy(_cy), size(_size), level(_level), index(_index){}

        void subdivide( glm::vec3 _eye_pos );

        void calculate_noise( u_char* _dest );

        float divide_threshold() {
            return static_cast<float>(this->size);
        }

        float unify_threshold() {
            return static_cast<float>(this->size)*1.5f;
        }
        
        bool is_divisible( float _dist_from_eye ) {
            
            return !( 
                ( this->level >= recursion_limit )   || 
                ( this->size <= min_size )           || 
                ( _dist_from_eye > divide_threshold() )

            ); 
        }
        
        void collapse( uint32_t _new_index );

        bool is_leaf() {
            return children[0] == nullptr;
        }

        

        //std::unique_ptr<QuadTree> create_child( int local_i, int local_j );

    };
    
    void subdivide_node( glm::vec3 _eye_pos, std::shared_ptr<QuadTree::Node> _node );

    std::shared_ptr<QuadTree::Node> top_node;
    std::vector<std::shared_ptr<QuadTree::Node>> leaves;


    uint64_t id_tracker;

    uint64_t leaf_count = 1;

    Texture noise_texture;
    
public:

    void generate();

    struct LeafInfo {

        uint32_t index;
        glm::vec2 pos;
        float size;
        uint32_t tess_edges[4]; // Booleans, but I want to avoid undefined behavior since glsl doesn't support booleans

        LeafInfo operator=( const LeafInfo& other ) {
            index = other.index;
            pos = other.pos;
            size = other.size;
            tess_edges[0] = other.tess_edges[0];
            tess_edges[1] = other.tess_edges[1];
            tess_edges[2] = other.tess_edges[2];
            tess_edges[3] = other.tess_edges[3];
        }

        static std::array<vk::VertexInputAttributeDescription, 4> get_attribute_descriptions() {
            std::array<vk::VertexInputAttributeDescription, 4> descs;
            descs[0].format = vk::Format::eR32Uint;
            descs[0].offset = offsetof( LeafInfo, index );
            descs[1].format = vk::Format::eR32G32Sfloat;
            descs[1].offset = offsetof( LeafInfo, pos );
            descs[2].format = vk::Format::eR32Sfloat;
            descs[2].offset = offsetof( LeafInfo, size );
            descs[3].format = vk::Format::eR32G32B32A32Uint;
            descs[3].offset = offsetof( LeafInfo, tess_edges );
            for( int i = 0; i < 4; i++ ) {
                descs[i].binding = 1;
                descs[i].location = i+ec::vertex::get_attribute_descriptions().size();
            }
            return descs;
        }

        static vk::VertexInputBindingDescription get_binding_description() {
            vk::VertexInputBindingDescription desc;
            desc.binding = 1;
            desc.stride = sizeof( LeafInfo );
            desc.inputRate = vk::VertexInputRate::eInstance;
            return desc;
        }
    };

    QuadTree( uint64_t _size = INT64_MAX*2 ){}
    
    void get_geometry( std::vector<vertex>& _vertices, std::vector<uint32_t>& _indices, uint32_t _n );

    std::vector<LeafInfo> get_leaf_infos();

    uint64_t get_leaf_count() { return leaf_count; }

    void get_noise_views( std::vector<vk::ImageView>& _views );
    void set_textures( Texture& texture );

    Texture generate_noise_texture(
        BaseApp* _current_app
    );
    void update( glm::vec3 _eye_pos );

    void calculate_noise( u_char* _dest );

    void delete_children();

    void update_tree_size();

    void update_children( glm::vec3 _eye_pos );

    void render( BaseApp& _current_app );

    void create_leaf();
};

}


#endif // RES_SRC_ENGINE_CLASSES_CHUNKTREE_HPP