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
    float* depthBuffer; // 2D array
};

class VoxelizationShader : public Shader {
public:
    virtual Pixel VertexShader(const Vertex& vertex) final override;
    virtual void FragmentShader(const Pixel& pixel) final override;

public:
    int axis;
    glm::vec3 reflectance;
    
    glm::vec3 bboxMin;
    glm::vec3 bboxMax;
    glm::vec4* voxels; // 3D array
    glm::ivec3 resolution;
};

class ShadowShader : public Shader {
public:
    virtual Pixel VertexShader(const Vertex& vertex) final override;
    virtual void FragmentShader(const Pixel& pixel) final override;

    static float Visibility(glm::vec3 position, Light* light, glm::vec3 bboxMin, glm::vec3 bboxMax, glm::ivec3 resolution, glm::vec4* voxels);

public:
    int axis;
    glm::vec3 normal;

    glm::vec3 bboxMin;
    glm::vec3 bboxMax;
    glm::vec4* voxels; // 3D array
    glm::ivec3 resolution;
    float* shadowMap;
    Light* light;

};

class VoxelRenderShader : public Shader {
public:
    virtual Pixel VertexShader(const Vertex& vertex) final override;
    virtual void FragmentShader(const Pixel& pixel) final override;

public:
    glm::vec3 normal;
    glm::vec3 reflectance;

    Camera* camera;
    SDL2Aux* screen;
    glm::ivec2 resolution;
    float* depthBuffer; // 2D array
};


class RenderShader : public Shader {
public:
    virtual Pixel VertexShader(const Vertex& vertex) final override;
    virtual void FragmentShader(const Pixel& pixel) final override;

private:
    glm::vec3 DirectLight(const Pixel& pixel);
    glm::vec3 IndirectLight(const Pixel& pixel);

public:
    glm::vec3 normal;
    glm::vec3 reflectance;

    Camera* camera;
    Light* light;
    glm::vec3 bboxMin;
    glm::vec3 bboxMax;
    glm::vec4* voxels;
    glm::ivec3 voxelResolution;
    float* shadowMap;
    SDL2Aux* screen;
    glm::ivec2 resolution;
    float* depthBuffer; // 2D array
};