add_library(FastNoise)
add_library(stb_image INTERFACE)
add_library(tiny_obj_loader INTERFACE)

include(FetchContent)

target_sources(FastNoise
    PRIVATE
        FastNoise.cpp
    
    PUBLIC
        FastNoise.h
)

target_include_directories(stb_image
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_include_directories(FastNoise
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_include_directories(tiny_obj_loader
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

FetchContent_Declare(
	glm
	GIT_REPOSITORY	https://github.com/g-truc/glm.git
	GIT_TAG 	0af55ccecd98d4e5a8d1fad7de25ba429d60e863 #refs/tags/1.0.1
)

FetchContent_MakeAvailable(glm)

include(FetchContent)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
)

FetchContent_MakeAvailable(glfw)
