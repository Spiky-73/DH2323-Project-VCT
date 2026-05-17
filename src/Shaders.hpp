#pragma once

#include <glm/glm.hpp>
#include "SDL2Auxiliary.h"
#include "Renderer.hpp"
#include "Objects.hpp"
#include "VoxelGrid.hpp"

class Lab3Shader : public Shader {
public:
    virtual Pixel VertexShader(const Vertex& vertex) final override;
    virtual void FragmentShader(const Pixel& pixel) final override;

public:
    // Per Triangle
    glm::vec3 normal;
    glm::vec3 reflectance;
    
    // Global
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
    // Per Triangle
    int axis;
    glm::vec3 reflectance;

    // Global
    VoxelGrid* voxels;
};

class ShadowShader : public Shader {
public:
    virtual Pixel VertexShader(const Vertex& vertex) final override;
    virtual void FragmentShader(const Pixel& pixel) final override;

    static float Visibility(glm::vec3 position, PointLight* light, VoxelGrid* voxels);

public:
    // Per Triangle
    int axis;
    glm::vec3 normal;

    // Global
    VoxelGrid* voxels;
    float* shadowMap;
    PointLight* light;
};

class VoxelRenderShader : public Shader {
public:
    virtual Pixel VertexShader(const Vertex& vertex) final override;
    virtual void FragmentShader(const Pixel& pixel) final override;

public:
    // Per Triangle
    glm::vec3 normal;
    glm::vec3 reflectance;

    // Global
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

    static glm::vec3 ConeTrace(VoxelGrid* voxels, glm::vec3 origin, glm::vec3 direction, float angle, float* occlusion);
    static glm::vec4 SampleVoxels(VoxelGrid* voxels, glm::vec3 position, float lod);

public:
    // Per Triangle
    glm::vec3 normal;
    glm::vec3 reflectance;

    // Global
    Camera* camera;
    PointLight* light;
    VoxelGrid* voxels;
    float* shadowMap;
    SDL2Aux* screen;
    float* depthBuffer; // 2D array
    LightingMode lightingMode;
    float coneAngleRatio;
    bool useReflectance;
};