#include "appConfig.h"
#include "glew.h"
#include "glm.hpp"
#include "gtx/matrix_decompose.hpp"
#include "stb_image.h"
#include <iostream>
#include <string>
#include <vector>

using namespace std;
int index = 0;

float skyboxSize = 300.0f;

float skyboxVertices[] = {
    -skyboxSize, skyboxSize, -skyboxSize,
    -skyboxSize, -skyboxSize, -skyboxSize,
    skyboxSize, -skyboxSize, -skyboxSize,
    skyboxSize, -skyboxSize, -skyboxSize,
    skyboxSize, skyboxSize, -skyboxSize,
    -skyboxSize, skyboxSize, -skyboxSize,

    -skyboxSize, -skyboxSize, skyboxSize,
    -skyboxSize, -skyboxSize, -skyboxSize,
    -skyboxSize, skyboxSize, -skyboxSize,
    -skyboxSize, skyboxSize, -skyboxSize,
    -skyboxSize, skyboxSize, skyboxSize,
    -skyboxSize, -skyboxSize, skyboxSize,

    skyboxSize, -skyboxSize, -skyboxSize,
    skyboxSize, -skyboxSize, skyboxSize,
    skyboxSize, skyboxSize, skyboxSize,
    skyboxSize, skyboxSize, skyboxSize,
    skyboxSize, skyboxSize, -skyboxSize,
    skyboxSize, -skyboxSize, -skyboxSize,

    -skyboxSize, -skyboxSize, skyboxSize,
    -skyboxSize, skyboxSize, skyboxSize,
    skyboxSize, skyboxSize, skyboxSize,
    skyboxSize, skyboxSize, skyboxSize,
    skyboxSize, -skyboxSize, skyboxSize,
    -skyboxSize, -skyboxSize, skyboxSize,

    -skyboxSize, skyboxSize, -skyboxSize,
    skyboxSize, skyboxSize, -skyboxSize,
    skyboxSize, skyboxSize, skyboxSize,
    skyboxSize, skyboxSize, skyboxSize,
    -skyboxSize, skyboxSize, skyboxSize,
    -skyboxSize, skyboxSize, -skyboxSize,

    -skyboxSize, -skyboxSize, -skyboxSize,
    -skyboxSize, -skyboxSize, skyboxSize,
    skyboxSize, -skyboxSize, -skyboxSize,
    skyboxSize, -skyboxSize, -skyboxSize,
    -skyboxSize, -skyboxSize, skyboxSize,
    skyboxSize, -skyboxSize, skyboxSize};

std::vector<std::string> faces = {
    "./textures/skybox/right.jpg",
    "./textures/skybox/left.jpg",
    "./textures/skybox/top.jpg",
    "./textures/skybox/bottom.jpg",
    "./textures/skybox/front.jpg",
    "./textures/skybox/back.jpg"};

unsigned int loadCubemap(std::vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                         0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            cout << "Cubemap tex failed to load at path: " << faces[i] << endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

float cameraAngle = 0;
glm::vec3 cameraSide;
glm::vec3 cameraDir;
glm::vec3 cameraPos = glm::vec3(0, 0, 0);
glm::mat4 cameraMatrix, perspectiveMatrix;
float old_x, old_y = -1;
float delta_x, delta_y = 0;
glm::quat rotationCamera = glm::quat(1, 0, 0, 0);
glm::quat rotation_y = glm::normalize(glm::angleAxis(209 * 0.03f, glm::vec3(1, 0, 0)));
glm::quat rotation_x = glm::normalize(glm::angleAxis(307 * 0.03f, glm::vec3(0, 1, 0)));
float dy = 0;
float dx = 0;

glm::vec3 lightDir = glm::normalize(glm::vec3(1, -100, 1));

glm::mat4 createCameraMatrix()
{
    auto rot_y = glm::angleAxis(delta_y * 0.03f, glm::vec3(1, 0, 0));
    auto rot_x = glm::angleAxis(delta_x * 0.03f, glm::vec3(0, 1, 0));

    dy += delta_y;
    dx += delta_x;
    delta_x = 0;
    delta_y = 0;
    rotation_x = glm::normalize(rot_x * rotation_x);
    rotation_y = glm::normalize(rot_y * rotation_y);
    rotationCamera = glm::normalize(rotation_y * rotation_x);

    auto inverse_rot = glm::inverse(rotationCamera);

    cameraDir = inverse_rot * glm::vec3(0, 0, -1);
    glm::vec3 up = glm::vec3(0, 1, 0);
    cameraSide = inverse_rot * glm::vec3(1, 0, 0);
    glm::mat4 cameraTranslation;
    cameraTranslation[3] = glm::vec4(-cameraPos, 1.0f);

    return glm::mat4_cast(rotationCamera) * cameraTranslation;
}

void keyboard(unsigned char key, int x, int y)
{
    float angleSpeed = 0.5f;
    float moveSpeed = 2.f;
    switch (key)
    {
    case 'z': cameraAngle -= angleSpeed; break;
    case 'x': cameraAngle += angleSpeed; break;
    case 'w': cameraPos += cameraDir * moveSpeed; break;
    case 's': cameraPos -= cameraDir * moveSpeed; break;
    case 'd': cameraPos += cameraSide * moveSpeed; break;
    case 'a': cameraPos -= cameraSide * moveSpeed; break;
    case '0': cameraPos = glm::vec3(0, 3, 0); index = 0; break;
    case 'r': cameraPos = glm::vec3(0, 0, 1); break;
    }
}


void mouse(int x, int y)
{
    if (old_x >= 0) {
        delta_x = x - old_x;
        delta_y = y - old_y;
    }
    old_x = x;
    old_y = y;
}

