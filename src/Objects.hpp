#pragma once

#include <glm/glm.hpp>

struct Camera {
    glm::vec3 position;
    glm::mat3 rotation;
    float focale;
};


struct Light {
    glm::vec3 position;
    glm::vec3 color;
    float intensity;
};