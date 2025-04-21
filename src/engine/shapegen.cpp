#include <vector>

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "core/resource_database.hpp"
#include "vertex.hpp"
#include "model.hpp"
#include "material.hpp"
#include "mesh.hpp"
#include "shapegen.hpp"

std::shared_ptr<StaticMesh> generateSphereMesh(int nLatitude, int nMeridian, bool flipTextureY=false);
std::shared_ptr<StaticMesh> generateRectangleMesh(float width=2.f, float height=2.f, bool flipTextureY=false);

std::shared_ptr<IResource> StaticMeshSphereLatLong::createResource(const nlohmann::json& methodParameters) {
    return generateSphereMesh(
        methodParameters.at("nLatitudes").get<uint32_t>(),
        methodParameters.at("nMeridians").get<uint32_t>(),
        methodParameters.find("flip_texture_y") != methodParameters.end()? methodParameters.at("flip_texture_y").get<bool>(): false
    );
}

std::shared_ptr<IResource> StaticMeshRectangleDimensions::createResource(const nlohmann::json& methodParameters) {
    return generateRectangleMesh(
        methodParameters.at("width").get<float>(),
        methodParameters.at("height").get<float>(),
        methodParameters.find("flip_texture_y") != methodParameters.end()? methodParameters.at("flip_texture_y").get<bool>(): false
    );
}

std::shared_ptr<IResource> StaticModelSphereLatLong::createResource(const nlohmann::json& methodParameters) {
    std::shared_ptr<StaticMesh> sphereMesh {
        ResourceDatabase::ConstructAnonymousResource<StaticMesh>({
            {"type", StaticMesh::getResourceTypeName()},
            {"method", StaticMeshSphereLatLong::getResourceConstructorName()},
            {"parameters", methodParameters}
        })
    };
    std::shared_ptr<Material> sphereMaterial { 
        ResourceDatabase::ConstructAnonymousResource<Material>({
            {"type", Material::getResourceTypeName()},
            {"method", MaterialFromDescription::getResourceConstructorName()},
            {"parameters", {
                {"properties", nlohmann::json::array()},
            }}
        })
    };

    std::shared_ptr<StaticModel> sphereModel { 
        std::make_shared<StaticModel>(
            std::vector<std::shared_ptr<StaticMesh>>{ sphereMesh },
            std::vector<std::shared_ptr<Material>>{ sphereMaterial }
        )
    };

    return sphereModel;
}

std::shared_ptr<IResource> StaticModelRectangleDimensions::createResource(const nlohmann::json& methodParameters) {
    std::shared_ptr<StaticMesh> rectangleMesh {
        ResourceDatabase::ConstructAnonymousResource<StaticMesh>({
            {"type", StaticMesh::getResourceTypeName()},
            {"method", StaticMeshRectangleDimensions::getResourceConstructorName()},
            {"parameters", methodParameters}
        })
    };
    std::shared_ptr<Material> rectangleMaterial { 
        ResourceDatabase::ConstructAnonymousResource<Material>({
            {"type", Material::getResourceTypeName()},
            {"method", MaterialFromDescription::getResourceConstructorName()},
            {"parameters", {
                {"properties", nlohmann::json::array()},
            }}
        })
    };

    std::shared_ptr<StaticModel> rectangleModel{
        std::make_shared<StaticModel>(
            std::vector<std::shared_ptr<StaticMesh>>{rectangleMesh},
            std::vector<std::shared_ptr<Material>>{rectangleMaterial}
        )
    };

    return rectangleModel;
}

std::shared_ptr<StaticMesh> generateSphereMesh(int nLatitude, int nMeridian, bool flipTextureY)  {
    assert(nLatitude >= 1);
    assert(nMeridian >= 2);

    const glm::mat3 textureCoordinateTransform { flipTextureY ?
        glm::mat3 { // column major order
            {1.f, 0.f, 0.f},
            {0.f, -1.f, 0.f},
            {0.f, 1.f, 1.f},
        }:
        glm::mat3 { 1.f }
    };

    const int nVerticesPerLatitude { 2 * nMeridian };
    const int nVerticesTotal { 2 + nLatitude * nVerticesPerLatitude };

    const float angleVerticalDelta { 180.f / (1 + nLatitude) };
    const float angleHorizontalDelta{ 180.f / nMeridian };

    std::vector<BuiltinVertexData> vertices(nVerticesTotal);
    int currentIndex { 0 };
    for(int i{0}; i < 2 + nLatitude; ++i) {
        const float angleVertical { i * angleVerticalDelta };
        const int nPointsCurrentLatitude {
            i % (1 + nLatitude)? nVerticesPerLatitude: 1
        };
        for(int j{0}; j < nPointsCurrentLatitude; ++j) {
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
            vertices[currentIndex].mUV3
                = vertices[currentIndex].mUV2
                = vertices[currentIndex].mUV1
                = static_cast<glm::vec2>(
                    textureCoordinateTransform 
                    * glm::vec3(
                        static_cast<float>(j)/nPointsCurrentLatitude, angleVertical / 180.f, 1.f
                    )
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
        const int nPointsCurrentLatitude {
            i%(1+nLatitude)? nVerticesPerLatitude: 1
        };
        const int nPointsPreviousLatitude {
            (i-1)%(1+nLatitude)? nVerticesPerLatitude: 1
        };
        const int currentBaseIndex {
            previousBaseIndex + nPointsPreviousLatitude
        };
        const int nJoiningFaces {std::max(nPointsCurrentLatitude, nPointsPreviousLatitude)};

        for(int j{0}; j < nJoiningFaces; ++j) {
            const int topleft { previousBaseIndex + (j % nPointsPreviousLatitude)};
            const int topright { previousBaseIndex + ((1+j) % nPointsPreviousLatitude)};
            const int bottomleft { currentBaseIndex + (j % nPointsCurrentLatitude) };
            const int bottomright  { currentBaseIndex + ((1+j) % nPointsCurrentLatitude)};

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

std::shared_ptr<StaticMesh> generateRectangleMesh(float width, float height, bool flipTextureY) {
    assert(width > 0.f);
    assert(height > 0.f);
    const glm::mat3 textureCoordinateTransform { flipTextureY ?
        glm::mat3 { // column major order
            {1.f, 0.f, 0.f},
            {0.f, -1.f, 0.f},
            {0.f, 1.f, 1.f},
        }:
        glm::mat3 { 1.f }
    };

    std::vector<BuiltinVertexData> vertices {
        {
            .mPosition {-width/2.f, height/2.f, 0.f, 1.f},
            .mNormal{0.f, 0.f, 1.f, 0.f},
            .mTangent{1.f, 0.f, 0.f, 0.f},
            .mColor{1.f, 1.f, 1.f, 1.f},
            .mUV1{ static_cast<glm::vec2>(textureCoordinateTransform * glm::vec3 {0.f, 1.f, 1.f}) },
            .mUV2{ static_cast<glm::vec2>(textureCoordinateTransform * glm::vec3 {0.f, 1.f, 1.f}) },
            .mUV3{ static_cast<glm::vec2>(textureCoordinateTransform * glm::vec3 {0.f, 1.f, 1.f}) },
        },
        {
            .mPosition {width/2.f, height/2.f, 0.f, 1.f},
            .mNormal{0.f, 0.f, 1.f, 0.f},
            .mTangent{1.f, 0.f, 0.f, 0.f},
            .mColor{1.f, 1.f, 1.f, 1.f},
            .mUV1{ static_cast<glm::vec2>(textureCoordinateTransform * glm::vec3 {1.f, 1.f, 1.f}) },
            .mUV2{ static_cast<glm::vec2>(textureCoordinateTransform * glm::vec3 {1.f, 1.f, 1.f}) },
            .mUV3{ static_cast<glm::vec2>(textureCoordinateTransform * glm::vec3 {1.f, 1.f, 1.f}) },
        },
        {
            .mPosition {width/2.f, -height/2.f, 0.f, 1.f},
            .mNormal {0.f, 0.f, 1.f, 0.f},
            .mTangent {1.f, 0.f, 0.f, 0.f},
            .mColor {1.f, 1.f, 1.f, 1.f},
            .mUV1{ static_cast<glm::vec2>(textureCoordinateTransform * glm::vec3 {1.f, 0.f, 1.f}) },
            .mUV2{ static_cast<glm::vec2>(textureCoordinateTransform * glm::vec3 {1.f, 0.f, 1.f}) },
            .mUV3{ static_cast<glm::vec2>(textureCoordinateTransform * glm::vec3 {1.f, 0.f, 1.f}) },
        },
        {
            .mPosition {-width/2.f, -height/2.f, 0.f, 1.f},
            .mNormal {0.f, 0.f, 1.f, 0.f},
            .mTangent {1.f, 0.f, 0.f, 0.f},
            .mColor {1.f, 1.f, 1.f, 1.f},
            .mUV1{ static_cast<glm::vec2>(textureCoordinateTransform * glm::vec3 {0.f, 0.f, 1.f}) },
            .mUV2{ static_cast<glm::vec2>(textureCoordinateTransform * glm::vec3 {0.f, 0.f, 1.f}) },
            .mUV3{ static_cast<glm::vec2>(textureCoordinateTransform * glm::vec3 {0.f, 0.f, 1.f}) },
        },
    };

    std::vector<GLuint> elements {
        {0}, {2}, {1},
        {0}, {3}, {2}
    };

    return std::make_shared<StaticMesh>(vertices, elements);
}
