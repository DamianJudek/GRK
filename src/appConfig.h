
#include <string>
#include <vector>
#include "glm.hpp"

extern float skyboxVertices[108];
extern std::vector<std::string> faces;
extern unsigned int loadCubemap(std::vector<std::string> faces);

extern float cameraAngle;
extern glm::vec3 cameraSide;
extern glm::vec3 cameraDir;
extern glm::vec3 cameraPos;
extern glm::mat4 cameraMatrix, perspectiveMatrix;
extern float old_x, old_y;
extern float delta_x, delta_y;
extern glm::quat rotationCamera;
extern glm::quat rotation_y;
extern glm::quat rotation_x;
extern float dy;
extern float dx;
extern glm::vec3 lightDir;
extern glm::mat4 createCameraMatrix();
extern void keyboard(unsigned char key, int x, int y);
extern void mouse(int x, int y);
