#include <iostream>
#include <glm/gtx/constants.hpp>

#include "TestModel.hpp"
#include "Objects.hpp"
#include "Renderer.hpp"
#include "utility.hpp"
#include "Shaders.hpp"

// ----------------------------------------------------------------------------
// GLOBAL VARIABLES

const int SCREEN_WIDTH = 500;
const int SCREEN_HEIGHT = 500;
SDL2Aux* sdlAux;
int t;
std::vector<Triangle> triangles;


Camera camera{ glm::vec3(0,0,-3.001), glm::mat3(1, 0, 0, 0, 1, 0, 0, 0, 1), SCREEN_HEIGHT };
float camTrSpeed = 2.f / 1000;
float yaw;
float camYawSpeed = glm::pi<float>() / 2 / 1000;

float depthBuffer[SCREEN_HEIGHT * SCREEN_WIDTH];

Light light{ glm::vec3(0, -0.5, -0.7), glm::vec3(1,1,1), 14 };
Light indirectLight{ glm::vec3(), glm::vec3(1, 1, 1), 0.5f, };

Renderer renderer;
Lab3Shader shader;

void Update(void)
{
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
	if (keystate[SDL_SCANCODE_W]) {
		light.position += glm::vec3(0, 0, camTrSpeed) * dt;
	}
	if (keystate[SDL_SCANCODE_S]) {
		light.position -= glm::vec3(0, 0, camTrSpeed) * dt;
	}
	if (keystate[SDL_SCANCODE_A]) {
		light.position -= glm::vec3(camTrSpeed, 0, 0) * dt;
	}
	if (keystate[SDL_SCANCODE_D]) {
		light.position += glm::vec3(camTrSpeed, 0, 0) * dt;
	}
	if (keystate[SDL_SCANCODE_Q]) {

	}
	if (keystate[SDL_SCANCODE_E]) {

	}
}

void Draw()
{
	sdlAux->clearPixels();
	for (size_t i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) depthBuffer[i] = 0;

	for (const auto& t : triangles) {
		shader.normal = t.normal;
		shader.reflectance = t.color;
		renderer.DrawPolygon({ {t.v0}, {t.v1}, {t.v2} }, shader);
	}

	sdlAux->render();
}


int main(int argc, char* argv[]) {
	LoadTestModel(triangles);  // Load model
	sdlAux = new SDL2Aux(SCREEN_WIDTH, SCREEN_HEIGHT);
	t = SDL_GetTicks();	// Set start value for timer.

	shader.camera = &camera;
	shader.light = &light;
	shader.indirectLight = &indirectLight;

	shader.screen = sdlAux;
	shader.resolution = glm::ivec2(SCREEN_WIDTH, SCREEN_HEIGHT);
	shader.depthBuffer = depthBuffer;

	while (!sdlAux->quitEvent())
	{
		Update();
		Draw();
	}
	sdlAux->saveBMP("screenshot.bmp");
	return 0;
}
