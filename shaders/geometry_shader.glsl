#version 330 core
// int vec3 f_color;
layout(triangles) in;
// layout(points) in;
layout(triangle_strip, max_vertices = 3) out; // no wireframe
// layout(line_strip, max_vertices = 3) out;     // Use wireframe

in vec3 v_color[];
in vec3 v_normal_cameraSpace[];
in vec3 v_eyeDirection_cameraSpace[];
in vec3 v_lightDirection_cameraSpace[];
in vec3 v_world_position[];

// out int;
out vec3 f_color;
out vec3 f_normal_cameraSpace;
out vec3 f_eyeDirection_cameraSpace;
out vec3 f_lightDirection_cameraSpace;
out vec3 f_barycentric;
out vec3 f_triangle_normal;
out float f_elevation;
out vec3 f_world_coord;
// Uniforms
uniform mat4 ModelMatrix;
uniform mat4 ViewMatrix;
uniform mat4 MirrorMatrix;
uniform float minElevation;
uniform int meshID;

// layout(triangles) out;
// layout(triangles, max_vertices = 1) out;

void main() {
    // Generate triangle normal
    vec4 v0 = gl_in[0].gl_Position;
    vec4 v1 = gl_in[1].gl_Position;
    vec4 v2 = gl_in[2].gl_Position;
    vec4 e1 = v1 - v0;
    vec4 e2 = v2 - v0;
    vec3 triangle_normal = normalize(cross(e1.xyz, e2.xyz));

    f_triangle_normal =
        (ViewMatrix * ModelMatrix * vec4(triangle_normal, 0.0)).xyz;

    for (int i = 0; i < 3; i++) {
        gl_Position = gl_in[i].gl_Position;
        f_world_coord = (ViewMatrix * ModelMatrix * vec4(gl_Position)).xyz;
        f_color = v_color[i];
        f_normal_cameraSpace = v_normal_cameraSpace[i];
        f_eyeDirection_cameraSpace = v_eyeDirection_cameraSpace[i];
        if (meshID == 0) {
            f_normal_cameraSpace = f_triangle_normal;
        }
        f_lightDirection_cameraSpace = v_lightDirection_cameraSpace[i];
        if (i == 0) {
            f_barycentric = vec3(1, 0, 0);
        } else if (i == 1) {
            f_barycentric = vec3(0, 1, 0);
        } else if (i == 2) {
            f_barycentric = vec3(0, 0, 1);
        }

        // TODO: emit black writeframe vertices
        EmitVertex();
    }

    EndPrimitive();
}