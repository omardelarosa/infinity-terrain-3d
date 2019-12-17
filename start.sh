#!/bin/bash

# Starting rasterizer with sample shaders
./build/infinityterrain_bin \
    -f shaders/fragment_shader.glsl \
    -v shaders/vertex_shader.glsl \
    -g shaders/geometry_shader.glsl \
    -v2 shaders/passthrough_vertex_shader.glsl \
    -m1 assets/unitcube.off \
    -m2 assets/robot.obj \
    -m3 assets/unitcube.off \
    -f2 shaders/pixelated_fragment_shader.glsl \
    # -f2 shaders/wobbly_fragment_shader.glsl