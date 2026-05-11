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
    PointLight* light;
    PointLight* indirectLight;
    SDL2Aux* screen;
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
    bool* isVoxel; // 3D array
    glm::vec3* voxels; // 3D array
    glm::ivec3 resolution;
};

class ShadowShader : public Shader {
public:
    virtual Pixel VertexShader(const Vertex& vertex) final override;
    virtual void FragmentShader(const Pixel& pixel) final override;

    static float Visibility(glm::vec3 position, PointLight* light, glm::vec3 bboxMin, glm::vec3 bboxMax, glm::ivec3 resolution, bool* isVoxel);

public:
    int axis;
    glm::vec3 normal;

    glm::vec3 bboxMin;
    glm::vec3 bboxMax;
    bool* isVoxel; // 3D array
    glm::vec3* voxels; // 3D array
    glm::ivec3 resolution;
    float* shadowMap;
    PointLight* light;
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
    float* depthBuffer; // 2D array
};

enum class LightingMode {
    None,
    Direct = 0b01,
    Indirect = 0b10,
    All = 0b11,
};

LightingMode operator&(LightingMode lhs, LightingMode rhs);
LightingMode operator|(LightingMode lhs, LightingMode rhs);

class RenderShader : public Shader {
public:
    virtual Pixel VertexShader(const Vertex& vertex) final override;
    virtual void FragmentShader(const Pixel& pixel) final override;

private:
    glm::vec3 DirectLight(const Pixel& pixel);
    glm::vec3 IndirectLight(const Pixel& pixel);

    glm::vec3 ConeTrace(glm::vec3 origin, glm::vec3 direction, float angle, float* occlusion, float startDist = 0);
    glm::vec4 SampleVoxels(glm::vec3 position, float lod);

public:
    glm::vec3 normal;
    glm::vec3 reflectance;

    Camera* camera;
    PointLight* light;
    glm::vec3 bboxMin;
    glm::vec3 bboxMax;
    bool* isVoxel;
    float* shadowMap;
    glm::vec3* voxels;
    glm::ivec3 voxelResolution;
    SDL2Aux* screen;
    float* depthBuffer; // 2D array

    LightingMode lightingMode;
};