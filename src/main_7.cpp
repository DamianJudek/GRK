#define STB_IMAGE_IMPLEMENTATION

#include <iostream>
#include <cmath>
#include <ctime>
#include "glm.hpp"
#include "glew.h"
#include "freeglut.h"
#include "gtx/matrix_decompose.hpp"
#include "ext.hpp"
#include "Shader_Loader.h"
#include "Render_Utils.h"
#include "Camera.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "stb_image.h"
#include "Texture.h"
#include "appConfig.h"
#include "utils.h"
#include "Particles.h"
#include "Physics.h"

using namespace std;

Core::Shader_Loader shaderLoader;

void renderScene()
{
	cameraMatrix = createCameraMatrix();
	perspectiveMatrix = Core::createPerspectiveMatrix(0.1, 2000);
	float current_time = glutGet(GLUT_ELAPSED_TIME) / 1000.f;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0f, 0.03f, 0.1f, 1.0f);

	drawSkybox();
	drawSubmarine();
	drawPlants();
	drawParticles();
	drawFish(current_time);
	drawCoins();
	drawBubbles(current_time);
	drawTerrain();

	glUseProgram(0);
	glutSwapBuffers();
}

void initModels()
{
	loadModelToContext("models/YellowSubmarine.obj", submarine);
	submarineTextureId = Core::LoadTexture("textures/submarine-tex.png");

	loadModelToContext("models/terrain_cube.obj", terrainCube);
	terrainTextureId = Core::LoadTexture("textures/sand.jpg");

	loadModelToContext("models/matteucia_struthiopteris_1.obj", flowerOne);
	flowerOneTexture = Core::LoadTexture("textures/matteuccia_struthiopteris_leaf_1_01_diffuse.jpg");
	loadModelToContext("models/senecio_1.obj", flowerTwo);
	flowerTwoTexture = Core::LoadTexture("textures/senecio_m_leaf_1_1_diffuse_1.jpg");

	fishTextureId = Core::LoadTexture("textures/PolyPackFish.png");
	loadModelToContext("models/FishV1.obj", fish_models[0]);
	loadModelToContext("models/FishV2.obj", fish_models[1]);
	loadModelToContext("models/FishV3.obj", fish_models[2]);
	loadModelToContext("models/FishV4.obj", fish_models[3]);

	loadModelToContext("models/Coin.obj", coin);
	coinTextureId = Core::LoadTexture("textures/Textures/BTC_Albedo.png");

	loadModelToContext("models/terrain_textured.obj", ground);
	groundTexture = Core::LoadTexture("textures/sand.jpg");

	loadModelToContext("models/sphere.obj", bubble);
}

void init()
{
	srand(time(0));
	glEnable(GL_DEPTH_TEST);
	colorShader = shaderLoader.CreateProgram("shaders/shader_color.vert", "shaders/shader_color.frag");
	textureShader = shaderLoader.CreateProgram("shaders/shader_tex.vert", "shaders/shader_tex.frag");
	particlesShader = shaderLoader.CreateProgram("shaders/particles.vert", "shaders/particles.frag");
	skyboxShader = shaderLoader.CreateProgram("shaders/skybox.vert", "shaders/skybox.frag");

	// cube VAO
	cubemapTexture = loadCubemap(faces);
	// skybox VAO
	initSkybox();

	initParticles();
	initModels();
	initPlants();
	initPaths(15, fishKeyframes);
	initPathRots();
	initFish(300);
	initCoins();
}

void shutdown()
{
	shaderLoader.DeleteProgram(colorShader);
	shaderLoader.DeleteProgram(textureShader);
	shaderLoader.DeleteProgram(skyboxShader);
	shaderLoader.DeleteProgram(particlesShader);

	deleteParticles();
}

void idle()
{
	glutPostRedisplay();
}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutSetOption(GLUT_MULTISAMPLE, 4);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA | GLUT_MULTISAMPLE);
	glutInitWindowPosition(400, 100);
	glutInitWindowSize(800, 800);
	glutCreateWindow("Underwater world");
	glewInit();
	init();
	glutPassiveMotionFunc(mouse);
	glutKeyboardFunc(keyboard);
	glutDisplayFunc(renderScene);
	glutIdleFunc(idle);
	glutMainLoop();
	shutdown();

	return 0;
}
