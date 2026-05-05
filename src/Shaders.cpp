#include "Shaders.hpp"

#include <glm/gtx/norm.hpp>
#include <glm/gtx/constants.hpp>

Pixel Lab3Shader::VertexShader(const Vertex& vertex) {
    Pixel p;
    auto u = camera->rotation * (vertex.position - glm::transpose(camera->rotation) * camera->position);
    p.zinv = 1 / u.z;
    p.x = camera->focale * u.x * p.zinv + resolution.x / 2;
    p.y = camera->focale * u.y * p.zinv + resolution.y / 2;
    p.pos3d = vertex.position * p.zinv;
    return p;
}

void Lab3Shader::FragmentShader(const Pixel& pixel) {
    if (pixel.x < 0 || pixel.x >= resolution.x) return;
    if (pixel.y < 0 || pixel.y >= resolution.y) return;
    if (pixel.zinv < depthBuffer[pixel.x + resolution.x * pixel.y]) return;
    depthBuffer[pixel.x + resolution.x * pixel.y] = pixel.zinv;

    auto r = light->position - pixel.pos3d / pixel.zinv;
    auto directLight = light->color * light->intensity * glm::max(glm::dot(glm::normalize(r), normal), 0.f) / (4 * glm::pi<float>() * glm::length2(r));
    auto illumination = reflectance * (indirectLight->color * indirectLight->intensity + directLight);
    screen->putPixel(pixel.x, pixel.y, illumination);
}
