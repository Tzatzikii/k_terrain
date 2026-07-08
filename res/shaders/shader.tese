#version 450
#extension GL_EXT_nonuniform_qualifier : enable
precision highp float;
precision highp sampler2D;

layout(quads, fractional_even_spacing, cw) in;

layout(binding = 0) uniform MVP {
    mat4 model;
    mat4 view;
    mat4 proj;
} mvp;

layout(location = 1) in vec2 inTexCoord[];
layout(location = 2) patch in uint inInstanceIndex;
layout(location = 3) patch in vec2 inInstanceCenter;
layout(location = 4) patch in float inInstanceSize;

layout(location = 1) out vec2 fragTexCoord;
//layout(location = 2) out float height;
layout(location = 2) flat out uint outInstanceIndex;
layout(location = 4) out float outInstanceSize;
layout(binding = 1) uniform sampler2D texSampler;
layout(binding = 2) uniform sampler2DArray noises;


void main() {
    outInstanceIndex = inInstanceIndex;
    outInstanceSize = inInstanceSize;
    fragTexCoord = gl_TessCoord.xy;
    float u = clamp(gl_TessCoord.x, 0.0, 1.0);
    float v = clamp(gl_TessCoord.y, 0.0, 1.0);

    fragTexCoord = vec2(u, v);

    vec4 p00 = gl_in[0].gl_Position;
    vec4 p10 = gl_in[1].gl_Position;
    vec4 p01 = gl_in[2].gl_Position;
    vec4 p11 = gl_in[3].gl_Position;

    vec4 bottom = mix(p00, p10, u);
    vec4 top    = mix(p01, p11, u);

    vec4 p = mix(bottom, top, v);
    p.xy += inInstanceCenter;

    float dist = sqrt( p.x*p.x + p.y*p.y );

    float height = texture(noises, vec3(fragTexCoord, float(inInstanceIndex))).r * 256 - 64;

    p.z += height; //+ log(float(inInstanceIndex));
    
    gl_Position = mvp.proj * mvp.view * p;


}