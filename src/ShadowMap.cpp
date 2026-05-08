#include "ShadowMap.hpp"

glm::mat3 ShadowCube::rotations[6] = {
    glm::mat3(glm::lookAt(glm::vec3(), glm::vec3(1, 0, 0), glm::vec3(0, 1, 0))), // +X
    glm::mat3(glm::lookAt(glm::vec3(), glm::vec3(0, 1, 0), glm::vec3(0, 0, 1))), // +Y // Y would be parallel to Up
    glm::mat3(glm::lookAt(glm::vec3(), glm::vec3(0, 0, 1), glm::vec3(0, 1, 0))), // +Z
    glm::mat3(glm::lookAt(glm::vec3(), glm::vec3(-1, 0, 0), glm::vec3(0, 1, 0))), // -X
    glm::mat3(glm::lookAt(glm::vec3(), glm::vec3(0, -1, 0), glm::vec3(0, 0, 1))), // -Y
    glm::mat3(glm::lookAt(glm::vec3(), glm::vec3(0, 0, -1), glm::vec3(0, 1, 0))), // -Z
};

float ShadowCube::Visibility(glm::vec3 position)
{
    return 0.0f;
}
