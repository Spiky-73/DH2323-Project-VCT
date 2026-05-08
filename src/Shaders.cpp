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
    auto position = (vertex.position - bboxMin) / (bboxMax - bboxMin) * glm::vec3(resolution);
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

void VoxelizationShader::FragmentShader(const Pixel& pixel) {
    glm::ivec3 position;
    switch (axis) {
    case 0:
        position = glm::ivec3(pixel.z, pixel.x, int(pixel.y));
        break;
    case 1:
        position = glm::ivec3(pixel.y, pixel.z, int(pixel.x));
        break;
    default:
        position = glm::ivec3(pixel.x, pixel.y, int(pixel.z));
        break;
    }
    if (position.x < 0 || position.x >= resolution.x) return;
    if (position.y < 0 || position.y >= resolution.y) return;
    if (position.z < 0 || position.z >= resolution.z) return;
    if (isVoxel[position.x + resolution.x * position.y + resolution.x * resolution.y * position.z]) return;
    isVoxel[position.x + resolution.x * position.y + resolution.x * resolution.y * position.z] = true;
    auto visibility = shadowCube->Visibility(pixel.pos3d);
    voxels[position.x + resolution.x * position.y + resolution.x * resolution.y * position.z] = reflectance * visibility;
}

// Pixel ShadowShader::VertexShader(const Vertex& vertex) {
//     Pixel p;
//     auto position = (vertex.position - bboxMin) / (bboxMax - bboxMin) * glm::vec3(resolution);
//     switch (axis) {
//     case 0:
//         p.x = position.y;
//         p.y = position.z;
//         p.z = position.x;
//         break;
//     case 1:
//         p.x = position.z;
//         p.y = position.x;
//         p.z = position.y;
//         break;
//     default:
//         p.x = position.x;
//         p.y = position.y;
//         p.z = position.z;
//         break;
//     }
//     p.pos3d = vertex.position;
//     return p;
// }

// void ShadowShader::FragmentShader(const Pixel& pixel) {
//     glm::ivec3 position;
//     switch (axis) {
//     case 0:
//         position = glm::ivec3(pixel.z, pixel.x, int(pixel.y));
//         break;
//     case 1:
//         position = glm::ivec3(pixel.y, pixel.z, int(pixel.x));
//         break;
//     default:
//         position = glm::ivec3(pixel.x, pixel.y, int(pixel.z));
//         break;
//     }
//     if (position.x < 0 || position.x >= resolution.x) return;
//     if (position.y < 0 || position.y >= resolution.y) return;
//     if (position.z < 0 || position.z >= resolution.z) return;

//     // Light has already been computer for that voxel
//     if (shadowMap[position.x + resolution.x * position.y + resolution.x * resolution.y * position.z] != 0) return;

//     float v = Visibility(pixel.pos3d, light, bboxMin, bboxMax, resolution, isVoxel);
//     shadowMap[position.x + resolution.x * position.y + resolution.x * resolution.y * position.z] = v;
//     if (v == 0) {
//         voxels[position.x + resolution.x * position.y + resolution.x * resolution.y * position.z] = glm::vec3();
//     }
//     else {
//         auto r = light->position - pixel.pos3d;
//         auto directLight = light->intensity * light->color * v * glm::max(glm::dot(glm::normalize(r), normal), 0.f) / (4 * glm::pi<float>() * glm::length2(r));
//         voxels[position.x + resolution.x * position.y + resolution.x * resolution.y * position.z] *= directLight;
//     }
// }

// float ShadowShader::Visibility(glm::vec3 position, PointLight* light, glm::vec3 bboxMin, glm::vec3 bboxMax, glm::ivec3 resolution, bool* isVoxel) {
//     auto start = (position - bboxMin) / (bboxMax - bboxMin) * glm::vec3(resolution);
//     auto target = (light->position - bboxMin) / (bboxMax - bboxMin) * glm::vec3(resolution);
//     auto delta = target - start;
//     int pixels = glm::max(glm::abs(delta.x), glm::max(glm::abs(delta.y), glm::abs(delta.z)));
//     auto intersections = Interpolate(start, target, pixels);

//     for (size_t i = 1; i < pixels; i++) {
//         auto p = intersections[i];
//         if (p.x < 0 || p.x >= resolution.x) return 1;
//         if (p.y < 0 || p.y >= resolution.y) return 1;
//         if (p.z < 0 || p.z >= resolution.z) return 1;
//         if (isVoxel[int(p.x) + resolution.x * int(p.y) + resolution.x * resolution.y * int(p.z)]) return 0;
//     }
//     return 1;
// }


Pixel ShadowShader::VertexShader(const Vertex& vertex) {
    Pixel p;
    auto u = camera->rotation * (vertex.position - camera->position);
    p.z = 1 / u.z;
    p.x = camera->focale * u.x * p.z + camera->resolution.x / 2;
    p.y = camera->focale * u.y * p.z + camera->resolution.y / 2;
    p.pos3d = vertex.position * p.z;
    return p;
}

void ShadowShader::FragmentShader(const Pixel& pixel) {
    if (pixel.x < 0 || pixel.x >= camera->resolution.x) return;
    if (pixel.y < 0 || pixel.y >= camera->resolution.y) return;
    if (pixel.z < depthBuffer[pixel.x + camera->resolution.x * pixel.y]) return;
    depthBuffer[pixel.x + camera->resolution.x * pixel.y] = pixel.z;
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
    screen->putPixel(pixel.x, pixel.y, color * reflectance);
}

glm::vec3 RenderShader::DirectLight(const Pixel& pixel)
{
    glm::ivec3 voxelPos((pixel.pos3d / pixel.z - bboxMin) / (bboxMax - bboxMin) * glm::vec3(voxelResolution));
    // float visibility = shadowMap[voxelPos.x + voxelResolution.x * voxelPos.y + voxelResolution.x * voxelResolution.y * voxelPos.z];
    // float visibility = ShadowShader::Visibility(pixel.pos3d / pixel.z, light, bboxMin, bboxMax, voxelResolution, isVoxel); // More accurate
    auto visibility = shadowCube->Visibility(pixel.pos3d);
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
    glm::vec3 color;
    float ambiantOcclusion = 0;
    for (size_t i = 0; i < NUM_CONES; i++) {
        float occlusion;
        color += coneWeights[i] * ConeTrace(pixel.pos3d, normal * coneDirections[i], glm::pi<float>() / 3, &occlusion, ((bboxMax - bboxMin) / glm::vec3(voxelResolution)).x);
        ambiantOcclusion += coneWeights[i] * occlusion;
    }
    return color * ambiantOcclusion;
}

glm::vec3 RenderShader::ConeTrace(glm::vec3 origin, glm::vec3 direction, float angle, float* occlusion, float startDist) {
    glm::vec3 color;
    *occlusion = 0;
    float dist = startDist;
    while (*occlusion < 0.95f) {
        auto pos = origin + direction * dist;
        dist += 2 * glm::tan(angle / 2) * dist;
        float lod = glm::log2(dist);
        auto voxelColor = SampleVoxels(pos, lod);
        color += glm::vec3(voxelColor) * (1-*occlusion);
        *occlusion += (1 - *occlusion) * voxelColor.a;
    }

    return color;
}

glm::vec4 RenderShader::SampleVoxels(glm::vec3 position, float lod) {
    auto voxelSize = (bboxMax - bboxMin) / glm::vec3(voxelResolution);
    glm::ivec3 voxel((position - bboxMin) / voxelSize);
    if (voxel.x < 0 || voxel.x >= voxelResolution.x) return glm::vec4(0, 0, 0, 1);
    if (voxel.y < 0 || voxel.y >= voxelResolution.y) return glm::vec4(0, 0, 0, 1);
    if (voxel.z < 0 || voxel.z >= voxelResolution.z) return glm::vec4(0, 0, 0, 1);
    if (!isVoxel[voxel.x + voxelResolution.x * voxel.y + voxelResolution.x * voxelResolution.y * voxel.z]) return glm::vec4(0, 0, 0, 0);
    return glm::vec4(voxels[voxel.x + voxelResolution.x * voxel.y + voxelResolution.x * voxelResolution.y * voxel.z], 1);
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
