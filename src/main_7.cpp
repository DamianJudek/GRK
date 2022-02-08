﻿#define STB_IMAGE_IMPLEMENTATION

#include <iostream>
#include <cmath>
#include <ctime>
//#include <random>
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

using namespace std;


/*
// random
random_device rd;
mt19937 gen(rd());
uniform_real_distribution<> dist(0.0f, 1.0f);
*/


GLuint programColor;
GLuint programTexture;
GLuint skyboxShader;
GLuint skyboxVAO, skyboxVBO;

unsigned int cubemapTexture = loadCubemap(faces);

Core::Shader_Loader shaderLoader;

Core::RenderContext submarine;
GLuint submarineTextureId;

Core::RenderContext seahorse;
Core::RenderContext fish_models[4];
GLuint fishTextureId;

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
	float current_time = glutGet(GLUT_ELAPSED_TIME) / 1000.f;

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
	drawObjectTexture(fish_models[0], glm::scale(glm::rotate(submarineTransformation, 4.71239f, glm::vec3(0.0f, 1.0f, 0.0f)), glm::vec3(10.f, 10.f, 10.f)), fishTextureId);

	// FISH OVER INTERPOLATED PATHS
	for (int i = 0; i < fishe.size(); i++) {
		drawObjectTexture(fish_models[fishe[i]->model_id], glm::scale(glm::rotate(animationMatrix(current_time+fishe[i]->t_offset, fishe[i]), 4.71238f, glm::vec3(0.0f, 1.0f, 0.0f)), glm::vec3(1.f, 1.f, 1.f)), fishTextureId);
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
	
	loadModelToContext("models/seahorse.obj", seahorse);

	fishTextureId = Core::LoadTexture("textures/PolyPackFish.png");
	loadModelToContext("models/FishV1.obj", fish_models[0]);
	loadModelToContext("models/FishV2.obj", fish_models[1]);
	loadModelToContext("models/FishV3.obj", fish_models[2]);
	loadModelToContext("models/FishV4.obj", fish_models[3]);

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
