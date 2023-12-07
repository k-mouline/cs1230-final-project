#ifndef TERRAINGENERATOR_H
#define TERRAINGENERATOR_H
#include "src/cube.h"
#include <vector>
#include <glm/glm.hpp>
#include <map>
#include "cube.h"

class TerrainGenerator
{
public:
    TerrainGenerator();
    std::map<std::pair<int, int>, std::vector<Cube*>> createTranslationMatricesForChunk(int chunkX, int chunkY);
    static float getOffset();
    static const int chunkSize = 10;
    static const int maxChunkHeight = 16;
    static const int chunkDepth  = 16;
    static const int offset = 1;
    glm::vec3 playerPosition;
    int renderDistance = 2; // Number of chunks to render in each direction from the player
    std::vector<Cube*> updatePlayerPosition(const glm::vec3& newPosition, std::vector<Cube*> oldCubes, bool isFirst);
    bool checkAndLoadChunks();
    const std::map<std::pair<int, int>, std::map<std::pair<int, int>, std::vector<Cube*>>>& getChunkMatrices() const;

    std::map<std::pair<int, int>, std::map<std::pair<int, int>, std::vector<Cube*>>> chunkMatrices1;
    std::map<std::pair<int, int>, std::map<std::pair<int, int>, std::vector<Cube*>>> cachedChunkMatrices2;

    std::vector<Cube*> cubesVector;

    // Takes in the camera position and gets the height of the terrain at that point.
    float getGroundHeight(glm::vec3 position);
};

#endif // TERRAINGENERATOR_H
