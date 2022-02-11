#include <string>
#include <vector>
#include "glm.hpp"
#include <queue>
#include "Render_Utils.h"

using namespace std;

//other
struct RandomObject
{
    glm::vec3 pos;

    virtual void renderSelf() = 0;
};

// shaders
extern GLuint particlesShader;
extern GLuint colorShader;
extern GLuint textureShader;
extern GLuint skyboxShader;

// skybox
extern GLuint skyboxVAO, skyboxVBO;
extern float skyboxVertices[108];
extern vector<std::string> faces;
extern unsigned int loadCubemap(std::vector<std::string> faces);
extern unsigned int cubemapTexture;

// submarine
extern Core::RenderContext submarine;
extern GLuint submarineTextureId;

// particles
extern vector<glm::vec3> geysersLocations;

// camera
extern float cameraAngle;
extern glm::vec3 cameraSide;
extern glm::vec3 cameraDir;
extern glm::vec3 cameraPos;
extern glm::mat4 cameraMatrix, perspectiveMatrix;
extern float old_x, old_y;
extern float delta_x, delta_y;
extern glm::quat rotationCamera;
extern glm::vec3 cameraVertical;
extern glm::quat rotation_y;
extern glm::quat rotation_x;
extern float dy;
extern float dx;

//!!!
extern glm::mat4 createCameraMatrix();

// light
extern glm::vec3 lightDir;

// plants
extern Core::RenderContext flower_models[];
extern GLuint flowerTextureIds[];
extern glm::vec3 plantsBuffer[];
struct Plant : public RandomObject
{
    int model_id;

    Plant(glm::vec3 pos, int model_id)
    {
        this->pos = pos;
        this->model_id = model_id;
    }

    void renderSelf()
    {
    }
};

// coins
extern Core::RenderContext coin;
extern GLuint coinTextureId;
extern bool getCoin;
extern glm::vec4 coins[];
extern int numberOfCoins;
struct Coin : public RandomObject
{
    Coin(glm::vec3 pos)
    {
        this->pos = pos;
    }
    void renderSelf()
    {
    }
};

// bubbles
extern bool createBubble;
extern float timeOfLastBubbleCreation;
struct Bubble
{
    float creationTime;
    glm::vec3 position;
    Bubble(float creationTime, glm::vec3 position)
    {
        this->creationTime = creationTime;
        this->position = position;
    }
};
extern std::vector<Bubble *> bubbles;
extern void makeBubble(float creationTime, glm::vec3 position);
extern Core::RenderContext bubble;

// fish
extern Core::RenderContext fish_models[];
extern GLuint fishTextureId;
extern std::vector<std::vector<glm::vec3>> paths;
extern std::vector<std::vector<glm::quat>> path_rots;

struct Fish
{
    int p_id;
    int p_size;
    float t_offset;
    int model_id;

    Fish(int path, float offset)
    {
        p_id = path;
        p_size = paths[p_id].size();
        t_offset = offset;
        model_id = (int)(rand() % 4);
    }
};

extern std::vector<Fish *> fishe;
extern vector<glm::vec2> fishKeyframes;

extern glm::mat4 animationMatrix(float time, Fish *cur_fish);
extern void initPaths(int path_amount,
                      vector<glm::vec2> fishKeyframes);
extern void initPathRots();
extern void initFish(int amount);

// Terrain
extern vector<vector<vector<vector<glm::vec3>>>> _terrainChunks;
extern vector<vector<vector<vector<RandomObject *>>>> _objectChunks;
extern Core::RenderContext terrainCube;
extern GLuint terrainTextureId;
extern int TERRAIN_CHUNK_SIZE;
extern int TERRAIN_RENDER_DISTANCE;
extern float P_DIST_DETAIL;
extern float P_DIST_GENERAL;
extern float P_SCALE_DETAIL;
extern float P_SCALE_GENERAL;
extern float BASE_CUBE_SCALE;
extern float CHUNK_AREA;
extern glm::vec2 PLANTS_PER_CHUNK;
