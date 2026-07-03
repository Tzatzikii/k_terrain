cmake_minimum_required(VERSION 3.23)

project(KTerrain)

add_executable(KTerrain)

add_subdirectory(libs)
add_subdirectory(src)

file(MAKE_DIRECTORY build/shaders)

add_custom_command(
    TARGET KTerrain
    POST_BUILD
    COMMENT "Compiling shaders..."
    COMMAND glslc ${CMAKE_SOURCE_DIR}/res/shaders/shader.frag -o ${CMAKE_BINARY_DIR}/shaders/shader.frag.spv
    COMMAND glslc ${CMAKE_SOURCE_DIR}/res/shaders/shader.vert -o ${CMAKE_BINARY_DIR}/shaders/shader.vert.spv
    COMMAND glslc ${CMAKE_SOURCE_DIR}/res/shaders/shader.tesc -o ${CMAKE_BINARY_DIR}/shaders/shader.tesc.spv
    COMMAND glslc ${CMAKE_SOURCE_DIR}/res/shaders/shader.tese -o ${CMAKE_BINARY_DIR}/shaders/shader.tese.spv
)