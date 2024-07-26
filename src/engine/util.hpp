#ifndef ZOUTIL_H
#define ZOUTIL_H

#include <glm/glm.hpp>

glm::mat4 buildModelMatrix(glm::vec4 position, glm::quat orientation, glm::vec3 scale = glm::vec3{1.f, 1.f, 1.f}); 

#endif
