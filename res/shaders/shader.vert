#version 450

layout(binding = 0) uniform MVP {
    mat4 model;
    mat4 view;
    mat4 proj;
} mvp;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in uint inInstanceIndex;
layout(location = 3) in vec2 inInstanceCenter;
layout(location = 4) in float inInstanceSize;
layout(location = 5) in uvec4 inInstanceTessEdges;

layout(location = 1) out vec2 outTexCoord;
layout(location = 2) out uint outInstanceIndex;
layout(location = 3) out vec2 outInstanceCenter;
layout(location = 4) out float outInstanceSize;
layout(location = 5) out uvec4 outInstanceTessEdges;

void main() {
    gl_Position = vec4(inPosition*inInstanceSize, 1.0);
    outTexCoord = inTexCoord;
    outInstanceIndex = inInstanceIndex;
    outInstanceCenter = inInstanceCenter;
    outInstanceSize = inInstanceSize;
    outInstanceTessEdges = inInstanceTessEdges;
}