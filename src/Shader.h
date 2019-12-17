#pragma once
#include <fstream>
#include <iostream>
#include <streambuf>
#include <string>

class Shader {
  private:
    std::string filename;
    std::string contents;

  public:
    Shader(){};
    // A helper method to load shaders from glsl files
    Shader(std::string _filename) {
        filename = _filename;
        // Load file
        std::ifstream fs(filename);
        std::string str = "";
        std::string line;
        std::cout << "Loading shader: " << filename << std::endl;
        if (fs.is_open()) {
            while (getline(fs, line)) {
                str = str + line + "\n";
            }
            fs.close();
        } else {
            std::cout << "Error reading file: " << filename << std::endl;
            // Terminate program when shaders cannot be read
            exit(1);
        }
        contents = str; // assign to variable
    }

    Shader(int shader_type) {
        // Default fragment shader
        if (shader_type == 1) {
            contents =
                "#version 150 core\n"
                "in vec3 f_color;\n"
                "in vec3 f_normal_cameraSpace;\n"
                "in vec3 f_lightDirection_cameraSpace;\n"
                "in vec3 vertexNormal;\n"
                "in vec3 f_barycentric;\n"
                "in vec3 f_triangle_normal;\n"
                "out vec4 outColor;\n"
                "uniform vec3 blinkingColor;\n"
                "uniform int isSelected;\n"
                "uniform int shadingMode;\n"
                "uniform vec3 backgroundColor;\n"
                "uniform float time;\n"
                "\n"
                "void main() {\n"
                "    // Render wireframe\n"
                "    vec3 baseColor = f_color;\n"
                "\n"
                "    float t = sin(time * 10.0) + 1.0;\n"
                "\n"
                "    // Creates blinking when selected\n"
                "    if (isSelected == 1) {\n"
                "        baseColor = f_color * t;\n"
                "    }\n"
                "\n"
                "    if (shadingMode == 0 || shadingMode == 1) { // Wireframe\n"
                "        if (any(lessThan(f_barycentric, vec3(0.05)))) {\n"
                "            if (isSelected == 1) {\n"
                "                outColor = vec4(t, t, t, 1.0);\n"
                "            } else {\n"
                "                outColor = vec4(0, 0, 0, 1.0);\n"
                "            }\n"
                "        } else {\n"
                "            if (shadingMode == 1) { // Flat shading\n"
                "                vec3 n = normalize(f_triangle_normal);\n"
                "                vec3 l = "
                "normalize(f_lightDirection_cameraSpace);\n"
                "                float cosTheta = clamp(dot(n, l), 0, 1);\n"
                "                // Shader code here\n"
                "                outColor = vec4(baseColor * cosTheta, 1.0);\n"
                "            } else { // No shading - wireframe\n"
                "                outColor = vec4(backgroundColor, 0.0f);\n"
                "            }\n"
                "        }\n"
                "    } else if (shadingMode == 2) { // Smooth shading\n"
                "        vec3 n = normalize(f_normal_cameraSpace);\n"
                "        vec3 l = normalize(f_lightDirection_cameraSpace);\n"
                "\n"
                "        float cosTheta = clamp(dot(n, l), 0, 1);\n"
                "        // Shader code here\n"
                "        outColor = vec4(baseColor * cosTheta, 1.0);\n"
                "    } else {\n"
                "        outColor = vec4(1.0, 0, 0, 1.0); // DEBUG - RED\n"
                "    }\n"
                "}";
        }

        // Default vertex shader
        if (shader_type == 2) {
            contents =
                "#version 150 core\n"
                "in vec3 mesh_0_position;\n"
                "in vec3 mesh_0_vertexNormal;\n"
                "in vec3 mesh_1_position;\n"
                "in vec3 mesh_1_vertexNormal;\n"
                "in vec3 mesh_2_position;\n"
                "in vec3 mesh_2_vertexNormal;\n"
                "\n"
                "// Values that stay constant for the whole mesh.\n"
                "uniform mat4 MVPMatrix;\n"
                "uniform mat4 ModelMatrix;\n"
                "uniform mat4 ViewMatrix;\n"
                "uniform vec3 lightPosition_world;\n"
                "uniform vec3 ModelColor;\n"
                "uniform int meshID;\n"
                "\n"
                "out vec3 v_color;\n"
                "out vec3 v_normal_cameraSpace;\n"
                "out vec3 v_lightDirection_cameraSpace;\n"
                "out int;\n"
                "\n"
                "void main() {\n"
                "    vec3 vertexNormal;\n"
                "    vec3 position;\n"
                "    // Position\n"
                "    switch (meshID) {\n"
                "    // Handle mesh ID 0\n"
                "    case 0:\n"
                "        position = mesh_0_position;\n"
                "        vertexNormal = mesh_0_vertexNormal;\n"
                "        break;\n"
                "    // Handle mesh ID 1\n"
                "    case 1:\n"
                "        position = mesh_1_position;\n"
                "        vertexNormal = mesh_1_vertexNormal;\n"
                "        break;\n"
                "    case 2:\n"
                "        position = mesh_2_position;\n"
                "        vertexNormal = mesh_2_vertexNormal;\n"
                "        break;\n"
                "    default:\n"
                "        // position = mesh_0_position;\n"
                "        // vertexNormal = mesh_0_vertexNormal;\n"
                "        // ERROR!\n"
                "        break;\n"
                "    }\n"
                "\n"
                "    gl_Position = MVPMatrix * vec4(position, 1.0);\n"
                "    v_color = ModelColor;\n"
                "\n"
                "    // Normals / lighting\n"
                "\n"
                "    // Position of the vertex, in worldspace : M * position\n"
                "    vec3 v_world_position = (ModelMatrix * vec4(position, "
                "1.0)).xyz;\n"
                "\n"
                "    // Vector that goes from the vertex to the camera, in "
                "camera space.\n"
                "    // In camera space, the camera is at the origin (0,0,0).\n"
                "    vec3 v_cameraSpace = (ViewMatrix * vec4(position, "
                "1.0)).xyz;\n"
                "    vec3 eyeDirection_cameraSpace = v_cameraSpace * -1;\n"
                "\n"
                "    // Vector that goes from the vertex to the light, in "
                "camera space. M is\n"
                "    // ommited because it's identity.\n"
                "    vec3 lightPosition_cameraSpace =\n"
                "        (ViewMatrix * vec4(lightPosition_world, 1)).xyz;\n"
                "    v_lightDirection_cameraSpace =\n"
                "        lightPosition_cameraSpace + "
                "eyeDirection_cameraSpace;\n"
                "\n"
                "    // // Vertex normal in camera space\n"
                "    v_normal_cameraSpace =\n"
                "        (ViewMatrix * ModelMatrix * vec4(vertexNormal, "
                "0.0)).xyz;\n"
                "}";
        }

        // Default geometry shader
        if (shader_type == 3) {
            contents =
                "#version 330 core\n"
                "// int vec3 f_color;\n"
                "layout(triangles) in;\n"
                "// layout(points) in;\n"
                "layout(triangle_strip, max_vertices = 3) out; // no "
                "wireframe\n"
                "// layout(line_strip, max_vertices = 3) out;     // Use "
                "wireframe\n"
                "\n"
                "in vec3 v_color[];\n"
                "in vec3 v_normal_cameraSpace[];\n"
                "in vec3 v_lightDirection_cameraSpace[];\n"
                "// out int;\n"
                "out vec3 f_color;\n"
                "out vec3 f_normal_cameraSpace;\n"
                "out vec3 f_lightDirection_cameraSpace;\n"
                "out vec3 f_barycentric;\n"
                "out vec3 f_triangle_normal;\n"
                "\n"
                "// Uniforms\n"
                "uniform mat4 ModelMatrix;\n"
                "uniform mat4 ViewMatrix;\n"
                "\n"
                "// layout(triangles) out;\n"
                "// layout(triangles, max_vertices = 1) out;\n"
                "\n"
                "void main() {\n"
                "    // Generate triangle normal\n"
                "    vec4 v0 = gl_in[0].gl_Position;\n"
                "    vec4 v1 = gl_in[1].gl_Position;\n"
                "    vec4 v2 = gl_in[2].gl_Position;\n"
                "    vec4 e1 = v1 - v0;\n"
                "    vec4 e2 = v2 - v0;\n"
                "    vec3 triangle_normal = normalize(cross(e1.xyz, e2.xyz));\n"
                "\n"
                "    f_triangle_normal =\n"
                "        (ViewMatrix * ModelMatrix * vec4(triangle_normal, "
                "0.0)).xyz;\n"
                "\n"
                "    for (int i = 0; i < 3; i++) {\n"
                "        gl_Position = gl_in[i].gl_Position;\n"
                "        f_color = v_color[i];\n"
                "        f_normal_cameraSpace = v_normal_cameraSpace[i];\n"
                "        f_lightDirection_cameraSpace = "
                "v_lightDirection_cameraSpace[i];\n"
                "        if (i == 0) {\n"
                "            f_barycentric = vec3(1, 0, 0);\n"
                "        } else if (i == 1) {\n"
                "            f_barycentric = vec3(0, 1, 0);\n"
                "        } else if (i == 2) {\n"
                "            f_barycentric = vec3(0, 0, 1);\n"
                "        }\n"
                "        // TODO: emit black writeframe vertices\n"
                "\n"
                "        EmitVertex();\n"
                "    }\n"
                "\n"
                "    EndPrimitive();\n"
                "}";
        }
    }

    std::string &read() { return contents; }
};