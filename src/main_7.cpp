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
#include <deque>

using namespace std;

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
float P_DIST_DETAIL = 0.08f;
float P_DIST_GENERAL = 0.01f;
float P_SCALE_DETAIL = 0.8f;
float P_SCALE_GENERAL = 10.0f;
float BASE_CUBE_SCALE = 0.25f;
float CHUNK_AREA = TERRAIN_CHUNK_SIZE/2;

std::vector<std::vector<std::vector<std::vector<glm::vec3>>>> _terrainChunks = { {}, {}, {}, {} };

void makeChunk(int x, int y) {
	int quadrant = 0;

	if (x < 0) quadrant += 2;
	if (y < 0) quadrant += 1;

	while (_terrainChunks[quadrant].size() <= abs(x)) {
		_terrainChunks[quadrant].push_back({});
	}

	while (_terrainChunks[quadrant][abs(x)].size() <= abs(y)) {
		_terrainChunks[quadrant][abs(x)].push_back({});
	}
	
	if (_terrainChunks[quadrant][abs(x)][abs(y)].size() == 0) {
		for (int i = 0; i < TERRAIN_CHUNK_SIZE * TERRAIN_CHUNK_SIZE; i++) {
			float x_pos = x * TERRAIN_CHUNK_SIZE + float(i / TERRAIN_CHUNK_SIZE);
			float y_pos = y * TERRAIN_CHUNK_SIZE + float(i % TERRAIN_CHUNK_SIZE);
			float perlin_sample_general = glm::perlin(glm::vec2(x_pos * P_DIST_GENERAL, y_pos * P_DIST_GENERAL));
			float perlin_sample_detail = glm::perlin(glm::vec2(x_pos * P_DIST_DETAIL, y_pos * P_DIST_DETAIL));
			_terrainChunks[quadrant][abs(x)][abs(y)].push_back(glm::vec3(x_pos * 0.5f, perlin_sample_general * P_SCALE_GENERAL + perlin_sample_detail * P_SCALE_DETAIL, y_pos * 0.5f));
		}
	}
}

std::vector<glm::vec3>& getChunk(int x, int y) {
	int quadrant = 0;

	if (x < 0) quadrant += 2;
	if (y < 0) quadrant += 1;

	makeChunk(x, y);
	return _terrainChunks[quadrant][abs(x)][abs(y)];
}

// return value is vec2 with x and y being the chunk coords
glm::vec2 findClosestChunk(glm::vec3 pos) {
	return glm::vec2(floor(pos.x / CHUNK_AREA), floor(pos.z / CHUNK_AREA));
}

/*
std::vector<std::vector<std::vector<float*>>> _terrainChunks = { {}, {}, {}, {} };
// outermost vector is wrapper for the 4 quadrants ((x,y): ++, +-, -+, -- respectively)
// second vectors are quadrants
// third vectors are the rows of the specific quadrants
// innermost vectors are pointers to first elements of specific chunks
// all the access should be handled by getChunk() function which returns the inntermost vector reference
// that might change though haha
// please don't touch this monstrosity

int TERRAIN_CHUNK_X = 8;
int TERRAIN_CHUNK_Y = 8;
float PERLIN_SCALE = 0.1f;

float* makeChunk(int quadrant, int x, int y) {
	std::vector< std::vector<glm::vec3>> vertices_temp = {};

	for (int i = 0; i < TERRAIN_CHUNK_X + 1; i++) {
		vertices_temp.push_back({});
		for (int j = 0; j < TERRAIN_CHUNK_Y + 1; j++) {
			float perlin_val = glm::perlin(glm::vec2((float)i * PERLIN_SCALE, (float)j * PERLIN_SCALE));
			vertices_temp[i].push_back(glm::vec3(i, j, perlin_val));
		}
	}

	float* vertices = (float*)calloc(3*3*2* TERRAIN_CHUNK_X * TERRAIN_CHUNK_Y, sizeof(float)); // 3 coords of 3 verts of 2 triangles in square times 8 rows times 8 columns

	std::vector<glm::vec2> vert_helper = {
		glm::vec2(0,0),
		glm::vec2(0,1),
		glm::vec2(1,0),
		glm::vec2(0,1),
		glm::vec2(1,0),
		glm::vec2(1,1)
	};

	int index = 0;
	for (int i = 0; i < TERRAIN_CHUNK_X; i++) {
		for (int j = 0; j < TERRAIN_CHUNK_Y; j++) {
			for (int vert = 0; vert < 6; vert++) {
				vertices[index] = vertices_temp[i + vert_helper[vert].x][j + vert_helper[vert].y].x;
				vertices[index+1] = vertices_temp[i + vert_helper[vert].x][j + vert_helper[vert].y].y;
				vertices[index+2] = vertices_temp[i + vert_helper[vert].x][j + vert_helper[vert].y].z;
				index = index + 3;
			}
		}
	}

	_terrainChunks[quadrant][x][y] = vertices;

	return vertices;
}

float* getChunk(int x, int y) {
	int quadrant = 0;

	if (x < 0) quadrant += 2;
	if (y < 0) quadrant += 1;

	x = abs(x);
	y = abs(y);

	int needToGen = 0;
	if (_terrainChunks[quadrant].size() >= x) {
		needToGen = 1;
		_terrainChunks[quadrant].resize(x);
	}
	if (_terrainChunks[quadrant][x].size() >= y) {
		needToGen = 1;
		_terrainChunks[quadrant][x].resize(y);
	}

	if(needToGen) makeChunk(quadrant, x, y);

	return _terrainChunks[quadrant][x][y];
};
*/


// TERRAIN STUFF END

void drawObjectColor(Core::RenderContext context, glm::mat4 modelMatrix, glm::vec3 color)
{
	GLuint program = programColor;

	glUseProgram(program);

	glUniform3f(glGetUniformLocation(program, "objectColor"), color.x, color.y, color.z);
	glUniform3f(glGetUniformLocation(program, "lightDir"), lightDir.x, lightDir.y, lightDir.z);

	glm::mat4 transformation = perspectiveMatrix * cameraMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(program, "modelViewProjectionMatrix"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(program, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);

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
	glUniformMatrix4fv(glGetUniformLocation(program, "modelViewProjectionMatrix"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(program, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);

	Core::DrawContext(context);

	glUseProgram(0);
}


void renderScene()
{	
	cameraMatrix = createCameraMatrix();
	perspectiveMatrix = Core::createPerspectiveMatrix(0.1, 2000);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0f, 0.03f, 0.1f, 1.0f);

	// SKYBOX
	glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
	glUseProgram(skyboxShader);
	glUniform1i(glGetUniformLocation(skyboxShader, "skybox"), 0);
	glm::mat4 transformation = perspectiveMatrix * glm::mat4(glm::mat3(cameraMatrix));
	glUniformMatrix4fv(glGetUniformLocation(skyboxShader, "projectionView"), 1, GL_FALSE, (float*)&transformation);
	glBindVertexArray(skyboxVAO);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
	glDepthFunc(GL_LESS); // set depth function back to default
		
	//SUBMARINE
	glm::mat4 submarineTransformation = glm::translate(glm::vec3(0, -1.0f, -1.9f)) * glm::rotate(glm::radians(0.0f), glm::vec3(0, 1, 0)) * glm::scale(glm::vec3(0.25f));
	glm::mat4 submarineModelMatrix = glm::translate(cameraPos + cameraDir * 0.5f) * glm::mat4_cast(glm::inverse(rotationCamera)) * submarineTransformation;
	drawObjectTexture(submarine, submarineTransformation, submarineTextureId);
	drawObjectTexture(submarine, submarineModelMatrix, submarineTextureId);

	/*for (int j = 0; j < 4; j++) {
		for (int k = 0; k < 8; k++) {
			std::vector<glm::vec3>& chunk_ref = getChunk(j, k);
			for (int i = 0; i < TERRAIN_CHUNK_SIZE * TERRAIN_CHUNK_SIZE; i++) {
				if ((i / 16) % 2 == 0 && i % 2 == 0)drawObjectTexture(terrainCube, glm::translate(chunk_ref[i]) * glm::scale(glm::vec3(BASE_CUBE_SCALE * 2)), terrainTextureId);

			}
		}
	}*/

	glm::vec2 cur_chunk = findClosestChunk(cameraPos);

	for (int j = -3; j <= 3; j++) {
		for (int k = -3; k <= 3; k++) {
			std::vector<glm::vec3>& chunk_ref = getChunk(cur_chunk.x + j, cur_chunk.y + k);
			float scale_multiplier = 2^max(abs(j),abs(k));
			for (int row = 0; row < TERRAIN_CHUNK_SIZE; row++) {
				for (int col = 0; col < TERRAIN_CHUNK_SIZE; col++) {
					drawObjectTexture(terrainCube, glm::translate(chunk_ref[row*TERRAIN_CHUNK_SIZE + col] - glm::vec3(BASE_CUBE_SCALE)) * glm::scale(glm::vec3(BASE_CUBE_SCALE)), terrainTextureId);
				}
			}

			
		}
	}

	

	


	glUseProgram(0);
	glutSwapBuffers();
}

void loadModelToContext(std::string path, Core::RenderContext& context)
{
	Assimp::Importer import;
	const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_CalcTangentSpace);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
		return;
	}
	context.initFromAssimpMesh(scene->mMeshes[0]);
}

void initModels() {
	loadModelToContext("models/YellowSubmarine.obj", submarine);
	submarineTextureId = Core::LoadTexture("textures/submarine-tex.png");

	loadModelToContext("models/terrain_cube.obj", terrainCube);
	terrainTextureId = Core::LoadTexture("textures/sand.jpg");
}

void createSkybox() {
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
}

void init()
{
	glEnable(GL_DEPTH_TEST);
	programColor = shaderLoader.CreateProgram("shaders/shader_color.vert", "shaders/shader_color.frag");
	programTexture = shaderLoader.CreateProgram("shaders/shader_tex.vert", "shaders/shader_tex.frag");
	skyboxShader = shaderLoader.CreateProgram("shaders/skybox.vert", "shaders/skybox.frag");
	// cube VAO
	cubemapTexture = loadCubemap(faces);
	// skybox VAO
	createSkybox();
	initModels();
}

void shutdown()
{
	shaderLoader.DeleteProgram(programColor);
	shaderLoader.DeleteProgram(programTexture);
	shaderLoader.DeleteProgram(skyboxShader);
}

void idle()
{
	glutPostRedisplay();
}

int main(int argc, char** argv)
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
