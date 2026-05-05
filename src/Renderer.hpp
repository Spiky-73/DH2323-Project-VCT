#pragma once

#include <glm/glm.hpp>
#include <utility>
#include <vector>


struct Vertex {
    glm::vec3 position;
};
struct Pixel {
    int x;
    int y;
    float zinv;
    glm::vec3 pos3d;
};

std::vector<Pixel> Interpolate(Pixel a, Pixel b, int size);


class Shader {
public:
    virtual Pixel VertexShader(const Vertex& v) = 0;
    virtual void FragmentShader(const Pixel& p) = 0;
};

enum class DrawMode {
    Default,
    Vertices,
    WireFrame,
};

class Renderer {
public:
    void DrawPolygon(const std::vector<Vertex>& vertices, Shader& shader, DrawMode mode = DrawMode::Default);
    void DrawLine(Vertex a, Vertex b, Shader& shader, DrawMode mode = DrawMode::Default);
    void DrawPoint(Vertex vertex, Shader& shader, DrawMode mode = DrawMode::Default);

private:
    std::vector<std::pair<Pixel, Pixel>> ComputePolygonRows(const std::vector<Pixel>& pixels);
    void DrawPolygon(const std::vector<Pixel>& pixels, Shader& shader);
    void DrawLine(Pixel a, Pixel b, Shader& shader);
    void DrawPoint(Pixel point, Shader& shader);
};