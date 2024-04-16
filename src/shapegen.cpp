#include <vector>

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "mesh.hpp"
#include "shapegen.hpp"

Mesh generateSphereMesh(int nLatitude, int nMeridian)  {
    if(nLatitude < 1) nLatitude = 1; // at least one latitude, the equator
    if(nMeridian < 2) nMeridian = 2; // at least 2 meridians, 90 deg apart
    const int nVerticesPerLatitude { 2 * nMeridian };
    const int nVerticesTotal { 2 + nLatitude * nVerticesPerLatitude };

    const float angleVerticalDelta { 180.f / (1 + nLatitude) };
    const float angleHorizontalDelta{ 180.f / nMeridian };

    std::vector<Vertex> vertices(nVerticesTotal);
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

    return {
        vertices,
        elements,
        {}
    };
}

Model generateSphereModel(int nLatitude, int nMeridian) {
    Mesh mesh { generateSphereMesh(nLatitude, nMeridian) };
    return {
        mesh
    };
}
