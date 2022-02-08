#include "glm.hpp"
#include "freeglut.h"

void initParticles();
void simulateParticles(glm::vec3 cameraPos);
void updateParticles();
void bindParticles(glm::vec3 cameraSide, glm::vec3 cameraVertical, glm::mat4 perspectiveMatrix, glm::mat4 cameraMatrix, GLuint programParticles);
void deleteParticles();
void renderParticles();