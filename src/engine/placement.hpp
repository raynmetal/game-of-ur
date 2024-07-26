#ifndef ZOPLACEMENT_H
#define ZOPLACEMENT_H

#include <glm/glm.hpp>

struct Placement {
    glm::vec4 mPosition {};
    glm::quat mOrientation { glm::vec3{0.f} };
    glm::vec3 mScale {1.f, 1.f, 1.f};
};

#endif
