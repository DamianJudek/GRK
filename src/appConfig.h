#include <string>
#include <vector>
#include "glm.hpp"
#include <queue>

using namespace std;

//skybox
extern float skyboxVertices[108];
extern vector<std::string> faces;
extern unsigned int loadCubemap(std::vector<std::string> faces);

//particles
extern vector <glm::vec3> geysersLocations;

//camera
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
extern void keyboard(unsigned char key, int x, int y);
extern void mouse(int x, int y);

//light
extern glm::vec3 lightDir;

//coins
extern bool getCoin;

//bubbles
extern bool createBubble;
extern float timeOfLastBubbleCreation;

struct Bubble {
    float creationTime;
    glm::vec3 position;
    Bubble(float creationTime, glm::vec3 position) {
        this->creationTime = creationTime;
        this->position = position;
    }
};


extern std::vector<Bubble*> bubbles;
extern void makeBubble(float creationTime, glm::vec3 position);

//fish
extern std::vector<std::vector<glm::vec3>> paths;
extern std::vector<std::vector<glm::quat>> path_rots;

struct Fish {
    int p_id;
    int p_size;
    float t_offset;
    int model_id;

    Fish(int path, float offset) {
        p_id = path;
        p_size = paths[p_id].size();
        t_offset = offset;
        model_id = (int)(rand() % 4);
    }
};

extern std::vector<Fish*> fishe;

extern glm::mat4 animationMatrix(float time, Fish* cur_fish);
extern void initPaths(int path_amount,
    glm::vec2 path_radius,
    glm::vec2 placement_area_x,
    glm::vec2 placement_area_y,
    glm::vec2 placement_area_z,
    glm::vec2 rand_x_offset,
    glm::vec2 rand_y_offset,
    glm::vec2 rand_z_offset);
extern void initPathRots();
extern void initFish(int amount);
