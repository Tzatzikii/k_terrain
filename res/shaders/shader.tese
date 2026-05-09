#version 450
#extension GL_EXT_nonuniform_qualifier : enable
precision highp float;
precision highp sampler2D;

layout(quads, fractional_odd_spacing, ccw) in;

layout(binding = 0) uniform MVP {
    mat4 model;
    mat4 view;
    mat4 proj;
} mvp;

layout(location = 1) in vec2 inTexCoord[];
layout(location = 2) patch in uint chunkIndex;

layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out float height;
layout(binding = 1) uniform sampler2D noises[136];
layout(binding = 2) uniform sampler2D texSampler;


void main() {
    float u = clamp(gl_TessCoord.x, 0.0, 1.0);
    float v = clamp(gl_TessCoord.y, 0.0, 1.0);

    fragTexCoord = vec2(u, v);

    vec4 p00 = gl_in[0].gl_Position;
    vec4 p01 = gl_in[1].gl_Position;
    vec4 p10 = gl_in[2].gl_Position;
    vec4 p11 = gl_in[3].gl_Position;

    vec4 p0 = (p01 - p00) * u + p00;
    vec4 p1 = (p11 - p10) * u + p10;
    vec4 p = (p1 - p0) * v + p0;

    // retrieve control point texture coordinates
    vec2 t00 = inTexCoord[0];
    vec2 t01 = inTexCoord[1];
    vec2 t10 = inTexCoord[2];
    vec2 t11 = inTexCoord[3];

    vec2 t0 = (t01 - t00) * u + t00;
    vec2 t1 = (t11 - t10) * u + t10;
    vec2 texCoord = (t1 - t0) * v + t0;

    fragTexCoord = texCoord;
    float dist = sqrt( p.x*p.x + p.y*p.y );

    // lookup texel at patch coordinate for height and scale + shift as desired
    height = textureLod(noises[nonuniformEXT(chunkIndex)], texCoord, 0.0).r * 256.0;
    //height = dist/2.0;

    // compute patch surface normal
    vec4 uVec = p01 - p00;
    vec4 vVec = p10 - p00;
    vec4 normal = normalize( vec4(cross(uVec.xyz, vVec.xyz), 0) );

    p.z += height;
    
    gl_Position = mvp.proj * mvp.view * mvp.model * p;


}