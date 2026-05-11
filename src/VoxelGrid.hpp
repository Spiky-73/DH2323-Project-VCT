#pragma once

#include <glm/glm.hpp>

class VoxelGrid {
public:
    glm::vec3 bboxMin;
    glm::ivec3 size;
    float voxelSize;

    bool* hasColor;
    glm::vec3* voxelColors;

    glm::ivec3 VoxelPosition(glm::vec3 worldPosition);

    bool IsVoxel(glm::ivec3 voxel);
    bool HasColor(glm::ivec3 voxel);

    void SetColor(glm::ivec3 voxel, glm::vec3 color);
    glm::vec3 Color(glm::ivec3 voxel);

    int VoxelIndex(glm::ivec3 voxel);

    void Clear();
};