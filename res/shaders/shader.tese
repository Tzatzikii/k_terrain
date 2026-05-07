#version 450

layout(triangles, equal_spacing, cw) in;

layout(binding = 0) uniform MVP {
    mat4 model;
    mat4 view;
    mat4 proj;
} mvp;

layout(location = 1) in vec2 inTexCoord[];
layout(location = 2) in uint chunkIndex[];

layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out float height;
layout(binding = 1) uniform sampler2D noises[136];
layout(binding = 2) uniform sampler2D texSampler;


void main() {
    float u = gl_TessCoord.x;
    float v = gl_TessCoord.y;

     // barycentric interpolation
    vec3 bary = gl_TessCoord;

    vec4 p0 = gl_in[0].gl_Position;
    vec4 p1 = gl_in[1].gl_Position;
    vec4 p2 = gl_in[2].gl_Position;

    // retrieve control point texture coordinates
    vec2 t0 = inTexCoord[0];
    vec2 t1 = inTexCoord[1];
    vec2 t2 = inTexCoord[2];

    vec2 texCoord =
        bary.x * t0 +
        bary.y * t1 +
        bary.z * t2;

    vec4 p =
    bary.x * p0 +
    bary.y * p1 +
    bary.z * p2;

    fragTexCoord = texCoord;
    float dist = sqrt( p.x*p.x + p.y*p.y );
    uint index = chunkIndex[0];

    // lookup texel at patch coordinate for height and scale + shift as desired
    height = (1.0-(texture(noises[index], texCoord).y)) * 256.0;

    // compute patch surface normal
    vec4 uVec = p1 - p0;
    vec4 vVec = p2 - p0;
    vec4 normal = normalize( vec4(cross(vVec.xyz, uVec.xyz), 0) );
    
    // displace point along normal
    p += normal * height;

    // ----------------------------------------------------------------------
    // output patch point position in clip space
    gl_Position = mvp.proj * mvp.view * mvp.model * p;

}