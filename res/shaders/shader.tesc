#version 450

layout(vertices = 3) out; // triangle patches

layout(location = 1) in vec2 in_tex_coords[];
layout(location = 1) out vec2 out_tex_coords[];

void main() {
    // Pass through positions
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;

    out_tex_coords[gl_InvocationID] = in_tex_coords[gl_InvocationID];

    // One invocation sets tessellation levels
    if (gl_InvocationID == 0) {
        // Outer edges
        gl_TessLevelOuter[0] = 4.0;
        gl_TessLevelOuter[1] = 4.0;
        gl_TessLevelOuter[2] = 4.0;

        // Inner (for triangles, only [0] is used)
        gl_TessLevelInner[0] = 4.0;
    }
}