// This example is heavily based on the tutorial at https://open.gl

// OpenGL Helpers to reduce the clutter
#include "lib/Helpers.h"

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
// GLFW is necessary to handle the OpenGL context
#include <GLFW/glfw3.h>
#else
// GLFW is necessary to handle the OpenGL context
#include <GLFW/glfw3.h>
#endif

// GLM
#include "glm/gtx/string_cast.hpp"
#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/noise.hpp>
#include <glm/gtx/matrix_operation.hpp>
#include <glm/gtx/rotate_vector.hpp>

// Timer
#include <chrono>

// std lib
#include <cstdlib>
#include <ctime>
#include <future>
#include <math.h>

// Custom classes
#include <Mesh.h>
#include <SceneObject.h>
#include <SceneObjectList.h>
#include <Shader.h>
#include <State.h>
#include <fstream>
#include <iostream>
#include <set>
#include <vector>

State UI_STATE;

// Vectors used to make buffers
std::vector<Mesh> meshes;

// Vector color constants
glm::vec3 GREEN(0.0, 1.0, 0.0);
glm::vec3 RED(1.0, 0.0, 0.0);
glm::vec3 BLUE(0.0, 0.0, 1.0);
glm::vec3 BLACK(0.0, 0.0, 0.0);
glm::vec3 WHITE(1.0, 1.0, 1.0);
glm::vec3 GRAY(0.5, 0.5, 0.5);
glm::vec3 MAGENTA(1.0, 0.0, 1.0);
glm::vec3 TEAL(0.0, 1.0, 1.0);
glm::vec3 YELLOW(1.0, 1.0, 0.0);
glm::vec3 PINK(1.0, 0.5, 0.5);
glm::vec3 PURPLE(.67, 0.67, 1.0);

// Other constants
float HALF_PI = M_PI / 2.0;
float TWO_PI = 2 * M_PI;
float FoV = 90.0;                  // The vertical Field of View
float near_clipping_plane = 0.1f;  // Near clipping plane.
float far_clipping_plane = 100.0f; // Far clipping plane.

// Array of available colors
std::vector<glm::vec3> colors = {
    TEAL,    //
    MAGENTA, //
    YELLOW,  //
    PINK,    //
    PURPLE,  //
    GREEN,   //
    RED,     //
    BLUE,    //
    BLACK,   // unused
    WHITE,   //
    // GRAY // unused
};

glm::mat4 MIRROR_3D_NONE = glm::mat4(1.0);
glm::mat4 MIRROR_3D_X = glm::diagonal4x4(glm::vec4(-1, 1, 1, 1));
glm::mat4 MIRROR_3D_Y =
    glm::diagonal4x4(glm::vec4(1, 1, -1, 1)); // Y is Z in grid world
glm::mat4 MIRROR_3D_XY = glm::diagonal4x4(glm::vec4(-1, 1, -1, 1));

glm::mat2 MIRROR_2D_NONE = glm::mat2(1.0);
glm::mat2 MIRROR_2D_X = glm::diagonal2x2(glm::vec2(-1, 1));
glm::mat2 MIRROR_2D_Y =
    glm::diagonal2x2(glm::vec2(1, -1)); // Y is Z in grid world
glm::mat2 MIRROR_2D_XY = glm::diagonal2x2(glm::vec2(-1, -1));

// Index buffer stuff

GLuint elementbuffer;
GLuint mesh_1_index_buffer;
GLuint mesh_2_index_buffer;
GLuint mesh_3_index_buffer;

// Rendering to texture
GLuint framebufferOut;
GLuint renderedTextureOut;
GLuint depthrenderbuffer;
// Set the list of draw buffers.
GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};

// The fullscreen quad's FBO
GLuint quad_VertexArrayID;
VertexBufferObject VBO_QUAD;

std::vector<float> g_quad_vertex_buffer_data = {
    -1.0f, -1.0f, 0.0f, //
    1.0f,  -1.0f, 0.0f, //
    -1.0f, 1.0f,  0.0f, //
    -1.0f, 1.0f,  0.0f, //
    1.0f,  -1.0f, 0.0f, //
    1.0f,  1.0f,  0.0f, //
};

// Quad program
Program quad_program;

GLuint mesh_index_buffer_refs[3] = {
    mesh_1_index_buffer,
    mesh_2_index_buffer,
    mesh_3_index_buffer,
};

// Contains the vertex positions
// Command line flags
std::string V_SHADER_PATH = "-v";
std::string F_SHADER_PATH = "-f";
std::string V2_SHADER_PATH = "-v2";
std::string F2_SHADER_PATH = "-f2";
std::string G_SHADER_PATH = "-g";
std::string MESH_1_PATH = "-m1";
std::string MESH_2_PATH = "-m2";
std::string MESH_3_PATH = "-m3";

// Values for mesh paths
std::string mesh_1_path = "";
std::string mesh_2_path = "";
std::string mesh_3_path = "";

// Is debug mode enabled
bool DEBUG_MODE_ENABLED = false;
bool ASYNC_ENABLED = true;
std::mutex key_mutex;            // mutex for key handling
std::mutex terrain_update_mutex; // mutex for terrain updating handling

// Macro for visual debugging
// #define DEBUG_VISUALS 1

// MVP Matrixes
glm::mat4 ModelMatrix;
glm::mat4 ViewMatrix;
glm::mat4 ProjectionMatrix;
glm::mat4 MVPmatrix;
glm::vec3 ModelColor;

glm::vec3 INITIAL_CAMERA_POS(3, 4, 3);
glm::vec3 INITIAL_LIGHT_POSITION(-3, 3, 3);
float INITIAL_ZOOM_AMOUNT = 3.0f;
float SKY_LIGHTING = 1.0;

// Object containers
const int MAX_TERRAIN_TILES = 9;
SceneObjectList scene_objects;
std::vector<int> terrain_objects;
SceneObject *player;

// Scene object default scaling
float scaling_factors_by_mesh_id[] = {
    1.0f,  // robot
    0.25f, // box
    1.0f   // terrain
};

// Camera sensitivity
float y_sensitivity = -4.0;
float x_sensitivity = 8.0;
float z_sensitivity = 1.0;
int y_inversion = 1;
int x_inversion = 1;
int z_inversion = 1;
const float INF = 100000.0;
double last_mouse_x = 0.0f;
double last_mouse_y = 0.0f;

int last_used_color_idx = 0;

// Camera rotation
glm::mat4 R = glm::mat4(1.0f);

// Window Params
int WIDTH = 640;
int HEIGHT = 480;

// Key press state
std::set<int> KEYS;
std::set<int> REPEATABLE_KEYS;
std::vector<int> REPEATABLE_KEYS_ARR = {GLFW_KEY_W, GLFW_KEY_A, GLFW_KEY_S,
                                        GLFW_KEY_D};

int MODS;
int KEY_PRESS_THROTTLE_FRAMES = 40;

// BEGIN

const float PERLIN_OCTAVES = 6;
const float PERLIN_PERSISTENCE = 8;

#ifdef DEBUG_VISUALS
const unsigned int XMAX = 10;
const unsigned int YMAX = 10;
const unsigned int ELEVATION_SCALE = 2.0;
#else
const unsigned int XMAX = 50;
const unsigned int YMAX = 50;
const float ELEVATION_SCALE = 10.0;
#endif

const int MEMO_X_SIZE = XMAX * 20;
const int MEMO_Y_SIZE = YMAX * 20;
float MIN_ELEVATION = 0.0;
float MAX_ELEVATION = ELEVATION_SCALE;
float MOVE_SPEED = 0.75;
float E[MEMO_X_SIZE][MEMO_Y_SIZE]; // grid of elevation values for mesh

glm::vec2 player_position(0, 0);

// Function Prototypes / Forward Declarations
void createModelInstance(int meshId);
void shiftTerrainBlockInWorldGrid(SceneObject *so_ptr, float x, float y);
void rotateCamera(float rot_angle);
void translateSelectedModelInstance(glm::vec3 updated_translation);

// Function definitions
float perlinOctave(float x, float y) {
    double total = 0;
    double frequency = 1;
    int octaves = PERLIN_OCTAVES;
    double persistence = PERLIN_PERSISTENCE;
    double amplitude = 1;
    double maxValue = 0; // Used for normalizing result to 0.0 - 1.0
    for (int i = 0; i < octaves; i++) {
        glm::vec2 v(x * frequency, y * frequency);
        total += glm::perlin(v) * amplitude;

        maxValue += amplitude;

        amplitude *= persistence;
        frequency *= 2;
    }

    return (float)(total / maxValue);
}

void updateTerrain() {
    if (!terrain_update_mutex.try_lock()) {
        return;
    }
    for (int i = 0; i < terrain_objects.size(); i++) {
        SceneObject *so = scene_objects.at(terrain_objects[i]);
        glm::vec2 so_loc = so->getWorldGridPos(XMAX - 1, YMAX - 1);
        glm::vec2 p_loc = player->getWorldGridPos(XMAX - 1, YMAX - 1);
        glm::vec2 dir = p_loc - so_loc;
        float x_dist = dir.x;
        float y_dist = dir.y;
        glm::vec3 t(0.0f, 0.0f, 0.0f);

        if (abs(x_dist) >= 2.0) {
            float x = (glm::sign(x_dist) * 3.0f) * (XMAX - 1);
            t.x = x;
        }

        if (abs(y_dist) >= 2.0) {
            float y = (glm::sign(y_dist) * 3.0f) * (YMAX - 1);
            t.z = y;
        }
        if (abs(x_dist) > 0.001 || abs(y_dist) > 0.001) {
            so->translate(t);
        }

        glm::vec2 so_next_loc = so->getWorldGridPos(XMAX - 1, YMAX - 1);

        /*

            Mirroring is applied to whichever the odd-numbered
            grid coordinate is.

        */

        // Case 0: No mirroring
        glm::mat4 mirroring_mat = MIRROR_3D_NONE; // No mirroring by default

        // Set mirroring
        int so_x = (int)so_next_loc.x;
        int so_y = (int)so_next_loc.y;
        // Case 1: both are odd
        if (so_x % 2 != 0 && so_y % 2 != 0) {
            mirroring_mat = MIRROR_3D_XY;
            // Case 2: y is odd, x is even
        } else if (so_y % 2 != 0 && so_x % 2 == 0) {
            mirroring_mat = MIRROR_3D_Y;
            // Case 3: x is odd, y is even
        } else if (so_x % 2 != 0 && so_y % 2 == 0) {
            mirroring_mat = MIRROR_3D_X;
        }

        so->setMirroring(mirroring_mat);
    }
    terrain_update_mutex.unlock();
}

// END

float noise(float x, float y) {
    int xi = (int)(x * MEMO_X_SIZE);
    int yi = (int)(y * MEMO_Y_SIZE);

    // TODO: MEMOIZE
    if (E[xi][yi] > -INF) {
        return E[xi][yi];
    } else {
        float vertical_scaling = ELEVATION_SCALE;
        float vertical_offset = 0.5;
        float lateral_scaling = 0.1;
        float val = perlinOctave(x * lateral_scaling, y * lateral_scaling) *
                        vertical_scaling +
                    vertical_offset;
        if (val < MIN_ELEVATION) {
            val = MIN_ELEVATION;
        }
        E[xi][yi] = val; // is this necessary?
        return val;
    }
}

void initNoiseTexture() {
    // Create memo table
    for (int x = 0; x < MEMO_X_SIZE; x++) {
        for (int y = 0; y < MEMO_Y_SIZE; y++) {
            E[x][y] = -INF;
        }
    }
}

void shiftTerrainBlockInWorldGrid(SceneObject *so_ptr, float x, float y) {
    glm::vec3 t(x * (XMAX - 1), 0.0, y * (YMAX - 1));
    so_ptr->translate(t);
}

void initWorld() {
    glm::vec2 grid[9] = {
        glm::vec2(-1, 1),  glm::vec2(0, 1),  glm::vec2(1, 1),  //
        glm::vec2(-1, 0),  glm::vec2(0, 0),  glm::vec2(1, 0),  //
        glm::vec2(-1, -1), glm::vec2(0, -1), glm::vec2(1, -1), //
    };

    glm::mat4 grid_mirror[9] = {
        //
        MIRROR_3D_XY, MIRROR_3D_Y,    MIRROR_3D_XY, //
        MIRROR_3D_X,  MIRROR_3D_NONE, MIRROR_3D_X,  //
        MIRROR_3D_XY, MIRROR_3D_Y,    MIRROR_3D_XY, //
    };

    terrain_objects.reserve(9);
    for (int i = 0; i < 9; i++) {
        createModelInstance(2); // Add terrain
        SceneObject *so_ptr = scene_objects.at(i);
        so_ptr->setColor(i);
        so_ptr->setMirroring(grid_mirror[i]);
        glm::vec3 t(grid[i].x * (XMAX - 1), 0.0, grid[i].y * (YMAX - 1));
        so_ptr->translate(t);
        glm::vec2 w = so_ptr->getWorldGridPos(XMAX - 1, YMAX - 1);
        terrain_objects.emplace_back(i);
    }

    // scene_objects

    // Update positions an mirrorings

    createModelInstance(0);       // Add robot
    player = scene_objects.at(9); // Store ref to robot
    player->setColor(8);
    player->rotation_axis_idx = 1;
    player->rotate(0.75f);
    player->vertex_color_blend_amount = 1.0f;

    translateSelectedModelInstance(glm::vec3(
        XMAX / 2, player->mesh->mesh_radius * player->scale.y, YMAX / 2));
}

void loadMeshes() {
    // Load meshes

    Mesh robot(mesh_2_path, 0);
    Mesh bumpy_cube(mesh_3_path, 1);

    initNoiseTexture();

    // Mesh cube(mesh_1_path, 2);
    Mesh terrain(2, XMAX, YMAX, noise);

    // Add meshes to array
    meshes.push_back(robot);
    meshes.push_back(bumpy_cube);
    meshes.push_back(terrain);
}

void bufferMeshes() {
    for (int i = 0; i < meshes.size(); i++) {
        Mesh *mesh = &meshes[i];
        mesh->vertex_key_name = "mesh_" + std::to_string(i) + "_position";
        mesh->vertex_color_key_name =
            "mesh_" + std::to_string(i) + "_vertexColor";
        mesh->vertex_normal_key_name =
            "mesh_" + std::to_string(i) + "_vertexNormal";
    }
}

void initUIState() {
    UI_STATE.current_pan = {4.0f, 3.0f};
    UI_STATE.aspect_ratio = (float)WIDTH / (float)HEIGHT;
    UI_STATE.current_zoom = INITIAL_ZOOM_AMOUNT;
    UI_STATE.light_position = INITIAL_LIGHT_POSITION;
    UI_STATE.camera_position = INITIAL_CAMERA_POS;
    UI_STATE.should_use_secondary_renderer = false;
    UI_STATE.viewport_scaling = 1;

    // Set repeatable keys
    for (int k : REPEATABLE_KEYS_ARR) {
        REPEATABLE_KEYS.insert(k);
    }
}

void setModelMatrix(SceneObjectList &scene_objects, int model_idx) {
    SceneObject *so_ptr = scene_objects.at(model_idx);
    so_ptr->updateModel();
    ModelMatrix = so_ptr->ModelMatrix;
    ModelColor = so_ptr->color;
}

void setViewMatrix() {
    float x = UI_STATE.camera_position.x;
    float y = UI_STATE.camera_position.y;
    float z = UI_STATE.camera_position.z;

    glm::vec3 cameraPosition(x, y,
                             z); // the position of your camera, in world space

    glm::vec3 cameraTarget(0.0, 0.0,
                           0.0); // where you want to look at, in world space

    // Look at selected model center
    if (UI_STATE.selected_model_idx != -1) {
        SceneObject *so = scene_objects.at(UI_STATE.selected_model_idx);
        cameraTarget.x = so->translation.x;
        cameraTarget.y = so->translation.y;
        cameraTarget.z = so->translation.z;
    }

    int direction = 1;

    glm::vec3 upVector( // determines orientation of camer
        0,              // x
        direction,      // 1 = right side up, -1 = upside-down
        0               // z
    );

    // aka CameraMatrix
    ViewMatrix = glm::lookAt(cameraPosition, cameraTarget, upVector);
}

void setProjectionMatrix() {
    float aspect_ratio =
        UI_STATE
            .aspect_ratio; // Aspect Ratio. Depends on the size of your window.

    // Generate each projection matrix based on UI STATE
    if (UI_STATE.should_use_orthographic_camera) {
        ProjectionMatrix = glm::ortho(
            -10.0f * aspect_ratio, // left, world coord, scaled by aspect ratio
            10.0f * aspect_ratio,  // right, world coord, scaled by aspect ratio
            -10.0f,                // bottom, world coord
            10.0f,                 // top, world coords
            0.0f,                  // near plane
            far_clipping_plane     // far plane
        );
    } else {
        ProjectionMatrix = glm::perspective( //
            glm::radians(FoV),               // field of view
            aspect_ratio,                    // aspect ratio
            near_clipping_plane,             // near plane
            far_clipping_plane               // far plane
        );
    }
}

void setMVPMatrix() {
    setViewMatrix();
    setProjectionMatrix();
    MVPmatrix = ProjectionMatrix * ViewMatrix * ModelMatrix;
}

// Updates camera movements for ortho camera
glm::vec3 mapCameraMovementForOrthoCamera(glm::vec3 cam_move) {
    glm::vec3 result(0.0f);
    if (cam_move.x < 0.0) {
        result.x = -1.0;
    }
    if (cam_move.x > 0.0) {
        result.x = 1.0;
    }

    if (cam_move.y < 0.0) {
        result.y = -1.0;
    }
    if (cam_move.y > 0.0) {
        result.y = 1.0;
    }

    if (cam_move.z < 0.0) {
        result.z = -1.0;
    }
    if (cam_move.z > 0.0) {
        result.z = 1.0;
    }

    return result;
}

// Uses spherical coordinates to move around the camera on mouse drag
void moveCameraPosition(glm::vec3 cameraMovement) {
    glm::vec3 cam = UI_STATE.camera_position;
    glm::vec3 cam_move(                                 //
        cameraMovement.x * x_sensitivity * x_inversion, // x
        cameraMovement.y * y_sensitivity * y_inversion, // y,
        cameraMovement.z * z_sensitivity * z_inversion  // z
    );

    // Normalize current position to spherical (rho, theta, phi)
    float curr_rho = sqrt(pow(cam.x, 2) + pow(cam.y, 2) + pow(cam.z, 2));
    float curr_theta = atan2(cam.y, cam.x);
    float curr_phi = acos(cam.z / curr_rho);

    if ( // if signs don't match on the left/right
        (cam.x > 0.0 && curr_theta < 0.0) || //
        (cam.x < 0.0 && curr_theta > 0.0)    //
    ) {
        cam.x = cam.x * -1.0;
    }

    if ( // if signs don't match on the up/down
        (cam.y > 0.0 && curr_phi < 0.0) || //
        (cam.y < 0.0 && curr_phi > 0.0)    //
    ) {
        cam.y = cam.y * -1.0;
    }

    float next_rho = curr_rho + cam_move.z;
    float next_theta = curr_theta + cam_move.x;
    float next_phi = (curr_phi + cam_move.y); // TODO: implement zoom

    float next_x = next_rho * sin(next_phi) * cos(next_theta);
    float next_y = next_rho * sin(next_phi) * sin(next_theta);
    float next_z = next_rho * cos(next_phi);

    UI_STATE.camera_position = glm::vec3(next_x, next_y, next_z);
}

void translateCamera(glm::vec3 cam_move) {
    glm::mat4 T = glm::translate(glm::mat4(1.0f), cam_move);
    glm::vec3 cam = UI_STATE.camera_position - player->translation;
    glm::vec4 next = T * glm::vec4(cam.x, cam.y, cam.z, 1.0f);
    glm::vec3 norm_next = glm::normalize(glm::vec3(next));
    UI_STATE.camera_position =
        player->translation + (norm_next * UI_STATE.current_zoom);
}

void translateLight(glm::vec3 light_move) {
    glm::mat4 T = glm::translate(glm::mat4(1.0f), light_move);
    glm::vec3 l = UI_STATE.light_position - player->translation;
    glm::vec4 next = T * glm::vec4(l.x, l.y, l.z, 1.0f);
    UI_STATE.light_position = player->translation + glm::vec3(next);
}

void createModelInstance(int meshId) {
    std::cout << "Creating an instance of mesh: " << meshId << std::endl;
    float scaling_factor = scaling_factors_by_mesh_id[meshId];
    int next_color_idx = last_used_color_idx + 1;
    if (next_color_idx < 0 || next_color_idx >= colors.size()) {
        next_color_idx = 0;
    }
    SceneObject o(
        glm::vec3(0.0, 0.0, 0.0), // translation
        glm::vec3(scaling_factor, scaling_factor, scaling_factor), // scale
        colors[next_color_idx],                                    // color
        &meshes[meshId]);
    o.colors = &colors; // attach reference to available colors
    scene_objects.push(o);

    UI_STATE.selected_model_idx = scene_objects.size() - 1;
    last_used_color_idx = next_color_idx; // updates last used color
}

float getYfromXZ(float x, float z) {
    glm::vec2 in_vec(x, z);
    glm::vec2 p_loc = player->getWorldGridPos(XMAX, YMAX);

    /*

        Mirroring is applied to whichever the odd-numbered
        grid coordinate is.

    */

    // Set mirroring
    int so_x = (int)p_loc.x;
    int so_y = (int)p_loc.y;
    glm::vec2 out_vec = in_vec;
    // Case 1: both are odd
    if (so_x % 2 != 0 && so_y % 2 != 0) {
        out_vec.x = 1.0f - out_vec.x;
        out_vec.y = 1.0f - out_vec.y;
        // Case 2: y is odd, x is even
    } else if (so_y % 2 != 0 && so_x % 2 == 0) {
        // out_vec.x = 1.0f - out_vec.x;
        out_vec.y = 1.0f - out_vec.y;
        // Case 3: x is odd, y is even
    } else if (so_x % 2 != 0 && so_y % 2 == 0) {
        // mirroring_mat = MIRROR_2D_X;
        out_vec.x = 1.0f - out_vec.x;
    }

    return noise(out_vec.x, out_vec.y);
}

void translateSelectedModelInstance(glm::vec3 updated_translation) {
    float elevation_scale = ELEVATION_SCALE;
    float lateral_scaling = 0.1;
    if (UI_STATE.selected_model_idx != -1) {
        SceneObject *so = scene_objects.at(UI_STATE.selected_model_idx);
        float last_elevation = so->elevation_offset;
        float x = ((int)(so->translation.x + updated_translation.x) % XMAX) /
                  (float)XMAX;
        float z = ((int)(so->translation.z + updated_translation.z) % YMAX) /
                  (float)YMAX;
        float height = so->mesh->mesh_radius * so->scale.y;
        float next_elevation = getYfromXZ(x, z) + height;
        so->elevation_offset = next_elevation;
        float delta_el = next_elevation - last_elevation;

        updated_translation.y = updated_translation.y + delta_el;

        so->translate(updated_translation);
        translateCamera(updated_translation);
        translateLight(updated_translation);

        // Asynchronously apply terrain updates accordingly
        std::async(std::launch::async, updateTerrain);
    }
}

void movePlayer(int key, float scale) {
    // Default case is forward
    glm::vec3 direction =
        glm::vec3(player->translation.x - UI_STATE.camera_position.x, // x
                  0.0,                                                // y
                  player->translation.z - UI_STATE.camera_position.z  // z
        );
    switch (key) {
    // Backward is invert
    case GLFW_KEY_S:
        direction = direction * -1.0f;
        break;
    case GLFW_KEY_A:
        direction = glm::rotate(direction, 1.5f, glm::vec3(0, 1, 0));
        break;
    case GLFW_KEY_D:
        direction = glm::rotate(direction, 1.5f, glm::vec3(0, -1, 0));
        break;
    case GLFW_KEY_SEMICOLON:
        direction = glm::vec3(0, 1.0f, 0); // UP
        break;
    case GLFW_KEY_L:
        if (player->translation.y >=
            (MIN_ELEVATION + (player->mesh->mesh_radius * 4.0f))) {
            direction = glm::vec3(0, -1.0f, 0); // DOWN
        }
        break;
    }
    glm::vec3 delta_dir = glm::normalize(direction) * scale;
    translateSelectedModelInstance(delta_dir);
}

void rotateCamera(float rot_angle) {
    glm::vec3 p_pos = player->translation;
    glm::vec3 cam = UI_STATE.camera_position;
    glm::vec3 d_pos = cam - p_pos;

    glm::mat4 R = glm::rotate(glm::mat4(1.0f), // ID matrix
                              rot_angle,       // angle of rotation
                              // axis of rotation
                              glm::vec3(0, // x
                                        1, // y
                                        0) // z
    );

    glm::vec4 c_pos = glm::vec4(cam - p_pos, 0);
    glm::vec4 next_pos = (R * c_pos) + glm::vec4(p_pos, 0);
    UI_STATE.camera_position = glm::vec3(next_pos);
}

void moveCamera(int key) {
    switch (key) {
    case GLFW_KEY_LEFT_BRACKET:
        moveCameraPosition(glm::vec3(0, 0, 1.0));
        break;
    case GLFW_KEY_RIGHT_BRACKET:
        moveCameraPosition(glm::vec3(0, 0, -1.0));
        break;
    case GLFW_KEY_LEFT:
        rotateCamera(-0.25);
        player->rotate(-0.25);
        break;
    case GLFW_KEY_RIGHT:
        rotateCamera(0.25);
        player->rotate(0.25);
        break;
    case GLFW_KEY_UP:
        translateCamera(glm::vec3(0, 1, 0));
        break;
    case GLFW_KEY_DOWN:
        translateCamera(glm::vec3(0, -1, 0));
        break;
    case GLFW_KEY_COMMA:
        UI_STATE.current_zoom = UI_STATE.current_zoom * 0.5;
        translateCamera(glm::vec3(1, 1, 1));
        break;
    case GLFW_KEY_PERIOD:
        UI_STATE.current_zoom = UI_STATE.current_zoom * 2;
        translateCamera(glm::vec3(1, 1, 1));
        break;
    }
}

void handle_key(int key, int mods, float scale) {
    if (!key_mutex.try_lock()) {
        return; // mutex unavailable, skip key handling
    }
    // Update the position of the first vertex if the keys 1,2, or 3 are pressed
    switch (key) {
    case GLFW_KEY_1:
        UI_STATE.camera_position = INITIAL_CAMERA_POS + player->translation;
        UI_STATE.current_zoom = INITIAL_ZOOM_AMOUNT;
        break;
    // Model Transformations
    case GLFW_KEY_EQUAL:
        SKY_LIGHTING = SKY_LIGHTING * 2.0f;
        break;
    case GLFW_KEY_MINUS:
        SKY_LIGHTING = SKY_LIGHTING / 2.0f;
        break;
    case GLFW_KEY_L:
    case GLFW_KEY_SEMICOLON:
    case GLFW_KEY_W:
    case GLFW_KEY_S:
    case GLFW_KEY_A:
    case GLFW_KEY_D:
        movePlayer(key, scale);
        break;
    // Camera movement
    case GLFW_KEY_LEFT_BRACKET:
    case GLFW_KEY_RIGHT_BRACKET:
    case GLFW_KEY_LEFT:
    case GLFW_KEY_RIGHT:
    case GLFW_KEY_UP:
    case GLFW_KEY_DOWN:
    case GLFW_KEY_COMMA:
    case GLFW_KEY_PERIOD:
        moveCamera(key);
        break;
    case GLFW_KEY_Z:
        UI_STATE.should_use_secondary_renderer =
            !UI_STATE.should_use_secondary_renderer;
        break;
    case GLFW_KEY_PAGE_DOWN:
        UI_STATE.pixel_width = UI_STATE.pixel_width / 2.0;
        break;
    case GLFW_KEY_PAGE_UP:
        UI_STATE.pixel_width = UI_STATE.pixel_width * 2.0;
        break;
    default:
        break;
    }
    key_mutex.unlock(); // release mutex
}

// Keyboard Callback
void key_callback(GLFWwindow *window, int key, int scancode, int action,
                  int mods) {
    MODS = mods;
    // Avoids repeated calls when key is lifted.  We only care about keypresses
    if (action != GLFW_PRESS) {
        // Handle any key releases
        if (action == GLFW_RELEASE) {
            // Code here...
            KEYS.erase(key);
        }
        return;
    }
    KEYS.insert(key);
}

// Mouse callback
void cursor_position_callback(GLFWwindow *window, double xpos, double ypos) {
    if (UI_STATE.is_left_mouse_pressed) {
        if (!key_mutex.try_lock()) {
            return;
        }
        // Get the position of the mouse in the window
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        // Get the size of the window
        int width, height;
        glfwGetWindowSize(window, &width, &height);

        // Convert screen position to [-1,1] coords
        double mouse_x = ((xpos / double(width)) * 2) - 1;
        double mouse_y = (((height - 1 - ypos) / double(height)) * 2) -
                         1; // NOTE: y axis is flipped in glfw
        if (abs(last_mouse_x) < 0.01 && abs(last_mouse_y) < 0.01) {
            last_mouse_x = mouse_x;
            last_mouse_y = mouse_y;
        } else {
            double delta_mouse_x = mouse_x - last_mouse_x;
            double delta_mouse_y = mouse_y - last_mouse_y;
            last_mouse_x = mouse_x;
            last_mouse_y = mouse_y;

            // NOTE: x rotates camera and player
            float rot_x = delta_mouse_x * MOVE_SPEED * x_sensitivity;
            rotateCamera(rot_x);
            player->rotate(rot_x);

            // NOTE: y tilts camera, not player
            translateCamera(
                glm::vec3(0, delta_mouse_y * MOVE_SPEED * y_sensitivity, 0));
        }
        key_mutex.unlock();
    }
}

void mouse_button_callback(GLFWwindow *window, int button, int action,
                           int mods) {

    // Get the position of the mouse in the window
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    // Get the size of the window
    int width, height;
    glfwGetWindowSize(window, &width, &height);

    // Convert screen position to [-1,1] coords
    double mouse_x = ((xpos / double(width)) * 2) - 1;
    double mouse_y = (((height - 1 - ypos) / double(height)) * 2) -
                     1; // NOTE: y axis is flipped in glfw

    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        UI_STATE.is_left_mouse_pressed = true;
    }

    // Handle mouse releases
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {

        UI_STATE.is_left_mouse_pressed = false;
        last_mouse_x = 0.0;
        last_mouse_y = 0.0;
    }

    // Handle mouse press
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        UI_STATE.is_left_mouse_pressed = true;
    }
}

void setAspectRatioViewMatrix(int width, int height) {
    WIDTH = width;
    HEIGHT = height;
    float aspect_ratio = (float)width / (float)height;
    UI_STATE.aspect_ratio = aspect_ratio;
    glViewport(0, 0, width, height);
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    int w = width;
    int h = height;
    setAspectRatioViewMatrix(w, h);
}

// Generates an index buffer and binds it to element buffer
void initIndexBuffer() {
    for (int i = 0; i < meshes.size(); i++) {
        Mesh *m = &meshes[i];
        std::cout << "Init buffer: " << i << " size: " << m->indices.size()
                  << std::endl;
        glGenBuffers(1, &mesh_index_buffer_refs[i]);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh_index_buffer_refs[i]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,                  //
                     m->indices.size() * sizeof(unsigned int), //
                     &m->indices[0],
                     GL_STATIC_DRAW //
        );
    }
}

bool initOutBuffer() {
    // Generate buffer to store texture
    glGenFramebuffers(1, &framebufferOut);
    glBindFramebuffer(GL_FRAMEBUFFER, framebufferOut);

    // Generate texture
    glGenTextures(1, &renderedTextureOut);

    // "Bind" the newly created texture : all future texture functions will
    // modify this texture
    glBindTexture(GL_TEXTURE_2D, renderedTextureOut);

    // Give an empty image to OpenGL ( the last "0" )
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WIDTH, HEIGHT, 0, GL_RGB,
                 GL_UNSIGNED_BYTE, 0);

    // Poor filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Depth buffer
    glGenRenderbuffers(1, &depthrenderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthrenderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, WIDTH, HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                              GL_RENDERBUFFER, depthrenderbuffer);

    // Set "renderedTexture" as our colour attachement #0
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                         renderedTextureOut, 0);

    glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers

    // Always check that our framebuffer is ok
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        return false;

    return true;
}

void initQuadBuffer() {
    // Setup VBO for quad buffer
    VBO_QUAD.init();

    // Send vertices to VBO
    VBO_QUAD.updateForStaticDraw(g_quad_vertex_buffer_data);
}

void configure_from_args(int argc, char *argv[], std::string &v_shader_path,
                         std::string &f_shader_path, std::string &g_shader_path,
                         std::string &v2_shader_path,
                         std::string &f2_shader_path) {
    int arg_idx = 1;
    while (arg_idx < argc) {
        if (argv[arg_idx] == V_SHADER_PATH && (arg_idx + 1) < argc) {
            v_shader_path = argv[arg_idx + 1];
        } else if (argv[arg_idx] == F_SHADER_PATH && (arg_idx + 1) < argc) {
            f_shader_path = argv[arg_idx + 1];
        } else if (argv[arg_idx] == V2_SHADER_PATH && (arg_idx + 1) < argc) {
            v2_shader_path = argv[arg_idx + 1];
        } else if (argv[arg_idx] == F2_SHADER_PATH && (arg_idx + 1) < argc) {
            f2_shader_path = argv[arg_idx + 1];
        } else if (argv[arg_idx] == MESH_1_PATH && (arg_idx + 1) < argc) {
            mesh_1_path = argv[arg_idx + 1];
        } else if (argv[arg_idx] == MESH_2_PATH && (arg_idx + 1) < argc) {
            mesh_2_path = argv[arg_idx + 1];
        } else if (argv[arg_idx] == MESH_3_PATH && (arg_idx + 1) < argc) {
            mesh_3_path = argv[arg_idx + 1];
        } else if (argv[arg_idx] == G_SHADER_PATH && (arg_idx + 1) < argc) {
            g_shader_path = argv[arg_idx + 1];
        }
        arg_idx++;
    }
}

int main(int argc, char *argv[]) {
    // Parse args
    std::string v_shader_path;
    std::string f_shader_path;
    std::string g_shader_path;
    std::string f2_shader_path;
    std::string v2_shader_path;
    configure_from_args(argc, argv, v_shader_path, f_shader_path, g_shader_path,
                        v2_shader_path, f2_shader_path);

    GLFWwindow *window;

    // Initialize the library
    if (!glfwInit())
        return -1;

    // Activate supersampling
    glfwWindowHint(GLFW_SAMPLES, 8);

    // Ensure that we get at least a 3.2 context
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);

    // On apple we have to load a core profile with forward compatibility
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // Create a windowed mode window and its OpenGL context
    window = glfwCreateWindow(WIDTH, HEIGHT, "Infinity Terrain", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    // Make the window's context current
    glfwMakeContextCurrent(window);

#ifndef __APPLE__
    glewExperimental = true;
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        /* Problem: glewInit failed, something is seriously wrong. */
        fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
    }
    glGetError(); // pull and savely ignonre unhandled errors like
                  // GL_INVALID_ENUM
    fprintf(stdout, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
#endif

    // Get real dimensions
    glfwGetFramebufferSize(window, &WIDTH, &HEIGHT);

    int major, minor, rev;
    major = glfwGetWindowAttrib(window, GLFW_CONTEXT_VERSION_MAJOR);
    minor = glfwGetWindowAttrib(window, GLFW_CONTEXT_VERSION_MINOR);
    rev = glfwGetWindowAttrib(window, GLFW_CONTEXT_REVISION);
    printf("OpenGL version recieved: %d.%d.%d\n", major, minor, rev);
    printf("Supported OpenGL is %s\n", (const char *)glGetString(GL_VERSION));
    printf("Supported GLSL is %s\n",
           (const char *)glGetString(GL_SHADING_LANGUAGE_VERSION));

    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    check_gl_error();
    glBindVertexArray(VertexArrayID);
    check_gl_error();

    // Initialize UI state
    initUIState();

    loadMeshes();

    // Test cube
    bufferMeshes();

    // Initialize the OpenGL Program
    // A program controls the OpenGL pipeline and it must contains
    // at least a vertex shader and a fragment shader to be valid

    Program program;
    Shader g_shader(3);
    Shader v_shader(2);
    Shader f_shader(1);

    // Secondary shader program
    Shader v2_shader(2);
    Shader f2_shader(1);
    v2_shader = Shader(v2_shader_path);
    f2_shader = Shader(f2_shader_path);

    if (v_shader_path != "") {
        v_shader = Shader(v_shader_path);
    }
    if (f_shader_path != "") {
        f_shader = Shader(f_shader_path);
    }
    if (g_shader_path != "") {
        g_shader = Shader(g_shader_path);
    }

    program.init(v_shader.read(), f_shader.read(), g_shader.read(), "outColor");
    program.bind();

    for (int i = 0; i < meshes.size(); i++) {
        std::cout << "Binding VBO for mesh : " << i << std::endl;
        Mesh *mesh = &meshes[i];
        // The vertex shader wants the position of the vertices as an input.
        // The following line connects the VBO we defined above with the
        // position "slot" in the vertex shader

        program.bindVertexAttribArray(mesh->vertex_key_name, mesh->VBO);

        // Bind normals buffer
        program.bindVertexAttribArray(mesh->vertex_normal_key_name,
                                      mesh->VBO_VN);

        // Bind vertex colors buffer
        program.bindVertexAttribArray(mesh->vertex_color_key_name, mesh->VBO_C);
    }

    // Create initial scene objects
    initWorld();

    initQuadBuffer(); // For texture

    // Create and compile our GLSL program from the shaders
    quad_program.init(v2_shader.read(), f2_shader.read(), "", "color");
    quad_program.bind();

    initOutBuffer();
    // Init index buffer
    initIndexBuffer();

    // Enable depth test
    glEnable(GL_DEPTH_TEST);

    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);

    // Save the current time --- it will be used to dynamically change the
    // triangle color
    auto t_start = std::chrono::high_resolution_clock::now();

    // Bind callbacks

    // Register the keyboard callback
    glfwSetKeyCallback(window, key_callback);

    // // Register the mouse callback
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    // Register the cursor position callback
    glfwSetCursorPosCallback(window, cursor_position_callback);

    // Update viewport
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    auto t_now = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration_cast<std::chrono::duration<float>>(
                     t_now - t_start)
                     .count();
    float last_frame_change_time = 0.0f;
    int counter = 1;
    int frame_counter = 0;
    // Loop until the user closes the window
    while (!glfwWindowShouldClose(window)) {
        // std::cout << "keys:";

        // Set the uniform value depending on the time difference
        t_now = std::chrono::high_resolution_clock::now();
        frame_counter += 1;
        float last_iteration_time = time;
        time = std::chrono::duration_cast<std::chrono::duration<float>>(t_now -
                                                                        t_start)
                   .count();
        float delta = time - last_iteration_time;
        last_frame_change_time += delta;
        if (last_frame_change_time >= 1.000) {
            last_frame_change_time = 0.0f;
        }

        if (frame_counter % KEY_PRESS_THROTTLE_FRAMES == 0) {
            // Fire keys
            std::set<int>::iterator it;
            for (it = KEYS.begin(); it != KEYS.end(); ++it) {
                if (ASYNC_ENABLED) {
                    // Asynchronously fire key callbacks
                    std::async(std::launch::async, handle_key, *it, MODS,
                               MOVE_SPEED);
                } else {
                    handle_key(*it, MODS, MOVE_SPEED);
                }
            }
        }

        // Set output framebuffer
        if (UI_STATE.should_use_secondary_renderer) {
            glBindFramebuffer(GL_FRAMEBUFFER, framebufferOut);
            player->setColor(
                9); // Color VBO_C is somehow dropped when pixel mode is enabled
        } else {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        glViewport(0, 0, WIDTH, HEIGHT);

        // Bind your program
        program.bind();

        // TODO: have this respond to window resizing

        // Clear the framebuffer
        glClearColor(0.5f * SKY_LIGHTING, 0.5f * SKY_LIGHTING,
                     1.0f * SKY_LIGHTING, 1.0f);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUniform1f(program.uniform("time"), time);
        glUniform1f(program.uniform("delta"), delta);
        glUniform3f(program.uniform("blinkingColor"),
                    (float)(sin(time * 4.0f) + 1.0f) / 2.0f, 0.0f, 0.0f);

        glUniform1i(program.uniform("shadingMode"), 0);
        glUniform3f(program.uniform("backgroundColor"), 0.5f, 0.5f, 0.5f);
        glUniform3f(program.uniform("lightPosition_world"),
                    UI_STATE.light_position.x, UI_STATE.light_position.y,
                    UI_STATE.light_position.z);
        glUniform2i(program.uniform("iResolution"), WIDTH, HEIGHT);
        glUniform1f(program.uniform("minElevation"), MIN_ELEVATION);
        glUniform1f(program.uniform("maxElevation"), MAX_ELEVATION);

// Use special coloring and parameters to debug visuals
#ifdef DEBUG_VISUALS
        glUniform1i(program.uniform("DEBUG_VISUALS"), 1);
#else
        glUniform1i(program.uniform("DEBUG_VISUALS"), 0);
#endif
        // Texture (end)

        // Loops through each scene object, drawing each one
        for (int i = 0; i < scene_objects.size(); i++) {
            SceneObject *so = scene_objects.at(i);
            // Apply updates to the selected object
            if (i == UI_STATE.selected_model_idx) {
                glUniform1i(program.uniform("isSelected"), 1);
            } else {
                glUniform1i(program.uniform("isSelected"), 0);
            }
            glUniform3f(program.uniform("ModelColor"), so->color.x, so->color.y,
                        so->color.z);

            glUniform1i(program.uniform("shadingMode"), so->shading_mode);

            glUniform1f(program.uniform("vertexColorBlendAmount"),
                        so->vertex_color_blend_amount);

            // Find model matrix in the scene objects list
            setModelMatrix(scene_objects, i);

            // Update MVP matrix based on state
            setMVPMatrix();

            // Send MVP matrix as uniform
            glUniformMatrix4fv(program.uniform("MVPMatrix"), 1, GLFW_FALSE,
                               &MVPmatrix[0][0]);

            // Send M matrix as a uniform. TODO: update for multiple models
            glUniformMatrix4fv(program.uniform("ModelMatrix"), 1, GLFW_FALSE,
                               &ModelMatrix[0][0]);

            glUniformMatrix4fv(program.uniform("MirrorMatrix"), 1, GLFW_FALSE,
                               &so->MirrorMatrix[0][0]);

            // Send V matrix as a uniform. TODO: update for multiple models
            glUniformMatrix4fv(program.uniform("ViewMatrix"), 1, GLFW_FALSE,
                               &ViewMatrix[0][0]);

            // Send P mattrtix
            glUniformMatrix4fv(program.uniform("ProjectionMatrix"), 1,
                               GLFW_FALSE, &ProjectionMatrix[0][0]);

            // Set mesh identifier for shader
            glUniform1i(program.uniform("meshID"), so->mesh->id);

            // TODO: add configurable light position

            // DYNAMIC ARRAY BUFFER
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,
                         mesh_index_buffer_refs[so->mesh->id]);

            // // Draw the triangles !
            glDrawElements(GL_TRIANGLES,             // mode
                           so->mesh->indices.size(), // count
                           GL_UNSIGNED_INT,          // type
                                                     //    &so->mesh->indices[0]
                           (void *)(0) // element array buffer offset
            );
        }

        // Handle secondary FX processing
        if (UI_STATE.should_use_secondary_renderer) {
            // glDisableVertexAttribArray(0);
            // Render to screen
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            glViewport(0, 0, WIDTH, HEIGHT);

            // Clear the screen
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            quad_program.bind();

            // Bind our texture in Texture Unit 0
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, renderedTextureOut);

            glUniform1i(quad_program.uniform("renderedTexture"), 0);
            glUniform1f(quad_program.uniform("time"),
                        (float)(glfwGetTime() * 10.0f));
            glUniform1f(quad_program.uniform("pixelWidth"),
                        UI_STATE.pixel_width);

            // 1rst attribute buffer : vertices
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, // attribute 0. No particular reason for
                                     // but
                                     // must match the layout in the shader.
                                  3, // size
                                  GL_FLOAT, // type
                                  GL_FALSE, // normalized?
                                  0,        // stride
                                  (void *)0 // array buffer offset
            );

            // Draw the triangles !
            glDrawArrays(GL_TRIANGLES, 0,
                         6); // 2*3 indices starting at 0 -> 2 triangles

            // glDisableVertexAttribArray(0);
        }

        // glDisableVertexAttribArray(0);

        // Swap front and back buffers
        glfwSwapBuffers(window);

        // Poll for and process events
        glfwPollEvents();

        if (DEBUG_MODE_ENABLED) {
            // DEBUG: renders a few test frames then exits early to see glsl
            // errors
            if (counter > 10) {
                exit(1);
            } else {
                counter++;
            }
        }
    }

    // Deallocate opengl memory
    program.free();
    quad_program.free();

    for (int i = 0; i < meshes.size(); i++) {
        Mesh *m = &meshes[i];
        m->VBO.free();
        m->VBO_VN.free();
        m->VBO_C.free();
    }

    // Deallocate glfw internals
    glfwTerminate();
    return 0;
}
