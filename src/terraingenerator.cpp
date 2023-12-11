#include "terraingenerator.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "FastNoiseLite.h"  // Include FastNoiseLite
#include "src/cube.h"
#include <random>
#include <iostream>

TerrainGenerator::TerrainGenerator(){
}

// Add this method to TerrainGenerator class
float TerrainGenerator::getFractalNoise(FastNoiseLite noise, float x, float y, int octaves, float persistence) {
    float total = 0.0f;
    float frequency = 1.0f;
    float amplitude = 1.0f;
    float maxValue = 0.0f;  // Used for normalizing result

    for(int i = 0; i < octaves; i++) {
        total += noise.GetNoise(x * frequency, y * frequency) * amplitude;

        maxValue += amplitude;
        amplitude *= persistence;
        frequency *= 2;
    }

    return total / maxValue;
}


void generateTree(int baseX, int baseY, int baseZ, std::map<std::pair<int,int>, std::vector<Cube*>>& matricesCubes, int originalX, int originalY) {
    int treeHeight = rand() % 5 + 4; // Random tree height between 4 and 8
    int leafRadius = 2; // Radius of the leaves around the top

    // make trunk TODO: incorporate textures
    for (int z = baseZ; z < baseZ + treeHeight; ++z) {
        glm::mat4 translation = glm::translate(glm::mat4(1.0f), glm::vec3(baseX, baseY, z));
        Cube* newCube = new Cube(translation);
        newCube->setID(2);
        matricesCubes[{originalX, originalY}].push_back(newCube);
    }

    // generate leaves
    for (int x = -leafRadius; x <= leafRadius; ++x) {
        for (int y = -leafRadius; y <= leafRadius; ++y) {
            for (int z = treeHeight - leafRadius; z <= treeHeight; ++z) {
                if (x * x + y * y + (z - treeHeight + leafRadius) * (z - treeHeight + leafRadius) <= leafRadius * leafRadius) {
                    glm::mat4 translation = glm::translate(glm::mat4(1.0f), glm::vec3(baseX + x, baseY + y, baseZ + z));
                    Cube* newCube = new Cube(translation);
                    newCube->setID(3);
                    matricesCubes[{originalX + x, originalY + y}].push_back(newCube);
                }
            }
        }
    }
}


// method takes in the current chunk location (using ints) and using these as offsets
std::map<std::pair<int, int>, std::vector<Cube*>> TerrainGenerator::createTranslationMatricesForChunk(int chunkX, int chunkY) {
    //std::map<std::pair<int, int>, std::vector<glm::mat4>> matrices;

    std::map<std::pair<int,int>, std::vector<Cube*>> matricesCubes;

    // use FastNoiseLite library (TODO: replace with our own perlin noise)
    FastNoiseLite noise;
    noise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);

    noise.SetFrequency(0.03f); // can change this to get more mountainous terrain

    // want to center the chunk around the players current location
    float centerXOffset = (chunkSize * offset) / 2.0f;
    float centerYOffset = (chunkSize * offset) / 2.0f;
    float centerZOffset = (maxChunkHeight * offset) / 2.0f;

    // based on what chunk we are in we need to offset from the origin using the values of chunkX and chunkY
    float worldXOffset = chunkX * chunkSize * offset;
    float worldYOffset = chunkY * chunkSize * offset;

    std::default_random_engine generator;
    std::uniform_real_distribution<float> distribution(0.0, 1.0);

    const int octaves = 8;
    const float persistence = 0.5f;

    // iterate through the block in the chunk
    for (int x = 0; x < chunkSize; x++) {
        for (int y = 0; y < chunkSize; y++) {

//            // use noise to get the height of terrain
//            float heightValue = getFractalNoise(noise, (float)x + chunkX * chunkSize, (float)y + chunkY * chunkSize, octaves, persistence);
//            int terrainHeight = static_cast<int>((heightValue + 1) * 0.5 * maxChunkHeight);


            float heightValue = noise.GetNoise((float)x + chunkX * chunkSize, (float)y + chunkY * chunkSize);
            int terrainHeight = static_cast<int>((heightValue + 1) * 0.5 * maxChunkHeight);



            // iterate through the depth of the terrain and create blocks up until the terrain height.
            for (int z = 0; z < chunkDepth; z++) {
                glm::mat4 translation = glm::translate(glm::mat4(1.0f), glm::vec3(
                                                                            x * offset - centerXOffset + worldXOffset,
                                                                            y * offset - centerYOffset + worldYOffset,
                                                                            -maxChunkHeight + z * offset - centerZOffset));

                if (z < terrainHeight) {
                    // use the calculated offsets from earlier to form the translation matrix of the given cube.
                    // z is offset by negative maxChunkHeight so that the blocks form beneath us.
                    // matrices[{x, y}].push_back(translation);
                    matricesCubes[{x,y}].push_back(new Cube(translation));
                }

                // Add a layer of water.
                if (z == chunkDepth - 10) {
                    if (z > terrainHeight) {
                        Cube *cube = new Cube(translation);
                        cube->setID(5);
                        matricesCubes[{x,y}].push_back(cube);
                    }

                }

                if(z == terrainHeight){
                    // generate random probability of creating a tree and calculate the x,y, and z values for where the tree exists
                    float chance = distribution(generator);
                    float currHeight = -maxChunkHeight + z * offset - centerZOffset;
                    float currX =  x * offset - centerXOffset + worldXOffset;
                    float currY = y * offset - centerYOffset + worldYOffset;

                    // if height and probability match characteristics generate tree
                    if(chance < treeProbability && currHeight > treeHeight){
                        generateTree(currX, currY, currHeight, matricesCubes,x,y); // Generate the tree
                    }
                    matricesCubes[{x,y}].push_back(new Cube(translation));
                }
            }

        }
    }

    return matricesCubes;
}

// based on the current camera position and render distance loads and unloads chunks
bool TerrainGenerator::checkAndLoadChunks() {
    // Adjust position for the fact that the original chunk is centered on the player.
    float adjustedPositionX = playerPosition.x + (playerPosition.x / abs(playerPosition.x)) * (float) chunkSize / 2.f;
    float adjustedPositionY = playerPosition.y + (playerPosition.y / abs(playerPosition.y)) * (float) chunkSize / 2.f;

    // Index of the current chunk that the player is in.
    int currentChunkX = static_cast<int>(adjustedPositionX / (chunkSize * offset));
    int currentChunkY = static_cast<int>(adjustedPositionY / (chunkSize * offset));

    // Create a list to keep track of chunks to unload
    std::vector<std::pair<int, int>> chunksToUnload;

    // Iterate through all loaded chunks
    for (auto& chunk : chunkMatrices1) {
        int x = chunk.first.first;
        int y = chunk.first.second;

        // Check if the chunk is outside the render distance
        if (x < currentChunkX - renderDistance || x > currentChunkX + renderDistance ||
            y < currentChunkY - renderDistance || y > currentChunkY + renderDistance) {
            // Mark this chunk for removal
            chunksToUnload.push_back(chunk.first);
        }
    }

    for (auto& chunk : chunksToUnload) {
        cachedChunkMatrices2[chunk] = chunkMatrices1[chunk];
        chunkMatrices1.erase(chunk);
    }

    // Load new chunks within the render distance
    for (int x = currentChunkX - renderDistance; x <= currentChunkX + renderDistance; ++x) {
        for (int y = currentChunkY - renderDistance; y <= currentChunkY + renderDistance; ++y) {
            std::pair<int, int> chunkKey = {x, y};
            if (chunkMatrices1.find(chunkKey) == chunkMatrices1.end()) {
                // Check cache first
                if (cachedChunkMatrices2.find(chunkKey) != cachedChunkMatrices2.end()) {
                    // Load from cache
                    chunkMatrices1[chunkKey] = cachedChunkMatrices2[chunkKey];
                    cachedChunkMatrices2.erase(chunkKey);

                } else {
                    // Generate new chunk
                    chunkMatrices1[chunkKey] = createTranslationMatricesForChunk(x, y);
                }
            }
        }
    }

    return !chunksToUnload.empty();

}

// takes in player position (cameraPosition instance variable) and updates chunks based on that
std::vector<Cube*> TerrainGenerator::updatePlayerPosition(const glm::vec3& newPosition, std::vector<Cube*> oldCubes, bool isFirst) {
    playerPosition = newPosition;

    std::vector<Cube*> cubesVector = oldCubes;
    // should return true or false
    if(checkAndLoadChunks() || isFirst){
        cubesVector.clear();
        for (const auto& chunkEntry : chunkMatrices1) {
            const auto& chunkMatrix = chunkEntry.second;

            for (const auto& blockColumn : chunkMatrix) {
                const auto& columnMatrices = blockColumn.second;

                for (Cube* cube : columnMatrices) {
                    cubesVector.push_back(cube);
                }
            }
        }
    }
    return cubesVector;
}

// finds the z value of the block underneath the camera position passed in.
float TerrainGenerator::getGroundHeight(glm::vec3 position) {
    // Adjust position for the fact that the original chunk is centered on the player.
    float adjustedPositionX = position.x + (position.x / abs(position.x)) * (float) chunkSize / 2.f;
    float adjustedPositionY = position.y + (position.y / abs(position.y)) * (float) chunkSize / 2.f;

    // Index of the current chunk that the player is in.
    int currentChunkX = static_cast<int>(adjustedPositionX / (chunkSize * offset));
    int currentChunkY = static_cast<int>(adjustedPositionY / (chunkSize * offset));

    // Value of X and Y, anywhere from 0 -> chunkSize, within the chunk.
    int chunkX = (int) position.x - chunkSize * currentChunkX + chunkSize / 2;
    int chunkY = (int) position.y - chunkSize * currentChunkY + chunkSize / 2;

    // Offset by -1 if it's in a negative chunk so value starts at 0.
    chunkX = (currentChunkX < 0) ? chunkX - 1 : chunkX;
    chunkY = (currentChunkY < 0) ? chunkY - 1 : chunkY;

    // Get the terrain height by using the size of the vector storing the blocks in a given column.
    int terrainHeight = chunkMatrices1[{currentChunkX, currentChunkY}][{chunkX, chunkY}].size();

    // This calculation is taken from the createTranslationMatricesForChunk function above.
    float finalZ = (float) (-maxChunkHeight + terrainHeight * offset - (maxChunkHeight * offset) / 2.0f + 1.5f);
    return finalZ;
}

// getter method for chunk data.
const std::map<std::pair<int, int>, std::map<std::pair<int, int>, std::vector<Cube*>>>& TerrainGenerator::getChunkMatrices() const {
    return chunkMatrices1;
}
