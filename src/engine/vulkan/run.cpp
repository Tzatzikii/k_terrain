#define STB_IMAGE_IMPLEMENTATION
#define TINYOBJLOADER_IMPLEMENTATION
#include "engine_core.hpp"
#include <chrono>

namespace ec {

void BaseApp::init_vulkan() {
    VULKAN_HPP_DEFAULT_DISPATCHER.init();
    create_instance();
    setup_debug_messenger();
    create_surface();
    pick_physical_device();
    create_logical_device();
    create_swapchain();
    create_image_views();
    create_render_pass();
    load_models();
    create_descriptor_set_layout();
    create_graphics_pipeline();
    create_color_resources();
    create_depth_resources();
    create_framebuffers();
    create_command_pool();
    create_texture_image();
    create_texture_image_view();
    create_texture_sampler();
    create_vertex_buffer();
    create_index_buffer();
    create_uniform_buffers();
    create_descriptor_pool();
    create_descriptor_sets();
    create_command_buffers();
    create_sync_objects();
}

void BaseApp::draw_frame() {
    device.waitForFences( 1, &in_flight_fences[current_frame], VK_TRUE, UINT64_MAX );
    
    uint32_t image_index;
    vk::Result result = device.acquireNextImageKHR( swapchain, UINT64_MAX, image_available_semaphores[current_frame], VK_NULL_HANDLE, &image_index );
    if( result == vk::Result::eErrorOutOfDateKHR ) {
        recreate_swapchain();
        return;
    }
    else if( result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR ) {
        throw std::runtime_error( "failed to acquire swap chain image!" );
    }
    
    device.resetFences( 1, &in_flight_fences[current_frame] );

    command_buffers[current_frame].reset({ });

    record_command_buffer( command_buffers[current_frame], image_index );

    update_uniform_buffer( current_frame );

    vk::SubmitInfo submit_info{};
    submit_info.sType = vk::StructureType::eSubmitInfo; //VK_STRUCTURE_TYPE_SUBMIT_INFO;
    
    vk::Semaphore wait_semaphores[] = { image_available_semaphores[current_frame] };
    vk::PipelineStageFlags wait_stages[] = { 
        vk::PipelineStageFlagBits::eColorAttachmentOutput //VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT 
    };
    submit_info.waitSemaphoreCount   = 1;
    submit_info.pWaitSemaphores      = wait_semaphores;
    submit_info.pWaitDstStageMask    = wait_stages;

    submit_info.commandBufferCount   = 1;
    submit_info.pCommandBuffers      = &command_buffers[current_frame];

    vk::Semaphore signal_semaphores[]    = { render_finished_semaphores[image_index] };
    submit_info.signalSemaphoreCount     = 1;
    submit_info.pSignalSemaphores        = signal_semaphores;

    if( graphics_queue.submit( 1, &submit_info, in_flight_fences[current_frame]) != vk::Result::eSuccess ) {
        throw std::runtime_error( "failed to subit draw command buffer!" );
    }

    vk::PresentInfoKHR present_info{};
    present_info.sType = vk::StructureType::ePresentInfoKHR; //VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores    = signal_semaphores;

    vk::SwapchainKHR swapchains[]   = { swapchain };
    present_info.swapchainCount     = 1;
    present_info.pSwapchains        = swapchains;
    present_info.pImageIndices      = &image_index;
    present_info.pResults           = nullptr;

    result = present_queue.presentKHR( &present_info );

    if( 
        result == vk::Result::eErrorOutOfDateKHR    || 
        result == vk::Result::eSuboptimalKHR        || 
        framebuffer_resized 
    ) {
            framebuffer_resized = false;
            recreate_swapchain();
    }
    else if( result != vk::Result::eSuccess ) {
        throw std::runtime_error( "failed to present swap chain image!" );
    }

    current_frame = ( current_frame + 1 ) % MAX_FRAMES_IN_FLIGHT;
    
}

void BaseApp::main_loop() {

    static auto prev = std::chrono::high_resolution_clock::now();
    static uint64_t frame = 0;
    
    
    while( !glfwWindowShouldClose(window) ) {
        auto now = std::chrono::high_resolution_clock::now();
        auto delta = now-prev;
        float dt = delta.count();

        glfwPollEvents();
        key_events( dt/1000.0f );
        draw_frame();

        prev = now;
        frame++;
    }

    device.waitIdle();
}

void BaseApp::run() {

    init_window();
    init_vulkan();
    main_loop();
    cleanup();

}

class TestApp : public BaseApp {
public:

    TestApp() {
        model_paths.push_back( MODEL_PATH );
        instances.push_back(
            {
                0,
                glm::vec2(0, 0),
                16.0f,
                {1, 1, 1, 0}
            }
        );
        instances.push_back(
            {
                1,
                glm::vec2(0, 1),
                8.0f,
                {1, 0, 1, 1}
            }
        );
    }

private:

    void key_events( float dt ) override {
        dt /= 1000.0f;

        float speed = 5.0f*dt;
        if( keys_pressed[ GLFW_KEY_W ] ) {
            camera.forwards(speed);
        }
        if( keys_pressed[ GLFW_KEY_S ] ) {
            camera.forwards(-speed);
        }
        if( keys_pressed[ GLFW_KEY_A ] ) {
            camera.sideways(-speed);
        }
        if( keys_pressed[ GLFW_KEY_D ] ) {
            camera.sideways(speed);
        }
        if( keys_pressed[ GLFW_KEY_U ] ) {
            quad_tree.update(get_camera_pos());
            instances = quad_tree.get_leaf_infos();
            quad_tree.update_noise_texture(this, noise_texture);
        }
    }

    void cursor_events( double xpos, double ypos ) override {
        static double px = 0;
        static double py = 0;
        double dx = px-xpos;
        double dy = py-ypos;
        camera.rotate( static_cast<float>( dy/5.0 ), static_cast<float>( dx/5.0 ) );
        px = xpos;
        py = ypos;
    }


    void record_command_buffer( vk::CommandBuffer command_buffer, uint32_t image_index ) override {
        vk::CommandBufferBeginInfo begin_info{};
        begin_info.sType = vk::StructureType::eCommandBufferBeginInfo, //VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.flags = vk::CommandBufferUsageFlagBits{0};
        begin_info.pInheritanceInfo = nullptr;

        if( command_buffer.begin( &begin_info) != vk::Result::eSuccess) {
            throw std::runtime_error( "failed to begin recording command buffer!" );
        }
        
        vk::RenderPassBeginInfo render_pass_info{};
        render_pass_info.sType              = vk::StructureType::eRenderPassBeginInfo; //VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        render_pass_info.renderPass         = render_pass;
        render_pass_info.framebuffer        = swapchain_framebuffers[image_index];
        render_pass_info.renderArea.offset  = vk::Offset2D{0, 0};
        render_pass_info.renderArea.extent  = swapchain_extent;
        
        std::array<vk::ClearValue, 2> clear_values{};
        clear_values[0].color           = vk::ClearColorValue{0.0f, 0.0f, 0.0f, 1.0f};
        clear_values[1].depthStencil    = vk::ClearDepthStencilValue{1.0f, 0};

        render_pass_info.clearValueCount = static_cast<uint32_t>( clear_values.size() );
        render_pass_info.pClearValues = clear_values.data();
        
        command_buffer.beginRenderPass( &render_pass_info, vk::SubpassContents::eInline);

        command_buffer.bindPipeline( vk::PipelineBindPoint::eGraphics, graphics_pipeline );

        if(current_frame % 4 == 0) {
          //  quad_tree.update(get_camera_pos());
        }
        //instances = quad_tree.get_leaf_infos();
        create_instance_buffer();

        vk::Buffer vertex_buffers[] = { vertex_buffer, instance_buffer };
        vk::DeviceSize offsets[] = { 0, 0 };
        command_buffer.bindVertexBuffers( 0, 2, vertex_buffers, offsets );

        command_buffer.bindIndexBuffer( index_buffer, 0, vk::IndexType::eUint32 );

        vk::Viewport viewport{};
        viewport.x          = 0.0f;
        viewport.y          = 0.0f;
        viewport.width      = static_cast<float>( swapchain_extent.width );
        viewport.height     = static_cast<float>( swapchain_extent.height );
        viewport.minDepth   = 0.0f;
        viewport.maxDepth   = 1.0f;
        command_buffer.setViewport( 0, 1, &viewport );
        
        vk::Rect2D scissor{};
        scissor.offset = vk::Offset2D{0, 0};
        scissor.extent = swapchain_extent;
        command_buffer.setScissor( 0, 1, &scissor );


        command_buffer.bindDescriptorSets( vk::PipelineBindPoint::eGraphics, pipeline_layout, 0, 1, &descriptor_sets[current_frame], 0, nullptr );

        command_buffer.drawIndexed( static_cast<uint32_t>( indices.size() ), instances.size(), 0, 0, 0 );
        
        command_buffer.endRenderPass();

        command_buffer.end();
    }

};


} // namespace ec
int32_t main() {
    
    ec::TestApp app = ec::TestApp();
    app.run();

    return 0;
}