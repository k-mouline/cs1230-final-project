#ifndef TERRAINGENERATOR_H
#define TERRAINGENERATOR_H
#include <vector>
#include <glm/glm.hpp>


class TerrainGenerator
{
public:
    TerrainGenerator();
    static std::vector<glm::mat4> createTranslationMatrices();
    static float getOffset();

};

#endif // TERRAINGENERATOR_H
