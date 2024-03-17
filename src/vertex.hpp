#ifndef ZOVERTEX_H
#define ZOVERTEX_H

#include <glm/glm.hpp>

struct Vertex {
    glm::vec4 mPosition;
    glm::vec4 mNormal;
    glm::vec4 mTangent;
    glm::vec4 mColor {1.f}; // by default, white
    glm::vec2 mTextureCoordinates;
};

#endif
