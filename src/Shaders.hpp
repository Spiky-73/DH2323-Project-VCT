#pragma once

#include <glm/glm.hpp>
#include "SDL2Auxiliary.h"
#include "Renderer.hpp"
#include "Objects.hpp"

class Lab3Shader : public Shader {
public:
    virtual Pixel VertexShader(const Vertex& vertex) final override;
    virtual void FragmentShader(const Pixel& pixel) final override;

public:
    glm::vec3 normal;
    glm::vec3 reflectance;

    Camera* camera;
    Light* light;
    Light* indirectLight;

    SDL2Aux* screen;
    glm::ivec2 resolution;
    float* depthBuffer;
};