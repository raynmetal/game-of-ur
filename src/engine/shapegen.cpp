#include <vector>

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "resource_database.hpp"
#include "vertex.hpp"
#include "mesh.hpp"
#include "shapegen.hpp"

std::shared_ptr<StaticMesh> generateSphereMesh(int nLatitude, int nMeridian);
std::shared_ptr<StaticMesh> generateRectangleMesh(float width=2.f, float height=2.f);

std::shared_ptr<IResource> StaticMeshSphereLatLong::createResource(const nlohmann::json& methodParameters) {
    return generateSphereMesh(
        methodParameters.at("nLatitudes").get<uint32_t>(),
        methodParameters.at("nMeridians").get<uint32_t>()
    );
}

std::shared_ptr<IResource> StaticMeshRectangleDimensions::createResource(const nlohmann::json& methodParameters) {
    return generateRectangleMesh(
        methodParameters.at("width").get<float>(),
        methodParameters.at("height").get<float>()
    );
}

std::shared_ptr<StaticMesh> generateSphereMesh(int nLatitude, int nMeridian)  {
    assert(nLatitude >= 1);
    assert(nMeridian >= 2);

    const int nVerticesPerLatitude { 2 * nMeridian };
    const int nVerticesTotal { 2 + nLatitude * nVerticesPerLatitude };

    const float angleVerticalDelta { 180.f / (1 + nLatitude) };
    const float angleHorizontalDelta{ 180.f / nMeridian };

    std::vector<BuiltinVertexData> vertices(nVerticesTotal);
    int currentIndex { 0 };
    for(int i{0}; i < 2 + nLatitude; ++i) {
        const float angleVertical { i * angleVerticalDelta };
        const int nPoints {
            i % (1 + nLatitude)? nVerticesPerLatitude: 1
        };
        for(int j{0}; j < nPoints; ++j) {
            const float angleHorizontal { j * angleHorizontalDelta };
            vertices[currentIndex].mPosition = glm::vec4(
                glm::sin(glm::radians(angleVertical)) * glm::sin(glm::radians(angleHorizontal)),
                glm::cos(glm::radians(angleVertical)),
                glm::sin(glm::radians(angleVertical)) * glm::cos(glm::radians(angleHorizontal)),
                1.f
            );
            vertices[currentIndex].mNormal = glm::vec4(glm::vec3(vertices[currentIndex].mPosition), 0.f);
            vertices[currentIndex].mTangent = glm::vec4(
                glm::sin(glm::radians(angleHorizontal+90.f)),
                0.f,
                glm::cos(glm::radians(angleHorizontal+90.f)),
                0.f
            );
            vertices[currentIndex].mColor = glm::vec4(1.f);
            ++currentIndex;
        }
    }

    //Generate elements, each set of 3 representing a triangle
    const int nTriangles {
        2*nVerticesPerLatitude * nLatitude
    };
    std::vector<GLuint> elements(3* static_cast<unsigned int>(nTriangles));
    int elementIndex {0};
    int previousBaseIndex {0};
    for(int i{1}; i < 2 + nLatitude; ++i) {
        const int nPoints {
            i%(1+nLatitude)? nVerticesPerLatitude: 1
        };
        const int nPointsPrevious {
            (i-1)%(1+nLatitude)? nVerticesPerLatitude: 1
        };
        const int currentBaseIndex {
            previousBaseIndex + nPointsPrevious
        };
        const int nJoiningFaces {std::max(nPoints, nPointsPrevious)};

        for(int j{0}; j < nJoiningFaces; ++j) {
            const int topleft { previousBaseIndex + (j % nPointsPrevious)};
            const int topright { previousBaseIndex + ((1+j) % nPointsPrevious)};
            const int bottomleft { currentBaseIndex + (j % nPoints) };
            const int bottomright  { currentBaseIndex + ((1+j) % nPoints)};

            if(bottomleft != bottomright) {
                elements[elementIndex++] = topleft;
                elements[elementIndex++] = bottomleft;
                elements[elementIndex++] = bottomright;
            }
            if(topleft != topright) {
                elements[elementIndex++] = topleft;
                elements[elementIndex++] = bottomright;
                elements[elementIndex++] = topright;
            }
        }
        previousBaseIndex = currentBaseIndex;
    }

    return std::make_shared<StaticMesh>(vertices, elements);
}

std::shared_ptr<StaticMesh> generateRectangleMesh(float width, float height) {
    assert(width > 0.f);
    assert(height > 0.f);

    std::vector<BuiltinVertexData> vertices {
        {
            .mPosition {-width/2.f, height/2.f, 0.f, 1.f},
            .mNormal{0.f, 0.f, 1.f, 0.f},
            .mTangent{1.f, 0.f, 0.f, 0.f},
            .mColor{1.f, 1.f, 1.f, 1.f},
            .mUV1{0.f, 1.f}
        },
        {
            .mPosition {width/2.f, height/2.f, 0.f, 1.f},
            .mNormal{0.f, 0.f, 1.f, 0.f},
            .mTangent{1.f, 0.f, 0.f, 0.f},
            .mColor{1.f, 1.f, 1.f, 1.f},
            .mUV1{1.f, 1.f}
        },
        {
            .mPosition {width/2.f, -height/2.f, 0.f, 1.f},
            .mNormal {0.f, 0.f, 1.f, 0.f},
            .mTangent {1.f, 0.f, 0.f, 0.f},
            .mColor {1.f, 1.f, 1.f, 1.f},
            .mUV1 {1.f, 0.f}
        },
        {
            .mPosition {-width/2.f, -height/2.f, 0.f, 1.f},
            .mNormal {0.f, 0.f, 1.f, 0.f},
            .mTangent {1.f, 0.f, 0.f, 0.f},
            .mColor {1.f, 1.f, 1.f, 1.f},
            .mUV1 {0.f, 0.f}
        },
    };
    std::vector<GLuint> elements {
        {0}, {2}, {1},
        {0}, {3}, {2}
    };

    return std::make_shared<StaticMesh>(vertices, elements);
}
