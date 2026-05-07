#include "vertex.hpp"


namespace ec {


vk::VertexInputBindingDescription vertex::get_binding_description() {

    VkVertexInputBindingDescription binding_description{};
    
    binding_description.binding = 0;
    binding_description.stride = sizeof( vertex );
    binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return binding_description;

}

std::array<vk::VertexInputAttributeDescription, 3> vertex::get_attribute_descriptions() {
    
    std::array<vk::VertexInputAttributeDescription, 3> attribute_descriptions{};

    attribute_descriptions[0].binding = 0;
        attribute_descriptions[0].location = 0;
        attribute_descriptions[0].format = vk::Format::eR32G32B32Sfloat; //VK_FORMAT_R32G32B32_SFLOAT;
        attribute_descriptions[0].offset = offsetof( vertex, pos );

        attribute_descriptions[1].binding = 0;
        attribute_descriptions[1].location = 1;
        attribute_descriptions[1].format = vk::Format::eR32G32B32Sfloat;
        attribute_descriptions[1].offset = offsetof( vertex, tex_coord );

        attribute_descriptions[2].binding = 0;
        attribute_descriptions[2].location = 2;
        attribute_descriptions[2].format = vk::Format::eR32Uint;
        attribute_descriptions[2].offset = offsetof( vertex, chunk_index );

        return attribute_descriptions;
}

bool vertex::operator==( const vertex& other ) const {
    return pos == other.pos && tex_coord == other.tex_coord;
}

} // namespace ec