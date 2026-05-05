#pragma once

#include <vector>
#include <glm/glm.hpp>

glm::mat3 RotationMatrix(float yaw);

template <typename T>
T Interpolate(T a, T b, float t) { return a + (b - a) * t; }
template <typename T>
std::vector<T> Interpolate(T a, T b, int size) {
    std::vector<T> points(size);
    if (size == 1) points[0] = Interpolate(a, b, 0.5f);
    else for (size_t i = 0; i < size; i++) points[i] = Interpolate(a, b, float(i) / (size - 1));
    return points;
}

std::vector<glm::ivec2> Interpolate(glm::ivec2 a, glm::ivec2 b);
