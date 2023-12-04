#ifndef TERRAINGENERATOR_H
#define TERRAINGENERATOR_H
#include <vector>
#include <glm/glm.hpp>
#include <map>



class TerrainGenerator
{
public:
    TerrainGenerator();
    std::vector<glm::mat4> createTranslationMatricesForChunk(int chunkX, int chunkY);
    static float getOffset();
    static const int chunkSize = 10;
    static const int maxChunkHeight = 16;
    static const int chunkDepth  = 16;
    static const int offset = 1;
    glm::vec3 playerPosition;
    int renderDistance = 3; // Number of chunks to render in each direction from the player
    std::map<std::pair<int, int>, std::vector<glm::mat4>> chunkMatrices;
    void updatePlayerPosition(const glm::vec3& newPosition);
    void checkAndLoadChunks();
    const std::map<std::pair<int, int>, std::vector<glm::mat4>>& getChunkMatrices() const;



};

#endif // TERRAINGENERATOR_H
