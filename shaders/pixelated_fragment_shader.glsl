#version 330 core

in vec2 UV;
in vec2 res;

layout(location = 0) out vec3 color;

uniform sampler2D renderedTexture;
uniform float time;
uniform float pixelWidth;

void main() {
    // Pixelated shader
    float Pixels = pixelWidth;
    float Intensity = 5.0;
    float dx = Intensity * (1.0 / Pixels);
    float dy = Intensity * (1.0 / Pixels);
    vec2 uv = vec2(dx * floor(UV.x / dx),  // derive x
                   dy * floor(UV.y / dy)); // derive y
    vec4 f_color = texture(renderedTexture, uv);

    // Passthrough
    // vec4 f_color = texture(renderedTexture, UV);
    color = f_color.xyz;
}