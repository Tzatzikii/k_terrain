#define STB_IMAGE_IMPLEMENTATION
#define TINYOBJLOADER_IMPLEMENTATION
#include "engine_core.hpp"

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
    create_descriptor_set_layout();
    create_graphics_pipeline();
    create_color_resources();
    create_depth_resources();
    create_framebuffers();
    create_command_pool();
    create_texture_image();
    create_texture_image_view();
    create_texture_sampler();
    load_model();
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

    vkResetCommandBuffer( command_buffers[current_frame], 0 );

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

    vk::Semaphore signal_semaphores[]    = { render_finished_semaphores[current_frame] };
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

    if( result == vk::Result::eErrorOutOfDateKHR || 
        result == vk::Result::eSuboptimalKHR || framebuffer_resized ) {
            framebuffer_resized = false;
            recreate_swapchain();
    }
    else if( result != vk::Result::eSuccess ) {
        throw std::runtime_error( "failed to present swap chain image!" );
    }

    current_frame = ( current_frame + 1 ) % MAX_FRAMES_IN_FLIGHT;
}

void BaseApp::main_loop() {
    while( !glfwWindowShouldClose(window)) {
        glfwPollEvents();
        draw_frame();
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

        vk::Buffer vertex_buffers[] = { vertex_buffer };
        vk::DeviceSize offsets[] = { 0 };
        command_buffer.bindVertexBuffers( 0, 1, vertex_buffers, offsets );

        vkCmdBindIndexBuffer( command_buffer, index_buffer, 0, VK_INDEX_TYPE_UINT32 );

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

        command_buffer.drawIndexed( static_cast<uint32_t>( indices.size() ), 1, 0, 0, 0 );
        
        command_buffer.endRenderPass();

        if( vkEndCommandBuffer( command_buffer ) != VK_SUCCESS ) {
            throw std::runtime_error( "failed to record command buffer!" );
        }
    }

};


} // namespace ec
int main() {
    
    ec::TestApp app = ec::TestApp();
    app.run();

    return 0;
}