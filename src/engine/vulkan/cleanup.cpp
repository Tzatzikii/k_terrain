#include "engine_core.hpp"

namespace ec {

void BaseApp::cleanup_swapchain() {
    
    device.destroyImageView( color_image_view, nullptr );
    device.destroyImage( color_image, nullptr );
    device.freeMemory( color_image_memory, nullptr );
    device.destroyImageView( depth_image_view, nullptr );
    device.destroyImage( depth_image, nullptr );
    device.freeMemory( depth_image_memory, nullptr );
    device.destroyImage( shadow_depth_image, nullptr );
    device.freeMemory( shadow_depth_image_memory, nullptr );
    


    for( auto framebuffer : swapchain_framebuffers ) {
        device.destroyFramebuffer( framebuffer, nullptr ); 
    }

    for( auto image_view : swapchain_image_views ) {
        device.destroyImageView( image_view, nullptr );
    }

    device.destroySwapchainKHR( swapchain, nullptr );
}

void BaseApp::cleanup() {
    cleanup_swapchain();

    texture.clean();
    chunk_tree->clean();
    //noise_texture.clean();

    device.destroySampler( texture_sampler, nullptr );
    device.destroySampler( noise_sampler, nullptr );
    //device.destroyImageView( texture_image_view, nullptr );

   // device.destroyImage( texture_image, nullptr );
   // device.freeMemory( texture_image_memory, nullptr );


    for( size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++ ) {
        device.destroyBuffer( uniform_buffers[i], nullptr );
        device.freeMemory( uniform_buffers_memory[i], nullptr );
    }

    device.destroyDescriptorPool( descriptor_pool, nullptr );

    device.destroyDescriptorSetLayout( descriptor_set_layout, nullptr );

    device.destroyBuffer( vertex_buffer, nullptr );
    device.freeMemory( vertex_buffer_memory, nullptr );
    device.destroyBuffer( index_buffer, nullptr );
    device.freeMemory( index_buffer_memory, nullptr );

    for( size_t i = 0; i < swapchain_images.size(); i++ ) {
        device.destroySemaphore( render_finished_semaphores[i], nullptr );
    }

    for( size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++ ) {
        device.destroySemaphore( image_available_semaphores[i], nullptr );
        device.destroyFence( in_flight_fences[i], nullptr );
    }
    device.destroyCommandPool( command_pool, nullptr );
    
    device.destroyPipeline( graphics_pipeline, nullptr );
    device.destroyPipelineLayout( pipeline_layout, nullptr );
    device.destroyRenderPass( render_pass, nullptr );
    
    if( enable_validation_layers ) {
        instance.destroyDebugUtilsMessengerEXT( debug_messenger ); 
    }
    
    device.destroy( nullptr );
    instance.destroySurfaceKHR( surface, nullptr );
    instance.destroy( nullptr) ;
    

    glfwDestroyWindow( window );

    glfwTerminate();
}


} // namespace ec