#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "util.hpp"

glm::mat4 buildModelMatrix(glm::vec4 position, glm::quat orientation, glm::vec3 scale) {
    glm::mat4 rotateMatrix { glm::normalize(orientation) };
    glm::mat4 translateMatrix { glm::translate(glm::mat4(1.f), glm::vec3(position)) };
    glm::mat4 scaleMatrix { glm::scale(glm::mat4(1.f), scale) };
    return translateMatrix * rotateMatrix * scaleMatrix;
}

