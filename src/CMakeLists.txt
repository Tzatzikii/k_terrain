target_link_libraries(KTerrain
    PRIVATE
        FastNoise
        stb_image
        tiny_obj_loader
        glm::glm
        glfw
)

add_subdirectory(engine)
add_subdirectory(imgui)