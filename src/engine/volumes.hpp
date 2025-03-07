#ifndef ZOVOLUMES_H
#define ZOVOLUMES_H

#include <glm/glm.hpp>

struct VolumeBox {
    glm::vec3 mDimensions {0.f, 0.f, 0.f};
};

struct VolumeCapsule {
    float mHeight {0.f};
    float mRadius {0.f};
};

struct VolumeSphere {
    float mRadius {0.f};
};

#endif
