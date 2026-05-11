#include "utility.hpp"

glm::mat3 RotationMatrix(float yaw) {
    return glm::mat3(
        glm::cos(yaw), 0, -glm::sin(yaw),
        0, 1, 0,
        glm::sin(yaw), 0, glm::cos(yaw)
    );
}

std::vector<glm::ivec2> Interpolate(glm::ivec2 a, glm::ivec2 b, int size) {
    std::vector<glm::ivec2> points(size);
    auto step = glm::vec2(b.x - a.x, b.y - a.y) / float(glm::max(size - 1, 1));
    glm::vec2 current(a);
    for (int i = 0; i < size; i++) {
        points[i] = current;
        current += step;
    }
    return points;
}
std::vector<glm::ivec3> Interpolate(glm::ivec3 a, glm::ivec3 b, int size) {
    std::vector<glm::ivec3> points(size);
    auto step = glm::vec3(b.x - a.x, b.y - a.y, b.z - a.z) / float(glm::max(size - 1, 1));
    glm::vec3 current(a);
    for (int i = 0; i < size; i++) {
        points[i] = current;
        current += step;
    }
    return points;
}
