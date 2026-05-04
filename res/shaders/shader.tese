#version 450

layout(triangles, equal_spacing, cw) in;

layout(binding = 0) uniform MVP {
    mat4 model;
    mat4 view;
    mat4 proj;
} mvp;

layout(location = 1) in vec2 inTexCoord[];

layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out float height;
layout(binding = 1) uniform sampler2D noiseSampler;
layout(binding = 2) uniform sampler2D texSampler;


void main() {
    float u = gl_TessCoord.x;
    float v = gl_TessCoord.y;

     // Barycentric interpolation
    vec3 bary = gl_TessCoord;

    vec4 p0 = gl_in[0].gl_Position;
    vec4 p1 = gl_in[1].gl_Position;
    vec4 p2 = gl_in[2].gl_Position;

    
    // ----------------------------------------------------------------------
    // retrieve control point texture coordinates
    vec2 t0 = inTexCoord[0];
    vec2 t1 = inTexCoord[1];
    vec2 t2 = inTexCoord[2];

    // bilinearly interpolate texture coordinate across patch
    // vec2 t0 = (t1 - t0) * u + t00;
    // vec2 t1 = (t11 - t1) * u + t10;
    // vec2 texCoord = (t1 - t0) * v + t0;
    vec2 texCoord =
        bary.x * t0 +
        bary.y * t1 +
        bary.z * t2;

    // lookup texel at patch coordinate for height and scale + shift as desired
    height = texture(noiseSampler, texCoord).y * 16.0; //* 64.0 - 16.0;

    // ----------------------------------------------------------------------
    // retrieve control point position coordinates
    // vec4 p00 = gl_in[0].gl_Position;
    // vec4 p01 = gl_in[1].gl_Position;
    // vec4 p10 = gl_in[2].gl_Position;
    // vec4 p11 = gl_in[3].gl_Position;

    // compute patch surface normal
    vec4 uVec = p1 - p0;
    vec4 vVec = p2 - p0;
    vec4 normal = normalize( vec4(cross(vVec.xyz, uVec.xyz), 0) );

    // bilinearly interpolate position coordinate across patch
    // vec4 p0 = (p01 - p00) * u + p00;
    // vec4 p1 = (p11 - p10) * u + p10;
    // vec4 p = (p1 - p0) * v + p0;

    vec4 p =
        bary.x * p0 +
        bary.y * p1 +
        bary.z * p2;


    // displace point along normal
    p += normal * height;

    // ----------------------------------------------------------------------
    // output patch point position in clip space
    gl_Position = mvp.proj * mvp.view * mvp.model * p;

}