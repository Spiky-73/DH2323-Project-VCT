#include <iostream>
#include <glm/gtx/constants.hpp>

#include "TestModel.hpp"
#include "Objects.hpp"
#include "Renderer.hpp"
#include "utility.hpp"
#include "Shaders.hpp"


enum class RenderMode {
	Lab3,
	Voxels,
	DirectLight,
	InDirectLight,
	VCT
};

// ----------------------------------------------------------------------------
// RENDERING
SDL2Aux* sdlAux;
Renderer renderer;
const int RESOLUTION_X = 500, RESOLUTION_Y = 500;
float depthBuffer[RESOLUTION_Y * RESOLUTION_X];
int t;
DrawMode mode = DrawMode::Default;


// ----------------------------------------------------------------------------
// SCENE
std::vector<Triangle> triangles;
Camera camera{ glm::vec3(0,0,-3.001), glm::mat3(1, 0, 0, 0, 1, 0, 0, 0, 1), glm::ivec2(RESOLUTION_X, RESOLUTION_Y), RESOLUTION_Y };
PointLight light{ glm::vec3(0, -0.5, -0.7), glm::vec3(1,1,1), 14 };
glm::vec3 bboxMin(-1.05, -1.05, -1.05);
glm::vec3 bboxMax(1.05, 1.05, 1.05);

// ----------------------------------------------------------------------------
// CAMERA MOVEMENT
float camTrSpeed = 2.f / 1000;
float yaw;
float camYawSpeed = glm::pi<float>() / 2 / 1000;

// ----------------------------------------------------------------------------
// SHADERS
RenderMode renderMode = RenderMode::Lab3;

Lab3Shader lab3Shader;
ShadowShader shadowShader;
VoxelizationShader voxelizationShader;
// ShadowShader shadowShader;
VoxelRenderShader voxelRenderShader;
RenderShader renderShader;

PointLight ambiantLight{ glm::vec3(), glm::vec3(1, 1, 1), 0.5f };
const int VOXEL_RESOLUTION = 32;
glm::vec3 voxelColors[VOXEL_RESOLUTION * VOXEL_RESOLUTION * VOXEL_RESOLUTION];
bool hasColor[VOXEL_RESOLUTION * VOXEL_RESOLUTION * VOXEL_RESOLUTION];
float shadowMap[VOXEL_RESOLUTION * VOXEL_RESOLUTION * VOXEL_RESOLUTION]; // TODO replace by a real shadow map
VoxelGrid voxels{ bboxMin, glm::ivec3(VOXEL_RESOLUTION, VOXEL_RESOLUTION, VOXEL_RESOLUTION), (bboxMax-bboxMin).x / VOXEL_RESOLUTION, hasColor, voxelColors };

void SetupShaders() {
	lab3Shader.camera = &camera;
	lab3Shader.light = &light;
	lab3Shader.indirectLight = &ambiantLight;
	lab3Shader.screen = sdlAux;
	lab3Shader.depthBuffer = depthBuffer;

	voxelizationShader.voxels = &voxels;

	shadowShader.voxels = &voxels;
	shadowShader.shadowMap = shadowMap;
	shadowShader.light = &light;
	voxelRenderShader.camera = &camera;
	voxelRenderShader.screen = sdlAux;
	voxelRenderShader.depthBuffer = depthBuffer;

	renderShader.camera = &camera;
	renderShader.light = &light;
	renderShader.voxels = &voxels;
	renderShader.shadowMap = shadowMap;
	renderShader.screen = sdlAux;
	renderShader.depthBuffer = depthBuffer;
	renderShader.lightingMode = renderMode == RenderMode::VCT ? LightingMode::All :
		renderMode == RenderMode::DirectLight ? LightingMode::Direct :
		LightingMode::Indirect;
	renderShader.coneAngleRatio = 1;
	renderShader.useReflectance = true;
}

void Update(void) {
	// Compute frame time:
	int t2 = SDL_GetTicks();
	float dt = float(t2 - t);
	t = t2;
	std::cout << "Render time: " << dt << " ms." << std::endl;

	const Uint8* keystate = SDL_GetKeyboardState(NULL);
	if (keystate[SDL_SCANCODE_UP]) {
		// Move camera forward
		camera.position.z += camTrSpeed * dt;
	}
	if (keystate[SDL_SCANCODE_DOWN]) {
		// Move camera backward
		camera.position.z -= camTrSpeed * dt;
	}
	if (keystate[SDL_SCANCODE_LEFT]) {
		// Move camera to the left
		yaw += camYawSpeed * dt;
		camera.rotation = RotationMatrix(yaw);
	}
	if (keystate[SDL_SCANCODE_RIGHT]) {
		// Move camera to the right
		yaw -= camYawSpeed * dt;
		camera.rotation = RotationMatrix(yaw);
	}
	if (keystate[SDL_SCANCODE_W]) light.position += glm::vec3(0, 0, camTrSpeed) * dt;
	if (keystate[SDL_SCANCODE_S]) light.position -= glm::vec3(0, 0, camTrSpeed) * dt;
	if (keystate[SDL_SCANCODE_A]) light.position -= glm::vec3(camTrSpeed, 0, 0) * dt;
	if (keystate[SDL_SCANCODE_D]) light.position += glm::vec3(camTrSpeed, 0, 0) * dt;
	if (keystate[SDL_SCANCODE_Q]) light.position -= glm::vec3(0, camTrSpeed, 0) * dt;
	if (keystate[SDL_SCANCODE_E]) light.position += glm::vec3(0, camTrSpeed, 0) * dt;

	if (keystate[SDL_SCANCODE_1]) renderMode = RenderMode::Lab3;
	if (keystate[SDL_SCANCODE_2]) renderMode = RenderMode::Voxels;
	if (keystate[SDL_SCANCODE_3]) {
		renderMode = RenderMode::DirectLight;
		renderShader.lightingMode = LightingMode::Direct;
		renderShader.useReflectance = true;
	}
	if (keystate[SDL_SCANCODE_4]) {
		renderMode = RenderMode::InDirectLight;
		renderShader.lightingMode = LightingMode::Indirect;
		renderShader.coneAngleRatio = 0.5f;
		renderShader.useReflectance = false;
	}
	if (keystate[SDL_SCANCODE_5]) {
		renderMode = RenderMode::VCT;
		renderShader.lightingMode = LightingMode::All;
		renderShader.coneAngleRatio = 0.5f;
		renderShader.useReflectance = true;
	}
	if (keystate[SDL_SCANCODE_6]) {
		renderMode = RenderMode::InDirectLight;
		renderShader.lightingMode = LightingMode::Indirect;
		renderShader.coneAngleRatio = 1;
		renderShader.useReflectance = false;
	}
	if (keystate[SDL_SCANCODE_7]) {
		renderMode = RenderMode::VCT;
		renderShader.lightingMode = LightingMode::All;
		renderShader.coneAngleRatio = 1;
		renderShader.useReflectance = true;
	}

	if (keystate[SDL_SCANCODE_8]) mode = DrawMode::Default;
	if (keystate[SDL_SCANCODE_9]) mode = DrawMode::Wireframe;
	if (keystate[SDL_SCANCODE_0]) mode = DrawMode::Vertices;
}

void DrawVoxels() {
	glm::vec3 vs = (bboxMax - bboxMin) / float(VOXEL_RESOLUTION);
	for (size_t x = 0; x < VOXEL_RESOLUTION; x++) {
		for (size_t y = 0; y < VOXEL_RESOLUTION; y++) {
			for (size_t z = 0; z < VOXEL_RESOLUTION; z++) {
				if (hasColor[x + VOXEL_RESOLUTION * y + VOXEL_RESOLUTION * VOXEL_RESOLUTION * z]) {
					glm::vec3 anchor = bboxMin + vs * glm::vec3(x, y, z);
					voxelRenderShader.reflectance = voxelColors[x + VOXEL_RESOLUTION * y + VOXEL_RESOLUTION * VOXEL_RESOLUTION * z];
					renderer.DrawPolygon({ {anchor}, {anchor + glm::vec3(vs.x,0,0)}, {anchor + glm::vec3(vs.x, vs.y,0)}, {anchor + glm::vec3(0, vs.y,0)} }, voxelRenderShader, mode);
					renderer.DrawPolygon({ {anchor}, {anchor + glm::vec3(0,vs.y,0)}, {anchor + glm::vec3(0, vs.y,vs.z)}, {anchor + glm::vec3(0, 0,vs.z)} }, voxelRenderShader, mode);
					renderer.DrawPolygon({ {anchor}, {anchor + glm::vec3(0,0,vs.z)}, {anchor + glm::vec3(vs.x,0,vs.z)}, {anchor + glm::vec3(vs.x,0,0)} }, voxelRenderShader, mode);
					renderer.DrawPolygon({ {anchor + glm::vec3(0,0,vs.z)}, {anchor + glm::vec3(vs.x,0,vs.z)}, {anchor + glm::vec3(vs.x, vs.y,vs.z)}, {anchor + glm::vec3(0, vs.y,vs.z)} }, voxelRenderShader, mode);
					renderer.DrawPolygon({ {anchor + glm::vec3(vs.x,0,0)}, {anchor + glm::vec3(vs.x,vs.y,0)}, {anchor + glm::vec3(vs.x, vs.y,vs.z)}, {anchor + glm::vec3(vs.x, 0,vs.z)} }, voxelRenderShader, mode);
					renderer.DrawPolygon({ {anchor + glm::vec3(0,vs.y,0)}, {anchor + glm::vec3(0,vs.y,vs.z)}, {anchor + glm::vec3(vs.x,vs.y,vs.z)}, {anchor + glm::vec3(vs.x,vs.y,0)} }, voxelRenderShader, mode);
				}
			}
		}
	}
}

void Draw()
{

	sdlAux->clearPixels();
	for (size_t i = 0; i < RESOLUTION_X * RESOLUTION_Y; i++) depthBuffer[i] = 0;

	if (renderMode == RenderMode::Lab3) {
		for (const auto& t : triangles) {
			lab3Shader.normal = t.normal;
			lab3Shader.reflectance = t.color;
			renderer.DrawPolygon({ {t.v0}, {t.v1}, {t.v2} }, lab3Shader, mode);
		}
		sdlAux->render();
		return;
	}

	// 1. Update the shadow map
	// TODO

	// 2. Generated the voxels with the reflectance as the color by "rendering the scene" on an orthographic camera centered on one side of the grid
	if (renderMode != RenderMode::DirectLight) {
		voxels.Clear();
		for (const auto& t : triangles) {
			auto nx = glm::abs(t.normal.x);
			auto ny = glm::abs(t.normal.y);
			auto nz = glm::abs(t.normal.z);
			voxelizationShader.axis = (nx >= ny && nx >= nz) ? 0 : (ny >= nx && ny >= nz) ? 1 : 2;
			voxelizationShader.reflectance = t.color;
			renderer.DrawPolygon({ {t.v0}, {t.v1}, {t.v2} }, voxelizationShader, DrawMode::Default);
		}
	}

	// 2.5. Bake in the visibility by ray casting from each voxel to light
	// This is not part of the original algorithm as it uses a shadow map
	for (size_t i = 0; i < VOXEL_RESOLUTION * VOXEL_RESOLUTION * VOXEL_RESOLUTION; i++) shadowMap[i] = 0;
	for (const auto& t : triangles) {
		auto nx = glm::abs(t.normal.x);
		auto ny = glm::abs(t.normal.y);
		auto nz = glm::abs(t.normal.z);
		shadowShader.normal = t.normal;
		shadowShader.axis = (nx >= ny && nx >= nz) ? 0 : (ny >= nx && ny >= nz) ? 1 : 2;
		renderer.DrawPolygon({ {t.v0}, {t.v1}, {t.v2} }, shadowShader, DrawMode::Default);
	}

	if (renderMode == RenderMode::Voxels) {
		DrawVoxels();
		sdlAux->render();
		return;
	}

	// 3. Render the scene, using the voxel data to compute direct illumination and cone tracing to compute indirect illumination
	for (const auto& t : triangles) {
		renderShader.normal = t.normal;
		renderShader.reflectance = t.color;
		renderer.DrawPolygon({ {t.v0}, {t.v1}, {t.v2} }, renderShader, mode);
	}
	sdlAux->render();
}


int main(int argc, char* argv[]) {
	LoadTestModel(triangles);  // Load model
	sdlAux = new SDL2Aux(RESOLUTION_X, RESOLUTION_Y);
	t = SDL_GetTicks();	// Set start value for timer.

	SetupShaders();

	while (!sdlAux->quitEvent())
	{
		Update();
		Draw();
	}
	sdlAux->saveBMP("screenshot.bmp");
	return 0;
}