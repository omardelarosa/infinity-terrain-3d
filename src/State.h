#pragma once

#include <Eigen/Core>

struct State {
    enum MODES {
        NONE = 0,
        INSERT = 1,
        TRANSLATION = 2,
        COLOR = 3,
        DELETE = 4,
        ANIMATE = 5
    };
    int current_mode = 0;
    bool should_use_orthographic_camera = false;
    int selected_model_idx = -1;
    int selected_polygon_idx = -1;
    int selected_vertex_polygon_idx = -1;
    int selected_vertex_idx = -1;
    bool has_set_last_move = false;
    bool is_left_mouse_pressed = false;
    bool should_use_secondary_renderer = false;
    float viewport_scaling = 1.0f; // retina screen?
    float aspect_ratio;
    float current_zoom = 1.0f;
    float pixel_width = 320.0f; // for pixelation
    int animation_start_frame = -1;
    Eigen::Vector2f current_pan;
    Eigen::MatrixXf view_matrix;
    glm::vec3 light_position = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 camera_position = glm::vec3(0.0f, 0.0f, 0.0f);
};