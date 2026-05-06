#include "Renderer.hpp"

#include "utility.hpp"

void Renderer::DrawPolygon(const std::vector<Vertex>& vertices, Shader& shader, DrawMode mode) {
    std::vector<Pixel> pixels(vertices.size());
    for (size_t i = 0; i < vertices.size(); i++) pixels[i] = shader.VertexShader(vertices[i]);

    switch (mode) {
    case DrawMode::Vertices:
        for (auto& p : pixels) DrawPoint(p, shader);
        break;
    case DrawMode::Wireframe:
        for (size_t i = 0; i < pixels.size(); i++) DrawLine(pixels[i], pixels[(i + 1) % pixels.size()], shader);
        break;
    default:
        DrawPolygon(pixels, shader);
        break;
    }
}

void Renderer::DrawLine(Vertex a, Vertex b, Shader& shader, DrawMode mode) {
    Pixel pa = shader.VertexShader(a);
    Pixel pb = shader.VertexShader(b);

    switch (mode) {
    case DrawMode::Vertices:
        DrawPoint(pa, shader);
        DrawPoint(pb, shader);
        break;
    default:
        DrawLine(pa, pb, shader);
        break;
    }
}

void Renderer::DrawPoint(Vertex vertex, Shader& shader, DrawMode mode) {
    Pixel pixel = shader.VertexShader(vertex);
    DrawPoint(pixel, shader);
}

std::vector<std::pair<Pixel, Pixel>> Renderer::ComputePolygonRows(const std::vector<Pixel>& pixels) {
    // 1. Find max and min y−value of the polygon and compute rows
    int yMax = std::numeric_limits<int>::min(), yMin = std::numeric_limits<int>::max();
    for (auto v : pixels) {
        yMax = glm::max(yMax, v.y);
        yMin = glm::min(yMin, v.y);
    }
    auto rows = yMax - yMin + 1;
    std::vector<std::pair<Pixel, Pixel>> rowPixels(rows);
    // 2. Resize leftPixels and rightPixels so that they have an element for each row
    rowPixels.resize(rows);

    // 3. Initialize the x-coordinates in leftPixels to +infinity and the x-coordinates in rightPixels to -infinity
    for (int i = 0; i < rows; i++) {
        rowPixels[i].first = { std::numeric_limits<int>::max() };
        rowPixels[i].second = { std::numeric_limits<int>::min() };
    }

    // 4 . Loop through every edge of polygon and use lerp to find the x-coordinates for each row
    for (size_t i = 0; i < pixels.size(); i++) {
        Pixel a = pixels[i];
        Pixel b = pixels[(i + 1) % pixels.size()];
        int pixels = glm::max(glm::abs(a.x - b.x), glm::abs(a.y - b.y)) + 1;
        auto line = Interpolate(a, b, pixels);
        for (auto p : line) {
            auto r = p.y - yMin;
            if (r < 0 || r >= rows) continue;
            if (p.x < rowPixels[r].first.x) rowPixels[r].first = p;
            if (p.x > rowPixels[r].second.x) rowPixels[r].second = p;
        }
    }
    return rowPixels;
}

void Renderer::DrawPolygon(const std::vector<Pixel>& pixels, Shader& shader) {
    auto rows = ComputePolygonRows(pixels);
    for (auto& r : rows) DrawLine(r.first, r.second, shader);
}

void Renderer::DrawLine(Pixel a, Pixel b, Shader& shader) {
    int pixels = glm::max(glm::abs(a.x - b.x), glm::abs(a.y - b.y)) + 1;
    auto line = Interpolate(a, b, pixels);
    for (auto p : line) DrawPoint(p, shader);
}

void Renderer::DrawPoint(Pixel point, Shader& shader) {
    shader.FragmentShader(point);
}

std::vector<Pixel> Interpolate(Pixel a, Pixel b, int size) {
    std::vector<Pixel> points(size);
    glm::vec3 position(a.x, a.y, a.z);
    auto positionStep = glm::vec3(b.x - a.x, b.y - a.y, b.z - a.z) / float(glm::max(size - 1, 1));
    glm::vec3 pos3d(a.pos3d);
    glm::vec3 pos3dStep = (b.pos3d - a.pos3d) / float(glm::max(size - 1, 1));
    for (int i = 0; i < size; i++) {
        points[i] = { int(position.x), int(position.y), position.z, pos3d };
        position += positionStep;
        pos3d += pos3dStep;
    }
    return points;
}
