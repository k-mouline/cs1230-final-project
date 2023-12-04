#include "terraingenerator.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "FastNoiseLite.h"  // Include FastNoiseLite
#include <random>


std::vector<glm::mat4> TerrainGenerator::createTranslationMatrices() {
    std::vector<glm::mat4> matrices;
    float offset = 1.0f; // Spacing between cubes
    int maxHeight = 16; // Maximum height (for terrain generation)
    int chunkHeight = 16; // Total height of the chunk

    FastNoiseLite noise;
    noise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    noise.SetFrequency(0.1f); // Lower frequency for more gradual changes

    for (int x = 0; x < 16; ++x) {
        for (int y = 0; y < 16; ++y) {
            float heightValue = noise.GetNoise((float)x, (float)y);
            int terrainHeight = static_cast<int>((heightValue + 1) * 0.5 * maxHeight); // Scale and quantize height

            for (int z = 0; z < chunkHeight; ++z) {
                if (z <= terrainHeight) {
                    // Fill terrain up to terrainHeight

                    // Subtract from maxHeight to generate below the origin
                    glm::mat4 translation = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, -maxHeight + z * offset));
                    matrices.push_back(translation);
                }
                // Additional logic for above the terrain can be added here
            }
        }
    }

    return matrices;
}


