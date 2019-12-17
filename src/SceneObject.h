#pragma once

// GLM
#include "Mesh.h"
#include "glm/gtx/string_cast.hpp"
#include <glm/ext.hpp>
#include <glm/glm.hpp>

// A class representing an object for drawing
class SceneObject {
  public:
    glm::mat4 TranslationMatrix;
    glm::mat4 ScalingMatrix;
    glm::mat4 RotationMatrix;
    SceneObject(glm::vec3 _translation, glm::vec3 _scale, glm::vec3 _color,
                Mesh *_mesh) {
        translation = _translation;
        scale = _scale;
        color = _color;
        color_idx = 0;
        rotation_angle = 0.0f;
        rotation_axis_idx = 0;
        mesh = _mesh;
        shading_mode = 2;               // Defaults to wireframe + color
        MirrorMatrix = glm::mat4(1.0f); // Identity by default
    }
    glm::vec3 rotation_axes[3] = {
        glm::vec3(1, 0, 0), // x-axis
        glm::vec3(0, 1, 0), // y-axis
        glm::vec3(0, 0, 1), // z-axis
    };
    int shading_mode = 2;
    glm::vec3 translation; // translation
    glm::vec3 scale;       // scale
    glm::vec3 color;       // Maybe this should not be black?
    float vertex_color_blend_amount = 0.0f;
    float elevation_offset = 0.0;
    int color_idx;
    glm::mat4 MirrorMatrix;
    vector<glm::vec3> *colors = nullptr;
    unsigned int rotation_axis_idx;
    float rotation_angle;
    glm::mat4 ModelMatrix = glm::mat4(1.0f); // model matrix
    Mesh *mesh = nullptr;
    glm::mat4 updateModel() {

        ScalingMatrix = glm::scale(glm::mat4(1.0f), scale);
        TranslationMatrix = glm::translate(glm::mat4(1.0f), translation);
        glm::mat4 CenterTranslationMatrix =
            glm::translate(glm::mat4(1.0f), mesh->center * -1.0f);
        glm::mat4 CenterTranslationMatrixInv =
            glm::inverse(CenterTranslationMatrix);
        RotationMatrix =
            CenterTranslationMatrixInv *         // return to original pos
            glm::rotate(                         //
                glm::mat4(1.0f),                 // identity matrix
                rotation_angle,                  // determine theta angle
                rotation_axes[rotation_axis_idx] // get current axis
                ) *                              //
            CenterTranslationMatrix;             // translate to barycenter
        // Apply all matrix transformations

        ModelMatrix =
            TranslationMatrix * ScalingMatrix * RotationMatrix * MirrorMatrix;

        return ModelMatrix;
    }

    float distFrom(glm::vec3 pointInSpace) {
        glm::vec3 world_center = ModelMatrix * glm::vec4(mesh->center, 1);
        return glm::distance(world_center, pointInSpace);
    }

    void toggleColor() {
        if (colors != nullptr) {
            int next_color_idx = color_idx + 1;
            if (next_color_idx >= colors->size()) {
                next_color_idx = 0;
            }
            glm::vec3 next_color = colors->at(next_color_idx);
            std::cout << "Updating color to " << next_color_idx
                      << glm::to_string(next_color) << std::endl;
            color = next_color;
            color_idx = next_color_idx;
        }
    }

    void setColor(int idx) {
        color_idx = idx % colors->size();
        color = colors->at(color_idx);
    }

    void setMirroring(glm::mat4 m) {
        glm::mat4 CenterTranslationMatrix =
            glm::translate(glm::mat4(1.0f), mesh->center * -1.0f);
        glm::mat4 CenterTranslationMatrixInv =
            glm::inverse(CenterTranslationMatrix);
        // TODO: Apply Mirroring
        MirrorMatrix = CenterTranslationMatrixInv * // return to original pos
                       m *                          // apply mirroring
                       CenterTranslationMatrix;     // center
    }

    glm::vec2 getWorldGridPos(int XMAX, int YMAX) {
        int world_grid_x = (int)floor((float)translation.x / (float)(XMAX));
        int world_grid_y = (int)floor((float)translation.z / (float)(YMAX));
        return glm::vec2(world_grid_x, world_grid_y);
    }

    void toggleShadingMode(int next_shading_mode) {
        if (next_shading_mode == -1) {
            next_shading_mode = shading_mode + 1;
        }
        if (next_shading_mode > 2 || next_shading_mode < 0) {
            next_shading_mode = 0;
        }
        shading_mode = next_shading_mode;
    }

    void translate(const glm::vec3 &updated_translation) {
        translation = translation + updated_translation;
        updateModel();
    }

    void rotate(float angle_update) {
        rotation_angle = rotation_angle + angle_update;
        updateModel();
    }

    void shiftRotationAxis() {
        rotation_axis_idx = ((rotation_axis_idx + 1) % 3);
    }

    void updateScale(float updated_scale) {
        scale = scale * updated_scale;
        updateModel();
    }

    bool checkIntersection(glm::vec3 normalized_click_ray,
                           glm::vec3 ray_origin) {
        updateModel();
        glm::vec3 mesh_center_world = mesh->GetWorldCenter(ModelMatrix);

        return mesh->DoesHit(
            normalized_click_ray, // ray from near plane at click point
            ray_origin,           // camera position is ray origin
            ModelMatrix,          // model matrix
            glm::length(scale) /
                2, // use scale length for sphere scaling -- divided by 2 to
                   // form a radius, not diameter
            mesh_center_world // mesh center relative to world
        );
    }
};

std::ostream &operator<<(std::ostream &os, const SceneObject &so) {
    os << "Scene Object:\n";
    os << "\tTranslation: " << glm::to_string(so.translation) << "\n";
    os << "\tColor: " << glm::to_string(so.color) << "\n";
    os << "\tScale: " << glm::to_string(so.scale) << "\n";
    os << "\tModel: " << glm::to_string(so.ModelMatrix) << "\n";
    return os;
}