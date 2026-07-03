find_package(Vulkan)

target_link_libraries(KTerrain
    PRIVATE
        Vulkan::Vulkan
)

target_sources(KTerrain
    PRIVATE
        buffers.cpp
        cleanup.cpp
        color_resources.cpp
        command_pool.cpp
        common.cpp
        debug_utils.cpp
        depth_resources.cpp
        descriptor_set_layout.cpp
        descriptors.cpp
        graphics_pipeline.cpp
        image.cpp
        images.cpp
        instance.cpp
        logical_device.cpp
        models.cpp
        physical_device.cpp
        render_pass.cpp
        run.cpp
        surface.cpp
        swapchain.cpp
        sync_objects.cpp
        texture.cpp
        texture.cpp
        textures.cpp
        window.cpp

    PUBLIC
        engine_core.hpp
        image.hpp
        texture.hpp

    )