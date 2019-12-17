#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vertexPosition_modelspace;
// in vec3 vertexPosition_modelspace;

uniform vec2 iResolution;

// Output data ; will be interpolated for each fragment.
out vec2 UV;
out vec2 res;

void main() {
    vec3 vIN = vertexPosition_modelspace.xyz;
    gl_Position = vec4(vIN, 1);

    UV = (vertexPosition_modelspace.xy + vec2(1, 1)) / 2.0;

    res = iResolution.xy;

    // // In GPU vertex calculation. -- NEEDS REPAIR
    // float x = float(((uint(gl_VertexID) + 2u) / 3u) % 2u);
    // float y = float(((uint(gl_VertexID) + 1u) / 3u) % 2u);

    // gl_Position = vec4(-1.0f + x * 2.0f, -1.0f + y * 2.0f, 0.0f, 1.0f);
    // UV = vec2(x, y);
}