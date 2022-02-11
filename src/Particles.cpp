#include "Texture.h"
#include <fstream>
#include <iterator>
#include <vector>
#include "picopng.h"
#include "stb_image.h"
#include "glm.hpp"
#include <cmath>
#include <algorithm>
#include "appConfig.h"

#define FOURCC_DXT1 0x31545844
#define FOURCC_DXT3 0x33545844
#define FOURCC_DXT5 0x35545844

GLuint billboard_vertex_buffer;
GLuint particles_position_buffer;
GLuint particles_color_buffer;
GLuint particleTexture;

static GLfloat *g_particule_position_size_data;
static GLubyte *g_particule_color_data;

int ParticlesCount = 0;
double lastTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;

struct Particle
{
	glm::vec3 pos, speed;
	unsigned char r, g, b, a; 
	float size, angle, weight;
	float life;	
	float cameradistance;

	bool operator<(const Particle &that) const
	{
		return this->cameradistance > that.cameradistance;
	}
};

const int MaxParticles = 500;
Particle ParticlesContainer[MaxParticles];
int LastUsedParticle = 0;
int locationIndex = 0;

int FindUnusedParticle()
{

	for (int i = LastUsedParticle; i < MaxParticles; i++)
	{
		if (ParticlesContainer[i].life < 0)
		{
			LastUsedParticle = i;
			return i;
		}
	}

	for (int i = 0; i < LastUsedParticle; i++)
	{
		if (ParticlesContainer[i].life < 0)
		{
			LastUsedParticle = i;
			return i;
		}
	}

	return 0;
}

void SortParticles()
{
	std::sort(&ParticlesContainer[0], &ParticlesContainer[MaxParticles]);
}

GLuint loadDDS(const char *imagepath)
{
	unsigned char header[124];

	FILE *fp;

	fp = fopen(imagepath, "rb");
	if (fp == NULL)
	{
		printf("%s could not be opened. Are you in the right directory ? Don't forget to read the FAQ !\n", imagepath);
		getchar();
		return 0;
	}

	char filecode[4];
	fread(filecode, 1, 4, fp);
	if (strncmp(filecode, "DDS ", 4) != 0)
	{
		fclose(fp);
		return 0;
	}

	fread(&header, 124, 1, fp);

	unsigned int height = *(unsigned int *)&(header[8]);
	unsigned int width = *(unsigned int *)&(header[12]);
	unsigned int linearSize = *(unsigned int *)&(header[16]);
	unsigned int mipMapCount = *(unsigned int *)&(header[24]);
	unsigned int fourCC = *(unsigned int *)&(header[80]);

	unsigned char *buffer;
	unsigned int bufsize;
	bufsize = mipMapCount > 1 ? linearSize * 2 : linearSize;
	buffer = (unsigned char *)malloc(bufsize * sizeof(unsigned char));
	fread(buffer, 1, bufsize, fp);
	fclose(fp);

	unsigned int components = (fourCC == FOURCC_DXT1) ? 3 : 4;
	unsigned int format;
	switch (fourCC)
	{
	case FOURCC_DXT1:
		format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
		break;
	case FOURCC_DXT3:
		format = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
		break;
	case FOURCC_DXT5:
		format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
		break;
	default:
		free(buffer);
		return 0;
	}

	GLuint textureID;
	glGenTextures(1, &textureID);

	glBindTexture(GL_TEXTURE_2D, textureID);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	unsigned int blockSize = (format == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT) ? 8 : 16;
	unsigned int offset = 0;

	for (unsigned int level = 0; level < mipMapCount && (width || height); ++level)
	{
		unsigned int size = ((width + 3) / 4) * ((height + 3) / 4) * blockSize;
		glCompressedTexImage2D(GL_TEXTURE_2D, level, format, width, height,
													 0, size, buffer + offset);

		offset += size;
		width /= 2;
		height /= 2;

		if (width < 1)
			width = 1;
		if (height < 1)
			height = 1;
	}

	free(buffer);

	return textureID;
}

void initParticles()
{
	particleTexture = loadDDS("textures/particle.DDS");

	g_particule_position_size_data = new GLfloat[MaxParticles * 4];
	g_particule_color_data = new GLubyte[MaxParticles * 4];

	for (int i = 0; i < MaxParticles; i++)
	{
		ParticlesContainer[i].life = -1.0f;
		ParticlesContainer[i].cameradistance = -1.0f;
	}

	static const GLfloat g_vertex_buffer_data[] = {
			-0.5f,
			-0.5f,
			0.0f,
			0.5f,
			-0.5f,
			0.0f,
			-0.5f,
			0.5f,
			0.0f,
			0.5f,
			0.5f,
			0.0f,
	};

	glGenBuffers(1, &billboard_vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, billboard_vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);
	glGenBuffers(1, &particles_position_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
	glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);
	glGenBuffers(1, &particles_color_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
	glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW);
}

void simulateParticles(glm::vec3 cameraPos)
{
	double currentTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
	double delta = currentTime - lastTime;
	lastTime = currentTime;

	int newparticles = (int)(delta * 200.0);
	if (newparticles > (int)(0.016f * 200.0))
		newparticles = (int)(0.016f * 200.0);

	for (int i = 0; i < newparticles; i++)
	{
		int particleIndex = FindUnusedParticle();
		ParticlesContainer[particleIndex].life = 5.0f;
		ParticlesContainer[particleIndex].pos = geysersLocations[locationIndex];
		locationIndex = (locationIndex + 1) % 5;

		float spread = 1.5f;
		glm::vec3 maindir = glm::vec3(0.0f, 10.0f, 0.0f);
		glm::vec3 randomdir = glm::vec3(
				(rand() % 2000 - 1000.0f) / 1000.0f,
				(rand() % 2000 - 1000.0f) / 1000.0f,
				(rand() % 2000 - 1000.0f) / 1000.0f);

		int particleColour = 200;
		ParticlesContainer[particleIndex].speed = maindir + randomdir * spread;
		ParticlesContainer[particleIndex].r = particleColour;
		ParticlesContainer[particleIndex].g = particleColour;
		ParticlesContainer[particleIndex].b = particleColour;
		ParticlesContainer[particleIndex].a = particleColour / 3;
		ParticlesContainer[particleIndex].size = 0.2f;
	}

	ParticlesCount = 0;
	for (int i = 0; i < MaxParticles; i++)
	{
		Particle &p = ParticlesContainer[i];

		if (p.life > 0.0f)
		{
			p.life -= delta;
			if (p.life > 0.0f)
			{

				p.speed += glm::vec3(0.0f, -1.5f, 0.0f) * (float)delta;
				p.pos += p.speed * (float)delta;
				p.cameradistance = glm::length(p.pos - cameraPos);

				g_particule_position_size_data[4 * ParticlesCount + 0] = p.pos.x;
				g_particule_position_size_data[4 * ParticlesCount + 1] = p.pos.y;
				g_particule_position_size_data[4 * ParticlesCount + 2] = p.pos.z;

				g_particule_position_size_data[4 * ParticlesCount + 3] = p.size;

				g_particule_color_data[4 * ParticlesCount + 0] = p.r;
				g_particule_color_data[4 * ParticlesCount + 1] = p.g;
				g_particule_color_data[4 * ParticlesCount + 2] = p.b;
				g_particule_color_data[4 * ParticlesCount + 3] = p.a;
			}
			else
			{
				p.cameradistance = -1.0f;
			}

			ParticlesCount++;
		}
	}

	SortParticles();
}

void updateParticles()
{
	glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
	glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, ParticlesCount * sizeof(GLfloat) * 4, g_particule_position_size_data);
	glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
	glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, ParticlesCount * sizeof(GLubyte) * 4, g_particule_color_data);
}

void bindParticles(glm::vec3 cameraSide, glm::vec3 cameraVertical, glm::mat4 perspectiveMatrix, glm::mat4 cameraMatrix, GLuint programParticles)
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glUseProgram(programParticles);

	GLuint CameraRight_worldspace_ID = glGetUniformLocation(programParticles, "CameraRight_worldspace");
	GLuint CameraUp_worldspace_ID = glGetUniformLocation(programParticles, "CameraUp_worldspace");
	GLuint ViewProjMatrixID = glGetUniformLocation(programParticles, "VP");

	GLuint TextureID = glGetUniformLocation(programParticles, "myTextureSampler");

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, particleTexture);
	glUniform1i(TextureID, 0);

	glUniform3f(CameraRight_worldspace_ID, cameraSide.x, cameraSide.y, cameraSide.z);
	glUniform3f(CameraUp_worldspace_ID, cameraVertical.x, cameraVertical.y, cameraVertical.z);

	glm::mat4 transformation = perspectiveMatrix * cameraMatrix;
	glUniformMatrix4fv(ViewProjMatrixID, 1, GL_FALSE, (float *)&transformation);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, billboard_vertex_buffer);
	glVertexAttribPointer(
			0,				
			3,				
			GL_FLOAT, 
			GL_FALSE, 
			0,				
			(void *)0
	);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
	glVertexAttribPointer(
			1,			
			4,				
			GL_FLOAT, 
			GL_FALSE,
			0,			
			(void *)0
	);

	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
	glVertexAttribPointer(
			2,
			4,
			GL_UNSIGNED_BYTE,
			GL_TRUE,				
			0,								
			(void *)0					
	);
}

void renderParticles()
{
	glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, ParticlesCount);
	glVertexAttribDivisor(0, 0);
	glVertexAttribDivisor(1, 1);
	glVertexAttribDivisor(2, 1);
	glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, ParticlesCount);
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
}

void deleteParticles()
{
	delete[] g_particule_position_size_data;
	glDeleteBuffers(1, &particles_color_buffer);
	glDeleteBuffers(1, &particles_position_buffer);
	glDeleteBuffers(1, &billboard_vertex_buffer);
}