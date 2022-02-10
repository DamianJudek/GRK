#include <string>
#include <vector>
#include "Render_Utils.h"

using namespace std;
extern void initSkybox();
extern void initPlants();
extern void initCoins();

extern float getHeightAtPoint(float x, float y);
extern void makeChunk(int x, int y);
extern vector<glm::vec3>& getChunk(int x, int y);
extern glm::vec2 findClosestChunk(glm::vec3 pos);

extern void drawObjectColor(Core::RenderContext context, glm::mat4 modelMatrix, glm::vec3 color);
extern void drawObjectTexture(Core::RenderContext context, glm::mat4 modelMatrix, GLuint textureId);
extern void loadModelToContext(std::string path, Core::RenderContext& context);

extern void drawSkybox();
extern void drawSubmarine();
extern void drawPlants();
extern void drawParticles();
extern void drawFish(float current_time);
extern void drawCoins();
extern void drawBubbles(float current_time);
extern void drawTerrain();

extern void keyboard(unsigned char key, int x, int y);
extern void mouse(int x, int y);