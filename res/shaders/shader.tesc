#version 450

layout(vertices = 4) out; // triangle patches

layout(location = 1) in vec2 inTexCoords[];
layout(location = 1) out vec2 outTexCoords[];
layout(location = 2) flat in uint inInstanceIndex[];
layout(location = 3) in vec2 inInstanceCenter[];
layout(location = 4) in float inInstanceSize[];
layout(location = 5) flat in uvec4 inTessEdges[];

layout(location = 2) patch out uint outInstanceIndex;
layout(location = 3) patch out vec2 outInstanceCenter;
layout(location = 4) patch out float outInstanceSize;

void main() {
    // Pass through positions
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;

    outTexCoords[gl_InvocationID] = inTexCoords[gl_InvocationID];
    float tessLevel = 64.0;

    // One invocation sets tessellation levels
    if (gl_InvocationID == 0) {
        outInstanceIndex    = inInstanceIndex[0];
        outInstanceCenter   = inInstanceCenter[0];
        outInstanceSize     = inInstanceSize[0];
        // Outer edges
        gl_TessLevelOuter[0] = tessLevel * inTessEdges[0].x;
        gl_TessLevelOuter[1] = tessLevel * inTessEdges[0].y;
        gl_TessLevelOuter[2] = tessLevel * inTessEdges[0].z;
        gl_TessLevelOuter[3] = tessLevel * inTessEdges[0].w;

        // Inner (for triangles, only [0] is used)
        gl_TessLevelInner[0] = tessLevel;
        gl_TessLevelInner[1] = tessLevel;
    }
}

