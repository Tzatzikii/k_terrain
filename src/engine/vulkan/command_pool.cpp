#include "engine_core.hpp"

namespace ec {

void BaseApp::create_command_pool() {
    //ec::QueueFamilyIndices q_family_indices = find_queue_families( physical_device );

    vk::CommandPoolCreateInfo pool_info{};
    pool_info.sType = vk::StructureType             ::eCommandPoolCreateInfo; //VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_info.flags = vk::CommandPoolCreateFlagBits ::eResetCommandBuffer; //VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    if( device.createCommandPool( &pool_info, nullptr, &command_pool) != vk::Result::eSuccess) {
        throw std::runtime_error( "failed to create command pool!" );
    }
}

} // namespace ec