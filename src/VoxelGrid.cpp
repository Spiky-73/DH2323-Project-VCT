#include "VoxelGrid.hpp"

glm::ivec3 VoxelGrid::VoxelPosition(glm::vec3 worldPosition) {
    return glm::ivec3((worldPosition - bboxMin) / voxelSize);
}

bool VoxelGrid::IsVoxel(glm::ivec3 voxel) {
    return 0 <= voxel.x && voxel.x < size.x
        && 0 <= voxel.y && voxel.y < size.y
        && 0 <= voxel.z && voxel.z < size.z;
}

bool VoxelGrid::HasColor(glm::ivec3 voxel) {
    return hasColor[VoxelIndex(voxel)];
}

void VoxelGrid::SetColor(glm::ivec3 voxel, glm::vec3 color) {
    auto index = VoxelIndex(voxel);
    hasColor[index] = true;
    voxelColors[index] = color;
}

glm::vec3 VoxelGrid::Color(glm::ivec3 voxel) {
    return voxelColors[VoxelIndex(voxel)];
}

int VoxelGrid::VoxelIndex(glm::ivec3 voxel) {
    return voxel.x + size.x * voxel.y + size.x * size.y * voxel.z;
}

void VoxelGrid::Clear() {
    for (size_t i = 0; i < size.x * size.y * size.z; i++) hasColor[i] = false;
}
