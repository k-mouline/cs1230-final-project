#include "terraingenerator.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "FastNoiseLite.h"  // Include FastNoiseLite
#include <random>

TerrainGenerator::TerrainGenerator(){


}

// method takes in the current chunk location (using ints) and using these as offsets
std::vector<glm::mat4> TerrainGenerator::createTranslationMatricesForChunk(int chunkX, int chunkY) {
    std::vector<glm::mat4> matrices;

    // offset instance variable is 1 right now so doesn't actually do anything but I think if we want to make mountains/more complex terrain
    // it would be used in this method


    // use FastNoiseLite library (TODO: replace with our own perlin noise)
    FastNoiseLite noise;
    noise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    noise.SetFrequency(0.05f);

    // want to center the chunk around the players current location
    float centerXOffset = (chunkSize * offset) / 2.0f;
    float centerYOffset = (chunkSize * offset) / 2.0f;
    float centerZOffset = (maxChunkHeight * offset) / 2.0f;

    // based on what chunk we are in we need to offset from the origin using the values of chunkX and chunkY
    float worldXOffset = chunkX * chunkSize * offset;
    float worldYOffset = chunkY * chunkSize * offset;

    // iterate through the chunks
    for (int x = 0; x < chunkSize; x++) {
        for (int y = 0; y < chunkSize; y++) {

            // use noise to get the height of terrain
            float heightValue = noise.GetNoise((float)x + chunkX * chunkSize, (float)y + chunkY * chunkSize);
            int terrainHeight = static_cast<int>((heightValue + 1) * 0.5 * maxChunkHeight);

            // iterate through the depth of the terrain and create blocks up until the terrain height.
            for (int z = 0; z < chunkDepth; z++) {
                if (z <= terrainHeight) {
                    // use the calculated offsets from earlier to form the translation matrix of the given cube.
                    // z is offset by negative maxChunkHeight so that the blocks form beneath us.
                    glm::mat4 translation = glm::translate(glm::mat4(1.0f), glm::vec3(
                                                                                x * offset - centerXOffset + worldXOffset,
                                                                                y * offset - centerYOffset + worldYOffset,
                                                                                -maxChunkHeight + z * offset - centerZOffset));
                    matrices.push_back(translation);
                }
            }
        }
    }
    return matrices;
}

// based on the current camera position and render distance loads and unloads chunks
void TerrainGenerator::checkAndLoadChunks() {
    int currentChunkX = static_cast<int>(playerPosition.x / (chunkSize * offset));
    int currentChunkY = static_cast<int>(playerPosition.y / (chunkSize * offset));

    // Create a list to keep track of chunks to unload
    std::vector<std::pair<int, int>> chunksToUnload;

    // Iterate through all loaded chunks
    for (auto& chunk : chunkMatrices) {
        int x = chunk.first.first;
        int y = chunk.first.second;

        // Check if the chunk is outside the render distance
        if (x < currentChunkX - renderDistance || x > currentChunkX + renderDistance ||
            y < currentChunkY - renderDistance || y > currentChunkY + renderDistance) {
            // Mark this chunk for removal
            chunksToUnload.push_back(chunk.first);
        }
    }

    // Remove chunks that are outside the render distance
    for (auto& chunk : chunksToUnload) {
        chunkMatrices.erase(chunk);
    }

    // Load new chunks within the render distance
    for (int x = currentChunkX - renderDistance; x <= currentChunkX + renderDistance; ++x) {
        for (int y = currentChunkY - renderDistance; y <= currentChunkY + renderDistance; ++y) {
            if (chunkMatrices.find({x, y}) == chunkMatrices.end()) {
                // Chunk not loaded, generate new chunk
                chunkMatrices[{x, y}] = createTranslationMatricesForChunk(x, y);
            }
        }
    }
}



// takes in player position (cameraPosition instance variable) and updates chunks based on that
void TerrainGenerator::updatePlayerPosition(const glm::vec3& newPosition) {
    playerPosition = newPosition;
    checkAndLoadChunks();
}

// getter method for chunk data.
const std::map<std::pair<int, int>, std::vector<glm::mat4>>& TerrainGenerator::getChunkMatrices() const {
    return chunkMatrices;
}






