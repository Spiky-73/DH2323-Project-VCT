#pragma once

#include <glm/glm.hpp>

struct Camera {
    glm::vec3 position;
    glm::mat3 rotation;
    glm::ivec2 resolution;
    float focale;
};

struct PointLight {
    glm::vec3 position;
    glm::vec3 color;
    float intensity;
};