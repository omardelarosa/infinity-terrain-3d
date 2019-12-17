#pragma once

#include <SceneObject.h>
#include <iostream>
#include <vector>

class SceneObjectList {
  private:
    std::vector<SceneObject> model_objects;

  public:
    int size() { return model_objects.size(); }
    void push(SceneObject &scene_object) {
        model_objects.push_back(scene_object);
    }
    SceneObject *at(int idx) {
        if (idx >= size()) {
            std::cout << "Invalid SceneObject index: " << idx << std::endl;
            return nullptr;
        }
        return &model_objects[idx];
    }

    void eraseAt(int idx) {
        model_objects.erase(            //
            model_objects.begin() + idx //
        );                              //
    }

    void log() {
        for (SceneObject &so : model_objects) {
            std::cout << so << std::endl;
        }
    }
};