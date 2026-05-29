#ifndef SRC_ENGINE_MATH_VERTEX_HPP
#define SRC_ENGINE_MATH_VERTEX_HPP

#include <vulkan/vulkan.hpp>
#include <array>

#include "../etc/header_libs.hpp"

namespace ec {

struct vertex {
    glm::vec3 pos;
    glm::vec2 tex_coord;

    static vk::VertexInputBindingDescription                    get_binding_description();
    static std::array<vk::VertexInputAttributeDescription, 2>   get_attribute_descriptions();
    
    bool    operator==  ( const vertex& other ) const;
};

} // namespace ec

namespace std {
    template<> struct hash<ec::vertex> {
        size_t operator()( ec::vertex const& vertex ) const {
            return  ( hash<glm::vec3>()( vertex.pos ) ) ^
                    ( hash<glm::vec2>()( vertex.tex_coord ) << 1 );
        }
    };
}

#endif // SRC_ENGINE_MATH_VERTEX_HPP