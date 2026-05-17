#include "Shaders.hpp"

#include <glm/gtx/norm.hpp>
#include <glm/gtx/constants.hpp>

#include "utility.hpp"

Pixel Lab3Shader::VertexShader(const Vertex& vertex) {
    Pixel p;
    auto u = camera->rotation * (vertex.position - glm::transpose(camera->rotation) * camera->position);
    p.z = 1 / u.z;
    p.x = camera->focale * u.x * p.z + camera->resolution.x / 2;
    p.y = camera->focale * u.y * p.z + camera->resolution.y / 2;
    p.pos3d = vertex.position * p.z;
    return p;
}

void Lab3Shader::FragmentShader(const Pixel& pixel) {
    if (pixel.x < 0 || pixel.x >= camera->resolution.x) return;
    if (pixel.y < 0 || pixel.y >= camera->resolution.y) return;
    if (pixel.z < depthBuffer[pixel.x + camera->resolution.x * pixel.y]) return;
    depthBuffer[pixel.x + camera->resolution.x * pixel.y] = pixel.z;

    auto r = light->position - pixel.pos3d / pixel.z;
    auto directLight = light->color * light->intensity * glm::max(glm::dot(glm::normalize(r), normal), 0.f) / (4 * glm::pi<float>() * glm::length2(r));
    auto illumination = reflectance * (indirectLight->color * indirectLight->intensity + directLight);
    screen->putPixel(pixel.x, pixel.y, illumination);
}

Pixel VoxelizationShader::VertexShader(const Vertex& vertex) {
    Pixel p;
    auto voxel = voxels->VoxelPosition(vertex.position);
    switch (axis) {
    case 0:
        p.x = voxel.y;
        p.y = voxel.z;
        p.z = voxel.x;
        break;
    case 1:
        p.x = voxel.z;
        p.y = voxel.x;
        p.z = voxel.y;
        break;
    default:
        p.x = voxel.x;
        p.y = voxel.y;
        p.z = voxel.z;
        break;
    }
    p.pos3d = vertex.position;
    return p;
}

void VoxelizationShader::FragmentShader(const Pixel& pixel) {
    glm::ivec3 voxel;
    switch (axis) {
    case 0:
        voxel = glm::ivec3(pixel.z, pixel.x, pixel.y);
        break;
    case 1:
        voxel = glm::ivec3(pixel.y, pixel.z, pixel.x);
        break;
    default:
        voxel = glm::ivec3(pixel.x, pixel.y, pixel.z);
        break;
    }
    if (!voxels->IsVoxel(voxel) || voxels->HasColor(voxel)) return;
    voxels->SetColor(voxel, reflectance);
}

Pixel ShadowShader::VertexShader(const Vertex& vertex) {
    Pixel p;
    auto position = voxels->VoxelPosition(vertex.position);
    switch (axis) {
    case 0:
        p.x = position.y;
        p.y = position.z;
        p.z = position.x;
        break;
    case 1:
        p.x = position.z;
        p.y = position.x;
        p.z = position.y;
        break;
    default:
        p.x = position.x;
        p.y = position.y;
        p.z = position.z;
        break;
    }
    p.pos3d = vertex.position;
    return p;
}

void ShadowShader::FragmentShader(const Pixel& pixel) {
    glm::ivec3 voxel;
    switch (axis) {
    case 0:
        voxel = glm::ivec3(pixel.z, pixel.x, int(pixel.y));
        break;
    case 1:
        voxel = glm::ivec3(pixel.y, pixel.z, int(pixel.x));
        break;
    default:
        voxel = glm::ivec3(pixel.x, pixel.y, int(pixel.z));
        break;
    }
    if (!voxels->IsVoxel(voxel)) return;

    auto index = voxels->VoxelIndex(voxel);
    // Light has already been computer for that voxel
    if (shadowMap[index] != 0) return;

    float v = Visibility(pixel.pos3d, light, voxels);
    shadowMap[index] = v;
    if (v == 0) {
        voxels->SetColor(voxel, glm::vec3());
    }
    else {
        auto r = light->position - pixel.pos3d;
        auto directLight = light->intensity * light->color * v * glm::max(glm::dot(glm::normalize(r), normal), 0.f) / (4 * glm::pi<float>() * glm::length2(r));
        voxels->SetColor(voxel, voxels->Color(voxel) * directLight);
    }
}

float ShadowShader::Visibility(glm::vec3 position, PointLight* light, VoxelGrid* voxels) {
    auto start = voxels->VoxelPosition(position);
    auto target = voxels->VoxelPosition(light->position);
    auto delta = target - start;
    int pixels = glm::max(glm::abs(delta.x), glm::max(glm::abs(delta.y), glm::abs(delta.z)));
    auto intersections = Interpolate(start, target, pixels);

    for (size_t i = 1; i < pixels; i++) {
        auto p = intersections[i];
        if (!voxels->IsVoxel(p)) return 1;
        if (voxels->HasColor(p)) return 0;
    }
    return 1;
}

Pixel VoxelRenderShader::VertexShader(const Vertex& vertex) {
    Pixel p;
    auto u = camera->rotation * (vertex.position - glm::transpose(camera->rotation) * camera->position);
    p.z = 1 / u.z;
    p.x = camera->focale * u.x * p.z + camera->resolution.x / 2;
    p.y = camera->focale * u.y * p.z + camera->resolution.y / 2;
    p.pos3d = vertex.position * p.z;
    return p;
}

void VoxelRenderShader::FragmentShader(const Pixel& pixel) {
    if (pixel.x < 0 || pixel.x >= camera->resolution.x) return;
    if (pixel.y < 0 || pixel.y >= camera->resolution.y) return;
    if (pixel.z < depthBuffer[pixel.x + camera->resolution.x * pixel.y]) return;
    depthBuffer[pixel.x + camera->resolution.x * pixel.y] = pixel.z;
    screen->putPixel(pixel.x, pixel.y, reflectance);
}

Pixel RenderShader::VertexShader(const Vertex& vertex) {
    Pixel p;
    auto u = camera->rotation * (vertex.position - glm::transpose(camera->rotation) * camera->position);
    p.z = 1 / u.z;
    p.x = camera->focale * u.x * p.z + camera->resolution.x / 2;
    p.y = camera->focale * u.y * p.z + camera->resolution.y / 2;
    p.pos3d = vertex.position * p.z;
    return p;
}

void RenderShader::FragmentShader(const Pixel& pixel) {
    if (pixel.x < 0 || pixel.x >= camera->resolution.x) return;
    if (pixel.y < 0 || pixel.y >= camera->resolution.y) return;
    if (pixel.z < depthBuffer[pixel.x + camera->resolution.x * pixel.y]) return;
    depthBuffer[pixel.x + camera->resolution.x * pixel.y] = pixel.z;

    glm::vec3 color;
    if (bool(lightingMode & LightingMode::Direct)) color += DirectLight(pixel);
    if (bool(lightingMode & LightingMode::Indirect)) color += IndirectLight(pixel);
    screen->putPixel(pixel.x, pixel.y, useReflectance ? color * reflectance : color);
}

glm::vec3 RenderShader::DirectLight(const Pixel& pixel)
{
    auto voxelPos = voxels->VoxelPosition(pixel.pos3d / pixel.z);
    // float visibility = shadowMap[voxels->VoxelIndex(voxelPos)];
    float visibility = ShadowShader::Visibility(pixel.pos3d / pixel.z, light, voxels); // More accurate
    if (visibility == 0) return glm::vec3();

    auto r = light->position - pixel.pos3d / pixel.z;
    return light->intensity * light->color * visibility * glm::max(glm::dot(glm::normalize(r), normal), 0.f) / (4 * glm::pi<float>() * glm::length2(r));
}


// 6 60 degree cone
const int NUM_CONES = 6;
glm::vec3 coneDirections[NUM_CONES] = {
    glm::vec3(0, 1, 0),
    glm::vec3(0, 0.5f, 0.866025f),
    glm::vec3(0.823639f, 0.5f, 0.267617f),
    glm::vec3(0.509037f, 0.5f, -0.700629f),
    glm::vec3(-0.509037f, 0.5f, -0.700629f),
    glm::vec3(-0.823639f, 0.5f, 0.267617f)
};
float coneWeights[NUM_CONES]{ 0.25f, 0.15f, 0.15f, 0.15f, 0.15f, 0.15f };

glm::vec3 RenderShader::IndirectLight(const Pixel& pixel) {
    glm::vec3 tangent = normal.y == 1 ? glm::vec3(1, 0, 0) : glm::vec3(0, 1, 0);
    glm::mat3 faceRotation = glm::mat3(tangent, normal, glm::cross(tangent, normal));
    float angle = glm::pi<float>() / 3 * coneAngleRatio;

    glm::vec3 color;
    float ambiantOcclusion = 0;
    for (size_t i = 0; i < NUM_CONES; i++) {
        float occlusion;
        color += coneWeights[i] * ConeTrace(voxels, pixel.pos3d / pixel.z, faceRotation * coneDirections[i], angle, &occlusion);
        ambiantOcclusion += coneWeights[i] * occlusion;
    }

    // size_t i = 0;
    // float occlusion;
    // auto color = ConeTrace(voxels, pixel.pos3d / pixel.z, faceRotation * coneDirections[i], angle, &occlusion);
    // auto ambiantOcclusion = occlusion;
    return color * ambiantOcclusion;
}

glm::vec3 RenderShader::ConeTrace(VoxelGrid* voxels, glm::vec3 origin, glm::vec3 direction, float angle, float* occlusion) {
    glm::vec3 color;
    *occlusion = 0;
    float dist = voxels->voxelSize;
    origin += direction * voxels->voxelSize;
    int steps = 0;
    float tan = glm::tan(angle / 2);
    while (*occlusion < 0.95f && steps < 4) {
        auto pos = origin + direction * dist;
        float lod = glm::log2(dist / voxels->voxelSize);
        dist += 2 * tan * dist;
        auto voxelColor = SampleVoxels(voxels, pos, lod);
        // float power = voxelColor.a / (2 * glm::pi<float>() * (dist * dist));
        color += glm::vec3(voxelColor) * (1 - *occlusion);
        *occlusion += (1 - *occlusion) * voxelColor.a;
        steps++;
    }
    return color;
}

glm::vec4 RenderShader::SampleVoxels(VoxelGrid* voxels, glm::vec3 position, float lod) {
    int size = std::pow(2, lod);

    glm::ivec3 orig;
    if (size == 1) orig = voxels->VoxelPosition(position);
    else orig = voxels->VoxelPosition(position - voxels->voxelSize * (size / 2.f));
    int sampledVoxels = 0;
    glm::vec3 color;
    for (size_t dx = 0; dx < size; dx++) {
        for (size_t dy = 0; dy < size; dy++) {
            for (size_t dz = 0; dz < size; dz++) {
                glm::ivec3 voxel = orig + glm::ivec3(dx, dy, dz);
                if (!voxels->IsVoxel(voxel) || !voxels->HasColor(voxel)) continue;;
                sampledVoxels++;
                color += voxels->Color(voxel);
            }
        }
    }
    if (sampledVoxels == 0) return glm::vec4();
    return glm::vec4(color / float(sampledVoxels), float(sampledVoxels) / (size * size * size));
}

LightingMode operator&(LightingMode lhs, LightingMode rhs) {
    return static_cast<LightingMode>(
        static_cast<std::underlying_type<LightingMode>::type>(lhs) &
        static_cast<std::underlying_type<LightingMode>::type>(rhs)
        );
}
LightingMode operator|(LightingMode lhs, LightingMode rhs) {
    return static_cast<LightingMode>(
        static_cast<std::underlying_type<LightingMode>::type>(lhs) |
        static_cast<std::underlying_type<LightingMode>::type>(rhs)
        );
}