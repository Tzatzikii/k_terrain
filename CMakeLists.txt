cmake_minimum_required(VERSION 3.23)

project(KTerrain)

add_executable(KTerrain)

add_subdirectory(libs)
add_subdirectory(src)

find_program(GLSLC glslc REQUIRED)

file(GLOB SHADERS
    "${CMAKE_SOURCE_DIR}/res/shaders/*.vert"
    "${CMAKE_SOURCE_DIR}/res/shaders/*.frag"
    "${CMAKE_SOURCE_DIR}/res/shaders/*.tesc"
    "${CMAKE_SOURCE_DIR}/res/shaders/*.tese"
)

set(SPIRV_FILES)

foreach(SHADER ${SHADERS})
    get_filename_component(NAME ${SHADER} NAME)
    set(SPIRV "${CMAKE_BINARY_DIR}/shaders/${NAME}.spv")

    add_custom_command(
        OUTPUT ${SPIRV}

        COMMAND ${CMAKE_COMMAND} -E make_directory
                ${CMAKE_BINARY_DIR}/shaders

        COMMAND ${GLSLC}
                ${SHADER}
                -o
                ${SPIRV}

        DEPENDS ${SHADER}

        COMMENT "Compiling ${NAME}"
        VERBATIM
    )

    list(APPEND SPIRV_FILES ${SPIRV})
endforeach()

add_custom_target(Shaders ALL
    DEPENDS ${SPIRV_FILES}
)

add_dependencies(KTerrain Shaders)