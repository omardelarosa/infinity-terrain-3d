#version 150 core
in vec3 f_color;
in vec3 f_normal_cameraSpace;
in vec3 f_eyeDirection_cameraSpace;
in vec3 f_lightDirection_cameraSpace;
in vec3 vertexNormal;
in vec3 f_barycentric;
in vec3 f_triangle_normal;
in float f_elevation;
out vec4 outColor;
uniform vec3 blinkingColor;
uniform int isSelected;
uniform int shadingMode;
uniform vec3 backgroundColor;
uniform float time;
uniform int meshID;
uniform float minElevation;
uniform float maxElevation;
uniform float vertexColorBlendAmount;
uniform int DEBUG_VISUALS;

void main() {
    // Render wireframe
    vec3 baseColor = f_color;

    float t = sin(time * 10.0) + 1.0;

    if (shadingMode == 0 || shadingMode == 1) { // Wireframe
        if (any(lessThan(f_barycentric, vec3(0.05)))) {
            if (isSelected == 1) {
                outColor = vec4(t, t, t, 1.0);
            } else {
                outColor = vec4(0, 0, 0, 1.0);
            }
        } else {
            if (shadingMode == 1) { // Flat shading
                vec3 n = normalize(f_triangle_normal);
                vec3 l = normalize(f_lightDirection_cameraSpace);
                float cosTheta = clamp(dot(n, l), 0, 1);
                // Shader code here
                outColor = vec4(baseColor * cosTheta, 1.0);
            } else { // No shading - wireframe
                outColor = vec4(backgroundColor, 0.0f);
            }
        }
    } else if (shadingMode == 2) { // Smooth shading
        vec3 n = normalize(f_normal_cameraSpace);
        vec3 l = normalize(f_lightDirection_cameraSpace);
        float ambient_light = 0.01;
        float cosTheta = clamp(dot(n, l) + ambient_light, 0, 1);
        // Shader code here
        vec3 c = (baseColor * cosTheta);
        outColor = vec4(c, 1.0);
    } else {
        outColor = vec4(1.0, 0, 0, 1.0); // DEBUG - RED
    }
}