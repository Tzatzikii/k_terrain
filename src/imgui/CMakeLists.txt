#add_subdirectory(backends)

target_include_directories(KTerrain
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_sources(KTerrain
    PRIVATE
        imgui_demo.cpp
        imgui_draw.cpp
        imgui_tables.cpp
        imgui_widgets.cpp
        imgui.cpp
        imgui_internal.h
        imconfig.h
        imstb_rectpack.h
        imstb_textedit.h
        imstb_truetype.h
        imgui_impl_glfw.cpp
        imgui_impl_vulkan.cpp

    PUBLIC
        imgui.h
        imgui_impl_glfw.h
        imgui_impl_vulkan.h
)