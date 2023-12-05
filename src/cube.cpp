#include "cube.h"

Cube::Cube(glm::mat4 ctm){
    transformationMatrix = ctm;
}


std::vector<float> Cube::initialize(int param1, int param2, float texCoord1Top, float texCoord2Top){
        // code that creates vertex data
        m_vertexData = std::vector<float>();
        m_param1 = param1;
        texCoord1 = texCoord1Top;
        texCoord2 = texCoord2Top;

        setVertexData();



        return m_vertexData;
    }

    // Inserts a glm::vec3 into a vector of floats.
    // This will come in handy if you want to take advantage of vectors to build your shape!
    void Cube::insertVec3(std::vector<float> &data, glm::vec3 v) {
        data.push_back(v.x);
        data.push_back(v.y);
        data.push_back(v.z);
    }

    void Cube::setVAO(GLuint &vao){
        VAO = vao;
    }

    GLuint Cube::getVAO(){
        return VAO;
    }

    glm::mat4 Cube::getCTM(){
        return transformationMatrix;
    }


    std::vector<float> Cube::getVertexData(){
        return m_vertexData;
    }


    void Cube::makeTile(glm::vec3 topLeft,
                        glm::vec3 topRight,
                        glm::vec3 bottomLeft,
                        glm::vec3 bottomRight) {


        float textureOffset = 1.0f/16.0f;
        // Calculate the normal
        glm::vec3 normal = glm::normalize(glm::cross(bottomLeft - topLeft, topRight - topLeft));

        // First Triangle (counterclockwise order)
        insertVec3(m_vertexData, topRight);
        insertVec3(m_vertexData, normal);
        m_vertexData.push_back(texCoord1+textureOffset);
        m_vertexData.push_back(texCoord2);


        insertVec3(m_vertexData, topLeft);
        insertVec3(m_vertexData, normal);
        m_vertexData.push_back(texCoord1);
        m_vertexData.push_back(texCoord2);

        insertVec3(m_vertexData, bottomLeft);
        insertVec3(m_vertexData, normal);
        m_vertexData.push_back(texCoord1);
        m_vertexData.push_back(texCoord2+textureOffset);

        // Second Triangle (counterclockwise order)
        insertVec3(m_vertexData, topRight);
        insertVec3(m_vertexData, normal);
        m_vertexData.push_back(texCoord1+textureOffset);
        m_vertexData.push_back(texCoord2);

        insertVec3(m_vertexData, bottomLeft);
        insertVec3(m_vertexData, normal);
        m_vertexData.push_back(texCoord1);
        m_vertexData.push_back(texCoord2+textureOffset);

        insertVec3(m_vertexData, bottomRight);
        insertVec3(m_vertexData, normal);
        m_vertexData.push_back(texCoord1+textureOffset);
        m_vertexData.push_back(texCoord2+textureOffset);




    }

    void Cube::makeFace(glm::vec3 topLeft,
                        glm::vec3 topRight,
                        glm::vec3 bottomLeft,
                        glm::vec3 bottomRight) {


        // Task 3: create a single side of the cube out of the 4
        //         given points and makeTile()
        // Note: think about how param 1 affects the number of triangles on
        //       the face of the cube

        // Use makeTile to create two triangles for the face
        makeTile(topLeft, topRight, bottomLeft, bottomRight);
        m_param1 = 1.0f;
        // calculate step size
        float step = 1.0f/m_param1;

        for (int i = 0; i < m_param1; i++) {
            for (int j = 0; j < m_param1; j++) {

                // calculate corners of vertices
                glm::vec3 quadTopLeft = topLeft + i*step*(topRight-topLeft)+j*step*(bottomLeft-topLeft);
                glm::vec3 quadTopRight = topLeft + (i+1)*step*(topRight-topLeft)+j*step*(bottomLeft-topLeft);
                glm::vec3 quadBottomLeft = topLeft + i*step*(topRight-topLeft)+(j+1)*step*(bottomLeft-topLeft);
                glm::vec3 quadBottomRight = topLeft + (i+1)*step*(topRight-topLeft)+(j+1)*step*(bottomLeft-topLeft);

                // call maketile with vertices
                makeTile(quadTopLeft, quadTopRight, quadBottomLeft, quadBottomRight);
            }
        }


    }

    void Cube::setVertexData() {

        makeFace(glm::vec3(-0.5f,  0.5f, 0.5f),
                 glm::vec3( 0.5f,  0.5f, 0.5f),
                 glm::vec3(-0.5f, -0.5f, 0.5f),
                 glm::vec3( 0.5f, -0.5f, 0.5f));

        makeFace(glm::vec3( 0.5f,  0.5f, -0.5f),
                 glm::vec3(-0.5f,  0.5f, -0.5f),
                 glm::vec3( 0.5f, -0.5f, -0.5f),
                 glm::vec3(-0.5f, -0.5f, -0.5f));

        makeFace(glm::vec3(-0.5f,  0.5f, -0.5f),
                 glm::vec3( 0.5f,  0.5f, -0.5f),
                 glm::vec3(-0.5f,  0.5f, 0.5f),
                 glm::vec3( 0.5f,  0.5f, 0.5f));

        makeFace(glm::vec3(0.5f, 0.5f, 0.5f),
                 glm::vec3(0.5f, 0.5f, -0.5f),
                 glm::vec3(0.5f, -0.5f, 0.5f),
                 glm::vec3(0.5f, -0.5f, -0.5f));

        makeFace(glm::vec3(-0.5f,  -0.5f, 0.5f),
                 glm::vec3(0.5f, -0.5f, 0.5f),
                 glm::vec3(-0.5f,  -0.5f, -0.5f),
                 glm::vec3(0.5f, -0.5f, -0.5f));

        makeFace(glm::vec3(-0.5f,  0.5f, -0.5f),
                 glm::vec3(-0.5f, 0.5f, 0.5f),
                 glm::vec3(-0.5f,  -0.5f, -0.5f),
                 glm::vec3(-0.5f, -0.5f, 0.5f));
    }

