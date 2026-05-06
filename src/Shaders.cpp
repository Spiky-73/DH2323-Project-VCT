#include "Shaders.hpp"

#include <glm/gtx/norm.hpp>
#include <glm/gtx/constants.hpp>

#include "utility.hpp"

Pixel Lab3Shader::VertexShader(const Vertex& vertex) {
    Pixel p;
    auto u = camera->rotation * (vertex.position - glm::transpose(camera->rotation) * camera->position);
    p.z = 1 / u.z;
    p.x = camera->focale * u.x * p.z + resolution.x / 2;
    p.y = camera->focale * u.y * p.z + resolution.y / 2;
    p.pos3d = vertex.position * p.z;
    return p;
}

void Lab3Shader::FragmentShader(const Pixel& pixel) {
    if (pixel.x < 0 || pixel.x >= resolution.x) return;
    if (pixel.y < 0 || pixel.y >= resolution.y) return;
    if (pixel.z < depthBuffer[pixel.x + resolution.x * pixel.y]) return;
    depthBuffer[pixel.x + resolution.x * pixel.y] = pixel.z;

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
    voxels[position.x + resolution.x * position.y + resolution.x * resolution.y * position.z] = glm::vec4(reflectance, 1); // TODO * visibility
}

Pixel ShadowShader::VertexShader(const Vertex& vertex) {
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

void ShadowShader::FragmentShader(const Pixel& pixel) {
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

    // Light has already been computer for that voxel
    if (shadowMap[position.x + resolution.x * position.y + resolution.x * resolution.y * position.z] != 0) return;

    float v = Visibility(pixel.pos3d);
    shadowMap[position.x + resolution.x * position.y + resolution.x * resolution.y * position.z] = v;
    if (v == 0) {
        voxels[position.x + resolution.x * position.y + resolution.x * resolution.y * position.z] *= glm::vec4(0, 0, 0, 1);
    }
    else {
        auto r = light->position - pixel.pos3d;
        auto directLight = light->intensity * light->color * v * glm::max(glm::dot(glm::normalize(r), normal), 0.f) / (4 * glm::pi<float>() * glm::length2(r));
        voxels[position.x + resolution.x * position.y + resolution.x * resolution.y * position.z] *= glm::vec4(directLight, 1);
    }
}

float ShadowShader::Visibility(glm::vec3 position) {
    auto start = (position - bboxMin) / (bboxMax - bboxMin) * glm::vec3(resolution);
    auto target = (light->position - bboxMin) / (bboxMax - bboxMin) * glm::vec3(resolution);
    auto delta = target - start;
    int pixels = glm::max(glm::abs(delta.x), glm::max(glm::abs(delta.y), glm::abs(delta.z)));
    auto intersections = Interpolate(start, target, pixels);

    float v = 1;
    for (size_t i = 1; i < pixels; i++) {
        auto p = intersections[i];
        if (p.x < 0 || p.x >= resolution.x) return v;
        if (p.y < 0 || p.y >= resolution.y) return v;
        if (p.z < 0 || p.z >= resolution.z) return v;
        v *= 1 - voxels[int(p.x) + resolution.x * int(p.y) + resolution.x * resolution.y * int(p.z)].a;
        if (v <= 0.001) return 0;
    }
    return v;
}

Pixel VoxelRenderShader::VertexShader(const Vertex& vertex) {
    Pixel p;
    auto u = camera->rotation * (vertex.position - glm::transpose(camera->rotation) * camera->position);
    p.z = 1 / u.z;
    p.x = camera->focale * u.x * p.z + resolution.x / 2;
    p.y = camera->focale * u.y * p.z + resolution.y / 2;
    p.pos3d = vertex.position * p.z;
    return p;
}

void VoxelRenderShader::FragmentShader(const Pixel& pixel) {
    if (pixel.x < 0 || pixel.x >= resolution.x) return;
    if (pixel.y < 0 || pixel.y >= resolution.y) return;
    if (pixel.z < depthBuffer[pixel.x + resolution.x * pixel.y]) return;
    depthBuffer[pixel.x + resolution.x * pixel.y] = pixel.z;
    screen->putPixel(pixel.x, pixel.y, reflectance);
}

Pixel RenderShader::VertexShader(const Vertex& vertex) {
    Pixel p;
    auto u = camera->rotation * (vertex.position - glm::transpose(camera->rotation) * camera->position);
    p.z = 1 / u.z;
    p.x = camera->focale * u.x * p.z + resolution.x / 2;
    p.y = camera->focale * u.y * p.z + resolution.y / 2;
    p.pos3d = vertex.position * p.z;
    return p;
}

void RenderShader::FragmentShader(const Pixel& pixel) {
    if (pixel.x < 0 || pixel.x >= resolution.x) return;
    if (pixel.y < 0 || pixel.y >= resolution.y) return;
    if (pixel.z < depthBuffer[pixel.x + resolution.x * pixel.y]) return;
    depthBuffer[pixel.x + resolution.x * pixel.y] = pixel.z;

    auto directLight = DirectLight(pixel);
    auto indirectLight = IndirectLight(pixel);

    auto color = (directLight + indirectLight) * reflectance;
    screen->putPixel(pixel.x, pixel.y, color);
}

glm::vec3 RenderShader::DirectLight(const Pixel& pixel)
{
    glm::ivec3 voxelPos((pixel.pos3d / pixel.z - bboxMin) / (bboxMax - bboxMin) * glm::vec3(voxelResolution));
    float visibility = shadowMap[voxelPos.x + voxelResolution.x * voxelPos.y + voxelResolution.x * voxelResolution.y * voxelPos.z];
    if (visibility == 0) return glm::vec3();

    auto r = light->position - pixel.pos3d / pixel.z;
    return light->intensity * light->color * visibility* glm::max(glm::dot(glm::normalize(r), normal), 0.f) / (4 * glm::pi<float>() * glm::length2(r));
}

glm::vec3 RenderShader::IndirectLight(const Pixel& pixel)
{
    return glm::vec3(0.5,0.5,0.5);
}
