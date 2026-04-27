#include "engine_core.hpp"

namespace ec {

void BaseApp::create_sync_objects() {

    image_available_semaphores.resize( MAX_FRAMES_IN_FLIGHT );
    render_finished_semaphores.resize( MAX_FRAMES_IN_FLIGHT );
    in_flight_fences.resize( MAX_FRAMES_IN_FLIGHT );

    vk::SemaphoreCreateInfo semaphore_info{};
    semaphore_info.sType = vk::StructureType::eSemaphoreCreateInfo; //VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    vk::FenceCreateInfo fence_info{};
    fence_info.sType = vk::StructureType        ::eFenceCreateInfo; //VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = vk::FenceCreateFlagBits  ::eSignaled; //VK_FENCE_CREATE_SIGNALED_BIT;

    for( size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++ ) {
        if( device.createSemaphore( &semaphore_info, nullptr, &image_available_semaphores[i] ) != vk::Result::eSuccess ||
            device.createSemaphore( &semaphore_info, nullptr, &render_finished_semaphores[i] ) != vk::Result::eSuccess || 
            device.createFence( &fence_info, nullptr, &in_flight_fences[i] ) != vk::Result::eSuccess ) {
                throw std::runtime_error( "failed to create semaphores!" );
        } 
    }

}
 

} // namespace ec