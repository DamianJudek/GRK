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
#include "Particles.h"
#include "Physics.h"

using namespace std;

GLuint programParticles;
GLuint programColor;
GLuint programTexture;
GLuint skyboxShader;
GLuint skyboxVAO, skyboxVBO;

unsigned int cubemapTexture = loadCubemap(faces);

Core::Shader_Loader shaderLoader;

Core::RenderContext submarine;
GLuint submarineTextureId;

// TERRAIN STUFF

Core::RenderContext terrainCube;
GLuint terrainTextureId;

int TERRAIN_CHUNK_SIZE = 16;
int TERRAIN_RENDER_DISTANCE = 9;
float P_DIST_DETAIL = 0.08f;
float P_DIST_GENERAL = 0.01f;
float P_SCALE_DETAIL = 0.8f;
float P_SCALE_GENERAL = 10.0f;
float BASE_CUBE_SCALE = 0.25f;
float CHUNK_AREA = TERRAIN_CHUNK_SIZE / 2;

// vectors starting from the outermost are: everything, quadrants (required because of negative coordinates), rows of chunks, chunks
// chunk vectors hold positions of cubes in the chunk in a single wrapping line
// please don't access this manually if you don't have to
std::vector<std::vector<std::vector<std::vector<glm::vec3>>>> _terrainChunks = {{}, {}, {}, {}};

void makeChunk(int x, int y)
{
	int quadrant = 0;

	if (x < 0)
		quadrant += 2;
	if (y < 0)
		quadrant += 1;

	while (_terrainChunks[quadrant].size() <= abs(x))
	{
		_terrainChunks[quadrant].push_back({});
	}

	while (_terrainChunks[quadrant][abs(x)].size() <= abs(y))
	{
		_terrainChunks[quadrant][abs(x)].push_back({});
	}

	if (_terrainChunks[quadrant][abs(x)][abs(y)].size() == 0)
	{
		for (int i = 0; i < TERRAIN_CHUNK_SIZE * TERRAIN_CHUNK_SIZE; i++)
		{
			float x_pos = x * TERRAIN_CHUNK_SIZE + float(i / TERRAIN_CHUNK_SIZE);
			float y_pos = y * TERRAIN_CHUNK_SIZE + float(i % TERRAIN_CHUNK_SIZE);
			float perlin_sample_general = glm::perlin(glm::vec2(x_pos * P_DIST_GENERAL, y_pos * P_DIST_GENERAL));
			float perlin_sample_detail = glm::perlin(glm::vec2(x_pos * P_DIST_DETAIL, y_pos * P_DIST_DETAIL));
			_terrainChunks[quadrant][abs(x)][abs(y)].push_back(glm::vec3(x_pos * 0.5f, perlin_sample_general * P_SCALE_GENERAL + perlin_sample_detail * P_SCALE_DETAIL, y_pos * 0.5f));
		}
	}
}

std::vector<glm::vec3> &getChunk(int x, int y)
{
	int quadrant = 0;

	if (x < 0)
		quadrant += 2;
	if (y < 0)
		quadrant += 1;

	makeChunk(x, y);
	return _terrainChunks[quadrant][abs(x)][abs(y)];
}

// return value is vec2 with x and y being the chunk coords
glm::vec2 findClosestChunk(glm::vec3 pos)
{
	return glm::vec2(floor(pos.x / CHUNK_AREA), floor(pos.z / CHUNK_AREA));
}

// TERRAIN STUFF END
Core::RenderContext flowerOne;
GLuint flowerOneTexture;
Core::RenderContext flowerTwo;
GLuint flowerTwoTexture;

glm::vec3 buffer[45];

Core::RenderContext seahorse;
Core::RenderContext fish_models[4];
GLuint fishTextureId;

Core::RenderContext coin;
GLuint coinTextureId;

Core::RenderContext ground;
GLuint groundTexture;

Core::RenderContext bubble;

glm::vec4 coins[10];
int numberOfCoins = 0;

void drawObjectColor(Core::RenderContext context, glm::mat4 modelMatrix, glm::vec3 color)
{
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	GLuint program = programColor;

	glUseProgram(program);

	glUniform3f(glGetUniformLocation(program, "objectColor"), color.x, color.y, color.z);
	glUniform3f(glGetUniformLocation(program, "lightDir"), lightDir.x, lightDir.y, lightDir.z);

	glm::mat4 transformation = perspectiveMatrix * cameraMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(program, "modelViewProjectionMatrix"), 1, GL_FALSE, (float *)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(program, "modelMatrix"), 1, GL_FALSE, (float *)&modelMatrix);

	Core::DrawContext(context);

	glUseProgram(0);
}

void drawObjectColor(Core::RenderContext context, glm::mat4 modelMatrix, glm::vec4 color)
{
	GLuint program = programColor;

	glUseProgram(program);

	glUniform3f(glGetUniformLocation(program, "objectColor"), color.x, color.y, color.z);
	glUniform3f(glGetUniformLocation(program, "lightDir"), lightDir.x, lightDir.y, lightDir.z);

	glm::mat4 transformation = perspectiveMatrix * cameraMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(program, "modelViewProjectionMatrix"), 1, GL_FALSE, (float *)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(program, "modelMatrix"), 1, GL_FALSE, (float *)&modelMatrix);

	Core::DrawContext(context);

	glUseProgram(0);
}

void drawObjectTexture(Core::RenderContext context, glm::mat4 modelMatrix, GLuint textureId)
{
	GLuint program = programTexture;

	glUseProgram(program);

	glUniform3f(glGetUniformLocation(program, "lightDir"), lightDir.x, lightDir.y, lightDir.z);
	Core::SetActiveTexture(textureId, "textureSampler", program, 0);

	glm::mat4 transformation = perspectiveMatrix * cameraMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(program, "modelViewProjectionMatrix"), 1, GL_FALSE, (float *)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(program, "modelMatrix"), 1, GL_FALSE, (float *)&modelMatrix);

	Core::DrawContext(context);

	glUseProgram(0);
}

void renderScene()
{
	cameraMatrix = createCameraMatrix();
	perspectiveMatrix = Core::createPerspectiveMatrix(0.1, 2000);
	float current_time = glutGet(GLUT_ELAPSED_TIME) / 1000.f;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0f, 0.03f, 0.1f, 1.0f);

	// SKYBOX
	glDepthFunc(GL_LEQUAL); // change depth function so depth test passes when values are equal to depth buffer's content
	glUseProgram(skyboxShader);
	glUniform1i(glGetUniformLocation(skyboxShader, "skybox"), 0);
	glm::mat4 transformation = perspectiveMatrix * glm::mat4(glm::mat3(cameraMatrix));
	glUniformMatrix4fv(glGetUniformLocation(skyboxShader, "projectionView"), 1, GL_FALSE, (float *)&transformation);
	glBindVertexArray(skyboxVAO);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
	glDepthFunc(GL_LESS); // set depth function back to default

	// SUBMARINE
	glm::mat4 submarineTransformation = glm::translate(glm::vec3(0, -1.0f, -1.9f)) * glm::rotate(glm::radians(0.0f), glm::vec3(0, 1, 0)) * glm::scale(glm::vec3(0.25f));
	glm::mat4 submarineModelMatrix = glm::translate(cameraPos + cameraDir * 0.5f) * glm::mat4_cast(glm::inverse(rotationCamera)) * submarineTransformation;
	drawObjectTexture(submarine, submarineTransformation, submarineTextureId);
	drawObjectTexture(submarine, submarineModelMatrix, submarineTextureId);
	drawObjectTexture(fish_models[0], glm::scale(glm::rotate(submarineTransformation, 4.71239f, glm::vec3(0.0f, 1.0f, 0.0f)), glm::vec3(10.f, 10.f, 10.f)), fishTextureId);

	for (int j = 0; j < 45; j++)
	{
		if (j % 9 == 0) drawObjectTexture(flowerOne, glm::translate(buffer[j]), flowerOneTexture);
		else drawObjectTexture(flowerTwo, glm::translate(buffer[j]), flowerTwoTexture);
	}

	// terrain lol?
	drawObjectTexture(ground, submarineTransformation, groundTexture);
	drawObjectColor(ground, submarineTransformation, glm::vec3(0.4, 0.5, 0.7));

	// particles
	simulateParticles(cameraPos);
	updateParticles();
	bindParticles(cameraSide, cameraVertical, perspectiveMatrix, cameraMatrix, programParticles);
	renderParticles();

	// FISH OVER INTERPOLATED PATHS
	for (int i = 0; i < fishe.size(); i++)
	{
		drawObjectTexture(fish_models[fishe[i]->model_id], glm::scale(glm::rotate(animationMatrix(current_time + fishe[i]->t_offset, fishe[i]), 4.71238f, glm::vec3(0.0f, 1.0f, 0.0f)), glm::vec3(1.f, 1.f, 1.f)), fishTextureId);
	}

	for (int j = 0; j < 10; j++)
	{
		if (getCoin && (((cameraPos.x - coins[j].x) > (-20.0f) && (cameraPos.x - coins[j].x) < (20.0f)) && ((cameraPos.y - coins[j].y) > (-20.0f) && (cameraPos.y - coins[j].y) < (20.0f)) && ((cameraPos.z - coins[j].z) > (-20.0f) && (cameraPos.z - coins[j].z) < (20.0f))) && coins[j].w == 1)
		{
			coins[j].w = 0;
			numberOfCoins += 1;
			std::cout << numberOfCoins << std::endl;
		}
		if (coins[j].w == 1)
		{
			drawObjectTexture(coin, glm::translate(glm::vec3(coins[j].x, coins[j].y, coins[j].z)), coinTextureId);
		}
	}

	if (createBubble && numberOfCoins > 0)
	{
		if (current_time - timeOfLastBubbleCreation > 1)
		{
			makeBubble(current_time, cameraPos);
			numberOfCoins -= 1;
			timeOfLastBubbleCreation = current_time;
		}
		else
		{
			std::cout << current_time - timeOfLastBubbleCreation << std::endl;
		}
		createBubble = false;
	}

	for (int i = 0; i < bubbles.size(); i++)
	{
		drawObjectColor(bubble, glm::scale(glm::translate(glm::vec3(bubbles[i]->position.x + 5, bubbles[i]->position.y + 5 + (current_time - bubbles[i]->creationTime), bubbles[i]->position.z)), glm::vec3(1, 1, 1)), glm::vec3(0.5, 0.5, 0.5));
	}

	glm::vec2 cur_chunk = findClosestChunk(cameraPos);

	for (int j = -TERRAIN_RENDER_DISTANCE; j <= TERRAIN_RENDER_DISTANCE; j++)
	{
		for (int k = -TERRAIN_RENDER_DISTANCE; k <= TERRAIN_RENDER_DISTANCE; k++)
		{
			std::vector<glm::vec3> &chunk_ref = getChunk(cur_chunk.x + j, cur_chunk.y + k);
			float scale_multiplier = fmax(1, pow(2, fmax(ceil(abs(j) / 3), ceil(abs(k) / 3))));

			for (int row = 0; row < TERRAIN_CHUNK_SIZE; row += scale_multiplier)
			{
				for (int col = 0; col < TERRAIN_CHUNK_SIZE; col += scale_multiplier)
				{
					drawObjectTexture(terrainCube, glm::translate(chunk_ref[row * TERRAIN_CHUNK_SIZE + col] + glm::vec3(BASE_CUBE_SCALE * scale_multiplier, -BASE_CUBE_SCALE * scale_multiplier, BASE_CUBE_SCALE * scale_multiplier)) * glm::scale(glm::vec3(BASE_CUBE_SCALE * scale_multiplier)), terrainTextureId);
				}
			}
		}
	}

	glUseProgram(0);
	glutSwapBuffers();
}

void loadModelToContext(std::string path, Core::RenderContext &context)
{
	Assimp::Importer import;
	const aiScene *scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_CalcTangentSpace);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
		return;
	}
	context.initFromAssimpMesh(scene->mMeshes[0]);
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

	loadModelToContext("models/terrain.obj", ground);
	groundTexture = Core::LoadTexture("textures/lava.jpg");
	loadModelToContext("models/seahorse.obj", seahorse);

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

void createSkybox()
{
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
}

void init()
{
	srand(time(0));
	glEnable(GL_DEPTH_TEST);
	programColor = shaderLoader.CreateProgram("shaders/shader_color.vert", "shaders/shader_color.frag");
	programTexture = shaderLoader.CreateProgram("shaders/shader_tex.vert", "shaders/shader_tex.frag");
	programParticles = shaderLoader.CreateProgram("shaders/particles.vert", "shaders/particles.frag");
	skyboxShader = shaderLoader.CreateProgram("shaders/skybox.vert", "shaders/skybox.frag");

	// cube VAO
	cubemapTexture = loadCubemap(faces);
	// skybox VAO
	createSkybox();
	initParticles();

	initModels();

	for (int i = 0; i < 45; i++)
	{
		buffer[i] = glm::ballRand(100.0);
		buffer[i].y = 0;
	}

	initPaths(15,
		/* path_radius */ glm::vec2(30.0f, 80.0f),
		/* placement_area_x */ glm::vec2(-150.0f, 150.0f),
		/* placement_area_y */ glm::vec2(-150.0f, 150.0f),
		/* placement_area_z */ glm::vec2(0.0f, 100.0f),
		/* rand_x_offset */ glm::vec2(-30.0f, 30.0f),
		/* rand_y_offset */ glm::vec2(-30.0f, 30.0f),
		/* rand_z_offset */ glm::vec2(-10.0f, 10.0f));
	initPathRots();
	initFish(300);

	for (int i = 0; i < 10; i++)
	{
		coins[i] = glm::vec4(glm::ballRand(100.0), 1);
		coins[i].y = 0;
	}
}

void shutdown()
{
	shaderLoader.DeleteProgram(programColor);
	shaderLoader.DeleteProgram(programTexture);
	shaderLoader.DeleteProgram(skyboxShader);
	shaderLoader.DeleteProgram(programParticles);

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
