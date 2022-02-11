#include <iostream>
#include <string>
#include <vector>
#include <random>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "Render_Utils.h"
#include "appConfig.h"
#include "Particles.h"

using namespace std;

random_device rd_TEMP;
mt19937 gen_TEMP(rd_TEMP());

void initSkybox()
{
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
}

void initPlants()
{
	for (int i = 0; i < 45; i++)
	{
		plantsBuffer[i] = glm::ballRand(100.0);
		plantsBuffer[i].y = 0;
	}
};

void initCoins()
{
	for (int i = 0; i < 10; i++)
	{
		coins[i] = glm::vec4(glm::ballRand(100.0), 1);
		coins[i].y = 0;
	}
};

float getHeightAtPoint(float x, float y)
{
	float perlin_sample_general = glm::perlin(glm::vec2(x * P_DIST_GENERAL, y * P_DIST_GENERAL));
	float perlin_sample_detail = glm::perlin(glm::vec2(x * P_DIST_DETAIL, y * P_DIST_DETAIL));
	return perlin_sample_general * P_SCALE_GENERAL + perlin_sample_detail * P_SCALE_DETAIL;
}

uniform_real_distribution<> pos_distr(0.0f, float(TERRAIN_CHUNK_SIZE));
uniform_real_distribution<> plant_id_distr(0.0f, 1.0f);
uniform_int_distribution<> plant_amount_distr(PLANTS_PER_CHUNK.x, PLANTS_PER_CHUNK.y);

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
		_objectChunks[quadrant].push_back({});
	}

	while (_terrainChunks[quadrant][abs(x)].size() <= abs(y))
	{
		_terrainChunks[quadrant][abs(x)].push_back({});
		_objectChunks[quadrant][abs(x)].push_back({});
	}

	if (_terrainChunks[quadrant][abs(x)][abs(y)].size() == 0)
	{
		for (int i = 0; i < TERRAIN_CHUNK_SIZE * TERRAIN_CHUNK_SIZE; i++)
		{
			float x_pos = (x * TERRAIN_CHUNK_SIZE + float(i / TERRAIN_CHUNK_SIZE)) * 0.5f;
			float y_pos = (y * TERRAIN_CHUNK_SIZE + float(i % TERRAIN_CHUNK_SIZE)) * 0.5f;
			_terrainChunks[quadrant][abs(x)][abs(y)].push_back(glm::vec3(x_pos, getHeightAtPoint(x_pos, y_pos), y_pos));
		}

		int plant_amount = plant_amount_distr(gen_TEMP);
		for (int i = 0; i < plant_amount; i++)
		{
			float x_pos = (x * TERRAIN_CHUNK_SIZE + pos_distr(gen_TEMP)) * 0.5f;
			float y_pos = (y * TERRAIN_CHUNK_SIZE + pos_distr(gen_TEMP)) * 0.5f;
			int plant_id = plant_id_distr(gen_TEMP) < 0.05 ? 0 : 1;
			RandomObject *temp = new Plant(glm::vec3(x_pos, getHeightAtPoint(x_pos, y_pos), y_pos), plant_id);
			_objectChunks[quadrant][abs(x)][abs(y)].push_back(temp);
		}
	}
}

std::vector<glm::vec3> &getTerrainChunk(int x, int y)
{
	int quadrant = 0;

	if (x < 0)
		quadrant += 2;
	if (y < 0)
		quadrant += 1;

	makeChunk(x, y);
	return _terrainChunks[quadrant][abs(x)][abs(y)];
}

std::vector<RandomObject *> &getObjectChunk(int x, int y)
{
	int quadrant = 0;

	if (x < 0)
		quadrant += 2;
	if (y < 0)
		quadrant += 1;

	return _objectChunks[quadrant][abs(x)][abs(y)];
}
// return value is vec2 with x and y being the chunk coords
glm::vec2 findClosestChunk(glm::vec3 pos)
{
	return glm::vec2(floor(pos.x / CHUNK_AREA), floor(pos.z / CHUNK_AREA));
}

void drawObjectColor(Core::RenderContext context, glm::mat4 modelMatrix, glm::vec3 color)
{
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	GLuint program = colorShader;

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
	GLuint program = textureShader;

	glUseProgram(program);

	glUniform3f(glGetUniformLocation(program, "lightDir"), lightDir.x, lightDir.y, lightDir.z);
	Core::SetActiveTexture(textureId, "textureSampler", program, 0);

	glm::mat4 transformation = perspectiveMatrix * cameraMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(program, "modelViewProjectionMatrix"), 1, GL_FALSE, (float *)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(program, "modelMatrix"), 1, GL_FALSE, (float *)&modelMatrix);

	Core::DrawContext(context);

	glUseProgram(0);
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

void drawSkybox()
{
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
}

void drawSubmarine()
{
	glm::mat4 submarineTransformation = glm::translate(glm::vec3(0, -1.0f, -1.9f)) * glm::rotate(glm::radians(0.0f), glm::vec3(0, 1, 0)) * glm::scale(glm::vec3(0.25f));
	glm::mat4 submarineModelMatrix = glm::translate(cameraPos + cameraDir * 0.5f) * glm::mat4_cast(glm::inverse(rotationCamera)) * submarineTransformation;
	drawObjectTexture(submarine, submarineModelMatrix, submarineTextureId);
}

void drawParticles()
{
	simulateParticles(cameraPos);
	updateParticles();
	bindParticles(cameraSide, cameraVertical, perspectiveMatrix, cameraMatrix, particlesShader);
	renderParticles();
}

void drawFish(float current_time)
{
	for (int i = 0; i < fishe.size(); i++)
	{
		drawObjectTexture(fish_models[fishe[i]->model_id], glm::scale(glm::rotate(animationMatrix(current_time + fishe[i]->t_offset, fishe[i]), 4.71238f, glm::vec3(0.0f, 1.0f, 0.0f)), glm::vec3(1.f, 1.f, 1.f)), fishTextureId);
	}
};

void drawCoins()
{
	for (int j = 0; j < 10; j++)
	{
		if (getCoin && (((cameraPos.x - coins[j].x) > (-20.0f) && (cameraPos.x - coins[j].x) < (20.0f)) && ((cameraPos.y - coins[j].y) > (-20.0f) && (cameraPos.y - coins[j].y) < (20.0f)) && ((cameraPos.z - coins[j].z) > (-20.0f) && (cameraPos.z - coins[j].z) < (20.0f))) && coins[j].w == 1)
		{
			coins[j].w = 0;
			numberOfCoins += 1;
			cout << numberOfCoins << endl;
		}
		if (coins[j].w == 1)
		{
			drawObjectTexture(coin, glm::translate(glm::vec3(coins[j].x, coins[j].y, coins[j].z)), coinTextureId);
		}
	}
};

void drawBubbles(float current_time)
{
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
			cout << current_time - timeOfLastBubbleCreation << endl;
		}
		createBubble = false;
	}

	for (int i = 0; i < bubbles.size(); i++)
	{
		drawObjectColor(bubble, glm::scale(glm::translate(glm::vec3(bubbles[i]->position.x + 5, bubbles[i]->position.y + 5 + (current_time - bubbles[i]->creationTime), bubbles[i]->position.z)), glm::vec3(1, 1, 1)), glm::vec3(0.5, 0.5, 0.5));
	}
};

void drawTerrain()
{
	glm::vec2 cur_chunk = findClosestChunk(cameraPos);

	for (int j = -TERRAIN_RENDER_DISTANCE; j <= TERRAIN_RENDER_DISTANCE; j++)
	{
		for (int k = -TERRAIN_RENDER_DISTANCE; k <= TERRAIN_RENDER_DISTANCE; k++)
		{
			std::vector<glm::vec3> &chunk_ref = getTerrainChunk(cur_chunk.x + j, cur_chunk.y + k);
			float scale_multiplier = fmax(1, pow(2, fmax(ceil(abs(j) / 3), ceil(abs(k) / 3))));

			for (int row = 0; row < TERRAIN_CHUNK_SIZE; row += scale_multiplier)
			{
				for (int col = 0; col < TERRAIN_CHUNK_SIZE; col += scale_multiplier)
				{
					drawObjectTexture(terrainCube, glm::translate(chunk_ref[row * TERRAIN_CHUNK_SIZE + col] + glm::vec3(BASE_CUBE_SCALE * scale_multiplier, -BASE_CUBE_SCALE * scale_multiplier, BASE_CUBE_SCALE * scale_multiplier)) * glm::scale(glm::vec3(BASE_CUBE_SCALE * scale_multiplier)), terrainTextureId);
				}
			}
			std::vector<RandomObject *> &object_chunk_ref = getObjectChunk(cur_chunk.x + j, cur_chunk.y + k);
			for (int index = 0; index < object_chunk_ref.size(); index++)
			{
				drawObjectTexture(flower_models[static_cast<Plant *>(object_chunk_ref[index])->model_id], glm::translate(object_chunk_ref[index]->pos) * glm::scale(glm::vec3(0.25f)), flowerTextureIds[static_cast<Plant *>(object_chunk_ref[index])->model_id]);
			}
		}
	}
}

void keyboard(unsigned char key, int x, int y)
{
	getCoin = false;
	createBubble = false;
	float angleSpeed = 0.5f;
	float moveSpeed = 2.f;
	switch (key)
	{
	case 'z':
		cameraAngle -= angleSpeed;
		break;
	case 'x':
		cameraAngle += angleSpeed;
		break;
	case 'w':
		cameraPos += cameraDir * moveSpeed;
		break;
	case 's':
		cameraPos -= cameraDir * moveSpeed;
		break;
	case 'd':
		cameraPos += cameraSide * moveSpeed;
		break;
	case 'a':
		cameraPos -= cameraSide * moveSpeed;
		break;
	case 'l':
		getCoin = true;
		break;
	case 'p':
		createBubble = true;
		break;
	}
}

void mouse(int x, int y)
{
	if (old_x >= 0)
	{
		delta_x = x - old_x;
		delta_y = y - old_y;
	}
	old_x = x;
	old_y = y;
}