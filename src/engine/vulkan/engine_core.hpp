#ifndef SRC_ENGINE_VULKAN_ENGINE_CORE_HPP
#define SRC_ENGINE_VULKAN_ENGINE_CORE_HPP

#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1

#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>
#include <optional>
#include <iostream>
#include <chrono>
#include <set>
#include "../etc/helpers.hpp"
#include "../etc/constants.hpp"
#include "../math/vertex.hpp"

namespace ec {

std::vector<char> read_file( const std::string& filename );

class BaseApp {

public:

    void run();
    
    static VKAPI_ATTR vk::Bool32 VKAPI_CALL debug_callback( 
            vk::DebugUtilsMessageSeverityFlagBitsEXT message_severity,
            vk::DebugUtilsMessageTypeFlagsEXT message_type,
            const vk::DebugUtilsMessengerCallbackDataEXT* p_callback_data,
            void * p_user_data
    );

protected:

    std::string APP_NAME          = "";
    uint32_t    APP_MAJOR_VERSION = 0;
    uint32_t    APP_MINOR_VERSION = 1;
    uint32_t    APP_PATCH_VERSION = 0;
    std::vector<std::string> model_paths;

    virtual void record_command_buffer( vk::CommandBuffer command_buffer, uint32_t image_index ){}


//private: TEMPORARY!!!!!!

    GLFWwindow*                 window;
    vk::Instance                instance;
    vk::DebugUtilsMessengerEXT  debug_messenger;
    vk::PhysicalDevice          physical_device = VK_NULL_HANDLE;
    vk::Device                  device;
    vk::Queue                   graphics_queue;
    vk::SurfaceKHR              surface;
    vk::Queue                   present_queue;

    std::vector<const char*>    device_extensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
    
    vk::SwapchainKHR            swapchain;
    std::vector<vk::Image>      swapchain_images;
    vk::Format                  swapchain_image_format;
    vk::Extent2D                swapchain_extent;

    std::vector<vk::ImageView>  swapchain_image_views;

    vk::RenderPass              render_pass;
    vk::DescriptorSetLayout     descriptor_set_layout;
    vk::PipelineLayout          pipeline_layout;
    vk::Pipeline                graphics_pipeline;

    std::vector<vk::Framebuffer>    swapchain_framebuffers;

    vk::CommandPool             command_pool;
    std::vector<vk::CommandBuffer>  command_buffers;

    std::vector<vk::Semaphore>  image_available_semaphores;
    std::vector<vk::Semaphore>  render_finished_semaphores;
    std::vector<vk::Fence>      in_flight_fences;

    bool                        framebuffer_resized = false;

    vk::Buffer                  vertex_buffer;
    vk::DeviceMemory            vertex_buffer_memory;
    vk::Buffer                  index_buffer;
    vk::DeviceMemory            index_buffer_memory;

    std::vector<vk::Buffer>     uniform_buffers;
    std::vector<vk::DeviceMemory>   uniform_buffers_memory;
    std::vector<void*>          uniform_buffers_mapped;

    vk::DescriptorPool          descriptor_pool;
    std::vector<vk::DescriptorSet>  descriptor_sets;

    uint32_t                    mip_levels;
    vk::Image                   texture_image;
    vk::DeviceMemory            texture_image_memory;
    vk::ImageView               texture_image_view;
    vk::Sampler                 texture_sampler;

    vk::Image                   depth_image;
    vk::DeviceMemory            depth_image_memory;
    vk::ImageView               depth_image_view;

    vk::Image                   color_image;
    vk::DeviceMemory            color_image_memory;
    vk::ImageView               color_image_view;

    vk::SampleCountFlagBits     msaa_samples = vk::SampleCountFlagBits::e1;


    std::vector<ec::vertex>     vertices;
    std::vector<uint32_t>       indices;

    uint32_t                    current_frame = 0;

    void main_loop();
    void draw_frame();

    void init_window();
    void init_vulkan();

    void create_instance();
    void setup_debug_messenger();
    void create_surface();
    void pick_physical_device();
    void create_logical_device();
    void create_swapchain();
    void create_image_views();
    void create_render_pass();
    void create_descriptor_set_layout();
    void create_graphics_pipeline();
    void create_color_resources();
    void create_depth_resources();
    void create_framebuffers();
    void create_command_pool();
    void create_texture_image();
    void create_texture_image_view();
    void create_texture_sampler();
    void load_model( std::string path );
    void load_models();
    void create_vertex_buffer();
    void create_index_buffer();
    void create_uniform_buffers();
    void create_descriptor_pool();
    void create_descriptor_sets();
    void create_command_buffers();
    void create_sync_objects();

    void cleanup();

    void                        populate_debug_messenger_create_info( vk::DebugUtilsMessengerCreateInfoEXT& create_info );

    std::vector<const char*>    get_required_extensions();

    ec::QueueFamilyIndices      find_queue_families( vk::PhysicalDevice device );

    uint32_t find_memory_type( uint32_t type_filter, vk::MemoryPropertyFlags properties );

    bool                        check_device_extension_support( vk::PhysicalDevice device );
    bool                        is_device_suitable( vk::PhysicalDevice device );

    ec::SwapChainSupportDetails query_swapchain_support( vk::PhysicalDevice device );
    vk::SurfaceFormatKHR        choose_swap_surface_format( std::vector<vk::SurfaceFormatKHR> &available_formats );
    vk::PresentModeKHR          choose_swap_present_mode( std::vector<vk::PresentModeKHR> present_modes );
    vk::Extent2D                choose_swap_extent( const vk::SurfaceCapabilitiesKHR capabilities );

    void                        recreate_swapchain();
    void                        cleanup_swapchain();

    void                        transition_image_layout( vk::Image image, vk::Format format, vk::ImageLayout old_layout, vk::ImageLayout new_layout, uint32_t mip_levels );


    vk::ImageView               create_image_view( vk::Image image, vk::Format format, vk::ImageAspectFlags aspect_flags, uint32_t mip_level );
    void                        create_image( 
                                    uint32_t                width, 
                                    uint32_t                height, 
                                    uint32_t                mip_levels, 
                                    vk::SampleCountFlagBits   sample_count, 
                                    vk::Format                format, 
                                    vk::ImageTiling           tiling,
                                    vk::ImageUsageFlags       usage, 
                                    vk::MemoryPropertyFlags   properties, 
                                    vk::Image&                image, 
                                    vk::DeviceMemory&         image_memory 
                                );

    void                        create_buffer(
                                    vk::DeviceSize          size, 
                                    vk::BufferUsageFlags    usage, 
                                    vk::MemoryPropertyFlags properties,
                                    vk::Buffer&             buffer, 
                                    vk::DeviceMemory&       buffer_memory
                                );

    vk::Format                  find_supported_format( const std::vector<vk::Format> & candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features );
    vk::Format                  find_depth_format();

    vk::ShaderModule            create_shader_module( const std::vector<char>& code );

    vk::CommandBuffer begin_single_time_commands();
    void end_single_time_commands( vk::CommandBuffer command_buffer );
    void copy_buffer_to_image( vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height );
    void copyBuffer( vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size );
    void generate_mipmaps( vk::Image image, vk::Format image_format, int32_t texWidth, int32_t texHeight, uint32_t mipLevels );
    void update_uniform_buffer( uint32_t currentImage );

    static bool check_validation_layer_support();
    static void framebuffer_resize_callback( GLFWwindow * window, int width, int height ) {
        auto app = reinterpret_cast<BaseApp*>( glfwGetWindowUserPointer( window ));
        app->framebuffer_resized = true;
    }

};

}; // namespace ec


#endif // SRC_ENGINE_VULKAN_ENGINE_CORE_HPP