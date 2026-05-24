#ifndef SRC_ENGINE_VULKAN_IMAGE_HPP
#define SRC_ENGINE_VULKAN_IMAGE_HPP

#include <vulkan/vulkan.hpp>

namespace ec {

extern uint32_t find_memory_type( vk::PhysicalDevice physical_device, uint32_t type_filter, vk::MemoryPropertyFlags properties );

struct ImageProperties {
    uint32_t                width;
    uint32_t                height;
    uint32_t                mip_levels;
    uint32_t                layer_count;
    vk::SampleCountFlagBits sample_count;
    vk::Format              format;
    vk::ImageTiling         tiling;
    vk::ImageUsageFlags     usage;
    vk::MemoryPropertyFlags memory_flags;
};


class Image2D {
private:
    ImageProperties properties; 
    vk::Image vk_image;
    vk::DeviceMemory vk_memory;  
    vk::ImageView view;
    vk::PhysicalDevice physical_device;
    vk::Device device;

public:
    void create( vk::PhysicalDevice& _phyisical_device, vk::Device& _device, ImageProperties& _properties );
    vk::ImageView& get_view() { return view; }
    void transition_layout( vk::CommandBuffer& _cmd_buffer, vk::ImageLayout _old, vk::ImageLayout _new );
    void create_view( vk::ImageAspectFlags _aspect_flags );
    void copy_from_buffer( vk::CommandBuffer& _cmd_buffer, vk::Buffer& _buffer );

    void clean();
};   

};

#endif