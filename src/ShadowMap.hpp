#pragma once

#include "Objects.hpp"
#include <glm/gtx/rotate_vector.hpp>


struct ShadowCube {
    static const int FACES = 6;
    static glm::mat3 rotations[FACES];

    glm::vec3* position;
    float* depthMaps[FACES]; // concatenated arrays of 6 faces
    int resolution;

    float Visibility(glm::vec3 position);
};