//#ifndef Fishes
//#define Fishes

#include "appConfig.h"
#include "glew.h"
#include "glm.hpp"
#include "gtx/matrix_decompose.hpp"
#include "stb_image.h"
#include <iostream>
#include <string>
#include <vector>

//fishe
#include <random>
#include <gtx/quaternion.hpp>
#include <ext.hpp>



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

glm::vec3 lightDir = glm::normalize(glm::vec3(1, 1, 1));

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

// random
random_device rd;
mt19937 gen(rd());
uniform_real_distribution<> dist(0.0f, 1.0f);

// FISHY STUFF
std::vector<std::vector<glm::vec3>> paths{};
std::vector<std::vector<glm::quat>> path_rots{};


std::vector<Fish*> fishe;

glm::mat4 animationMatrix(float time, Fish* cur_fish) {
    std::vector<glm::vec3>& cur_path = paths[cur_fish->p_id];

    float speed = 1.;
    time = time * speed;
    std::vector<float> distances;
    float timeStep = 0;
    for (int i = 0; i < cur_fish->p_size; i++) {
        float local_distance = (cur_path[i] - cur_path[(i + 1) % cur_fish->p_size]).length();
        timeStep += local_distance;
        distances.push_back(local_distance);
    }
    time = fmod(time, timeStep);

    //index of first keyPoint
    int index = 0;

    //increment until at the current keyPoint
    while (distances[index] <= time) {
        time = time - distances[index];
        index += 1;
    }

    //t coefficient between 0 and 1 for interpolation
    float t = time / distances[index];

    // position interpolation
    glm::vec3 pos = glm::catmullRom(
        cur_path[(index - 1) % (cur_fish->p_size)],
        cur_path[(index) % (cur_fish->p_size)],
        cur_path[(index + 1) % (cur_fish->p_size)],
        cur_path[(index + 2) % (cur_fish->p_size)], 
        t
    );


    // rotation interpolation

    glm::quat rq[4];
    for (int i = 0; i < 4; i++) {
        rq[i] = path_rots[cur_fish->p_id][(index - 1 + i) % cur_fish->p_size];
    }

    auto a1 = rq[1] * glm::exp((glm::log(glm::inverse(rq[1]) * rq[0]) + glm::log(glm::inverse(rq[1]) * rq[2])) / (-4.f));
    auto a2 = rq[2] * glm::exp((glm::log(glm::inverse(rq[2]) * rq[1]) + glm::log(glm::inverse(rq[2]) * rq[3])) / (-4.f));

    auto animationRotation = glm::squad(rq[1], rq[2], a1, a2, t);
    //auto animationRotation = glm::quat(1, 0, 0, 0);

    glm::mat4 result = glm::translate(pos) * glm::mat4_cast(animationRotation);


    return result;
}

void initPaths(int path_amount,
    glm::vec2 path_radius = glm::vec2(40.0f, 60.0f),
    glm::vec2 placement_area_x = glm::vec2(40.0f, 60.0f),
    glm::vec2 placement_area_y = glm::vec2(40.0f, 60.0f),
    glm::vec2 placement_area_z = glm::vec2(10.0f, 25.0f),
    glm::vec2 rand_x_offset = glm::vec2(-15.0f, 15.0f),
    glm::vec2 rand_y_offset = glm::vec2(-15.0f, 15.0f),
    glm::vec2 rand_z_offset = glm::vec2(-5.0f, 5.0f)
) {
    for (int path_id = 0; path_id < path_amount; path_id++) {
        paths.push_back({});

        //radius
        uniform_real_distribution<> distr(path_radius.x, path_radius.y);
        float crx = distr(gen);
        float cry = distr(gen);

        //center coords
        distr = uniform_real_distribution<>(placement_area_x.x, placement_area_x.y);
        float cx = distr(gen);
        distr = uniform_real_distribution<>(placement_area_y.x, placement_area_y.y);
        float cy = distr(gen);
        distr = uniform_real_distribution<>(placement_area_z.x, placement_area_z.y);
        float cz = distr(gen);

        std::vector<float> coordvec;

        distr = uniform_real_distribution<>(rand_x_offset.x, rand_x_offset.y);
        for (int i = 0; i < 12; i++) coordvec.push_back(cx + cos(i * 30 * 3.14 / 180.0) * crx + distr(gen));

        distr = uniform_real_distribution<>(rand_z_offset.x, rand_z_offset.y);
        for (int i = 0; i < 12; i++) coordvec.push_back(cz + distr(gen));

        distr = uniform_real_distribution<>(rand_y_offset.x, rand_y_offset.y);
        for (int i = 0; i < 12; i++) coordvec.push_back(cy + sin(i * 30 * 3.14 / 180.0) * cry + distr(gen));


        for (int i = 0; i < 12; i++) paths[paths.size() - 1].push_back(glm::vec3(coordvec[i], coordvec[i + 12], coordvec[i + 24]));
    }


}

void initPathRots() {
    for (int i = 0; i < paths.size(); i++) {
        glm::vec3 oldDirection = glm::vec3(0, 0, 1);
        glm::quat oldRotationCamera = glm::quat(1, 0, 0, 0);
        path_rots.push_back({});
        for (int j = 0; j < paths[i].size(); j++) {
            glm::vec3 newDir = paths[i][(j + 1) % paths[i].size()] - paths[i][(j) % paths[i].size()];
            rotationCamera = glm::normalize(glm::rotationCamera(glm::normalize(oldDirection), glm::normalize(newDir)) * oldRotationCamera);
            path_rots[i].push_back(rotationCamera);
            oldDirection = newDir;
            oldRotationCamera = rotationCamera;
        }
    }
}

void initFish(int amount) {
    for (int i = 0; i < amount; i++) fishe.push_back(new Fish(rand() % paths.size(), i * dist(gen)));
}
// FISHY STUFF END

// end fishe

//#endif

