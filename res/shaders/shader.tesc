#version 450

layout(vertices = 4) out; // triangle patches

layout(location = 1) in vec2 inTexCoords[];
layout(location = 1) out vec2 outTexCoords[];
layout(location = 2) flat in uint instanceIndex[];
layout(location = 3) in vec2 inInstanceCenter[];
layout(location = 4) in float inInstanceSize[];
layout(location = 5) flat in uvec4 inTessEdges[];


void main() {
    // Pass through positions
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;

    outTexCoords[gl_InvocationID] = inTexCoords[gl_InvocationID];
    float tessLevel = 16.0;

    // One invocation sets tessellation levels
    if (gl_InvocationID == 0) {
        // Outer edges
        gl_TessLevelOuter[0] = tessLevel / ( inTessEdges[0].x > 0 ? 2.0 : 1.0 );
        gl_TessLevelOuter[1] = tessLevel / ( inTessEdges[0].y > 0 ? 2.0 : 1.0 );
        gl_TessLevelOuter[2] = tessLevel / ( inTessEdges[0].z > 0 ? 2.0 : 1.0 );
        gl_TessLevelOuter[3] = tessLevel / ( inTessEdges[0].w > 0 ? 2.0 : 1.0 );

        // Inner (for triangles, only [0] is used)
        gl_TessLevelInner[0] = tessLevel;
        gl_TessLevelInner[1] = tessLevel;
    }
}

