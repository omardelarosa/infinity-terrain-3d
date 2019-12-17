#version 330 core

in vec2 UV;
in vec2 res;

layout(location = 0) out vec3 color;

uniform sampler2D renderedTexture;
uniform float time;

void main() {
    float w_factor = res.x;
    float h_factor = res.y;
    // Wobbly shader
    color = texture(renderedTexture,   // rendered texture
                    UV + 0.005 * vec2( //
                                     sin(time + w_factor * UV.x), // x
                                     cos(time + h_factor * UV.y)) // y
                    )                                             //
                .xyz;
}