#version 150 core
in vec3 mesh_0_position;
in vec3 mesh_0_vertexNormal;
in vec3 mesh_0_vertexColor;
in vec3 mesh_1_position;
in vec3 mesh_1_vertexNormal;
in vec3 mesh_1_vertexColor;
in vec3 mesh_2_position;
in vec3 mesh_2_vertexNormal;
in vec3 mesh_2_vertexColor;

// Values that stay constant for the whole mesh.
uniform mat4 MVPMatrix;
uniform mat4 ModelMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;
uniform mat4 MirrorMatrix;
uniform vec3 lightPosition_world;
uniform vec3 ModelColor;
uniform int meshID;
uniform float minElevation;
uniform float maxElevation;
uniform int DEBUG_VISUALS;
uniform float vertexColorBlendAmount;

out vec3 v_color;
out vec3 v_eyeDirection_cameraSpace;
out vec3 v_normal_cameraSpace;
out vec3 v_lightDirection_cameraSpace;
out int;

void main() {
    vec3 vertexNormal;
    vec3 position;
    vec3 vertexColor;
    // Position
    switch (meshID) {
    // Handle mesh ID 0
    case 0:
        position = mesh_0_position;
        vertexNormal = mesh_0_vertexNormal;
        vertexColor = mesh_0_vertexColor;
        break;
    // Handle mesh ID 1
    case 1:
        position = mesh_1_position;
        vertexNormal = mesh_1_vertexNormal;
        vertexColor = mesh_1_vertexColor;
        break;
    case 2:
        position = mesh_2_position;
        vertexNormal = mesh_2_vertexNormal;
        vertexColor = mesh_2_vertexColor;
        break;
    default:
        // position = mesh_0_position;
        // vertexNormal = mesh_0_vertexNormal;
        // ERROR!
        break;
    }

    gl_Position = MVPMatrix * vec4(position, 1.0);
    v_color = ModelColor + (vertexColor * vertexColorBlendAmount);

    // Normals / lighting

    // Position of the vertex, in worldspace : M * position
    vec3 v_world_position = (ModelMatrix * vec4(position, 1.0)).xyz;

    // // // Set terrain color based on elevation
    if (meshID == 2) {
        if (DEBUG_VISUALS == 0) {
            if (v_world_position.y <= minElevation) {
                v_color = vec3(0.1, 0.1, 0.78); // BLUE
            } else if (v_world_position.y > (maxElevation - 7.5)) {
                v_color = vec3(.9, 1, .9); // WHITE
                // v_color = vec3(.44, .28, .24); // BROWN
            } else {
                v_color = vec3(.5, 0.9, .0); // GREEN
            }
        }
    }

    // Vector that goes from the vertex to the camera, in camera space.
    // In camera space, the camera is at the origin (0,0,0).
    vec3 v_cameraSpace = (vec4(v_world_position, 1.0)).xyz;
    v_eyeDirection_cameraSpace = vec3(0, 0, 0) - v_cameraSpace;

    // Vector that goes from the vertex to the light, in camera space. M is
    // ommited because it's identity.
    // vec3 lightPosition_cameraSpace = lightPosition_world;
    vec3 lightPosition_cameraSpace = (vec4(lightPosition_world, 1)).xyz;
    v_lightDirection_cameraSpace =
        lightPosition_cameraSpace + v_eyeDirection_cameraSpace;

    // Vertex normal in camera space
    v_normal_cameraSpace =
        (ViewMatrix * ModelMatrix * vec4(vertexNormal, 0.0)).xyz;
}