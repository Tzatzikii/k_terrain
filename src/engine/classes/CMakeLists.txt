target_sources(KTerrain
    PRIVATE
        camera.cpp
        quadtree.cpp
        chunk.cpp
        object3d.cpp
        terrain.cpp
        vertex.cpp

    PUBLIC
        camera.hpp
        quadtree.hpp
        object3d.hpp
        terrain.hpp
        vertex.hpp
)