#ifndef FOOLSENGINE_UTIL_H
#define FOOLSENGINE_UTIL_H

#include <glm/glm.hpp>

namespace ToyMaker {
    glm::mat4 buildModelMatrix(glm::vec4 position, glm::quat orientation, glm::vec3 scale = glm::vec3{1.f, 1.f, 1.f}); 

    class RangeMapperLinear {
    public:
        RangeMapperLinear(
            double inputLowerBound, double inputUpperBound,
            double outputLowerBound, double outputUpperBound
        );
        double operator() (double value) const;
    private:
        double mInputLowerBound;
        double mInputUpperBound;
        double mOutputLowerBound;
        double mOutputUpperBound;
    };
}

#endif
