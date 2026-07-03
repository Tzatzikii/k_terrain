target_sources(KTerrain
    PRIVATE
        camera.cpp
        chunk_tree.cpp
        chunk.cpp
        object3d.cpp
        terrain.cpp
        vertex.cpp

    PUBLIC
        camera.hpp
        chunk_tree.hpp
        object3d.hpp
        terrain.hpp
        vertex.hpp
)