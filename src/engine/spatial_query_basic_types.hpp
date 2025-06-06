#ifndef FOOLSENGINE_SPATIALQUERYGEOMETRY_H
#define FOOLSENGINE_SPATIALQUERYGEOMETRY_H

#include <cmath>
#include <array>
#include <glm/glm.hpp>

namespace ToyMakersEngine {
    using BoxCorner = uint8_t;
    enum BoxCornerSpecifier: BoxCorner {
        RIGHT=0x1,
        TOP=0x2,
        FRONT=0x4
    };


    inline bool isFinite(float number) {
        return std::isfinite(number);
    }
    inline bool isFinite(const glm::vec3& vector) {
        return isFinite(vector.x) && isFinite(vector.y) && isFinite(vector.z);
    }
    inline bool isPositive(float number) {
        return number > 0.f;
    }
    inline bool isPositive(const glm::vec3& vector) {
        return isPositive(vector.x) && isPositive(vector.y) && isPositive(vector.z);
    }
    inline bool isNonNegative(float number) {
        return number >= 0.f;
    }
    inline bool isNonNegative(const glm::vec3& vector) {
        return isNonNegative(vector.x) && isNonNegative(vector.y) && isNonNegative(vector.z);
    }

    template <typename TDerived>
    struct Volume;


    struct VolumeBase_ {
        inline static constexpr std::array<glm::vec3, 8> GetCornerSignsArray() {
            std::array<glm::vec3, 8> cornerSigns {};
            for(uint8_t corner {0}; corner < 8; ++corner) {
                cornerSigns[corner].x = corner&BoxCornerSpecifier::RIGHT? 1.f: -1.f;
                cornerSigns[corner].y = corner&BoxCornerSpecifier::TOP? 1.f: -1.f;
                cornerSigns[corner].z = corner&BoxCornerSpecifier::FRONT? 1.f: -1.f;
            }
            return cornerSigns;
        }

        inline static std::array<glm::vec3, 8> ComputeBoxCorners(const glm::vec3& boxDimensions) {
            const std::array<glm::vec3, 8> cornerSignsArray { GetCornerSignsArray() };
            glm::vec3 absoluteCornerOffset { .5f * boxDimensions };
            std::array<glm::vec3, 8> cornerArray { .5f * boxDimensions };
            for(uint8_t corner{0}; corner < 8; ++corner) {
                cornerArray[corner] = cornerSignsArray[corner] * absoluteCornerOffset;
            }
            return cornerArray;
        }

        // Poor man's vtable I guess, without breaking Volume's ability to
        // be aggregate initialized
        template <typename TDerived>
        inline std::array<glm::vec3, 8> getVolumeRelativeBoxCorners() const {
            return Volume<TDerived>::getVolumeRelativeBoxCorners();
        }
    };

    template <typename TDerived>
    struct Volume: public VolumeBase_ {
        // poor man's vtable cont'd
        inline std::array<glm::vec3, 8> getVolumeRelativeBoxCorners() const {
            return TDerived::getVolumeRelativeBoxCorners();
        }
    };

    struct VolumeBox: public Volume<VolumeBox> {
        glm::vec3 mDimensions {0.f, 0.f, 0.f};
        inline std::array<glm::vec3, 8> getVolumeRelativeBoxCorners () const {
            return ComputeBoxCorners(mDimensions);
        }
        inline bool isSensible() const {
            return isPositive(mDimensions) && isFinite(mDimensions);
        }
    };

    struct VolumeCapsule: public Volume<VolumeCapsule> {
        float mHeight {0.f};
        float mRadius {0.f};
        inline std::array<glm::vec3, 8> getVolumeRelativeBoxCorners () const {
            const glm::vec3 boxDimensions { 2.f*mRadius, mHeight + 2.f*mRadius, 2.f*mRadius};
            return ComputeBoxCorners(boxDimensions);
        }
        inline bool isSensible() const {
            return (
                isPositive(mHeight) && isFinite(mHeight)
                && isPositive(mRadius) && isFinite(mRadius)
            );
        }
    };

    struct VolumeSphere: public Volume<VolumeSphere> {
        float mRadius {0.f};
        inline std::array<glm::vec3, 8> getVolumeRelativeBoxCorners () const {
            return ComputeBoxCorners(glm::vec3{2*mRadius});
        }
        inline bool isSensible() const {
            return isPositive(mRadius) && isFinite(mRadius);
        }
    };

    struct AreaTriangle {
        std::array<glm::vec3, 3> mPoints {};
        inline bool isSensible() const {
            return (
                (
                    isFinite(mPoints[0]) && isFinite(mPoints[1]) && isFinite(mPoints[2])
                ) && (
                    isPositive(
                        glm::cross(
                            mPoints[2] - mPoints[0], mPoints[1] - mPoints[0]
                        ).length()
                    )
                )
            );
        }
    };

    struct AreaCircle {
        float mRadius { 0.f };
        glm::vec3 mCenter { 0.f };
        glm::vec3 mNormal { 0.f, -1.f, 0.f };
        inline bool isSensible() const {
            return (
                (
                    isFinite(mRadius) && isPositive(mRadius)
                ) && (
                    isFinite(mNormal)
                ) && (
                    isFinite(mCenter)
                )
            );
        }
    };

    struct Ray {
        glm::vec3 mStart { 0.f };
        glm::vec3 mDirection { 0.f, 0.f, -1.f };
        float mLength { std::numeric_limits<float>::infinity() };

        inline bool isSensible() const {
            return (
                (
                    isFinite(mDirection) && isPositive(mDirection.length())
                ) && (
                    isFinite(mStart)
                ) && (
                    isPositive(mLength)
                )
            );
        }
    };

    struct Plane {
        glm::vec3 mPointOnPlane { 0.f };
        glm::vec3 mNormal { 0.f, 0.f, -1.f };

        inline bool isSensible() const {
            return (
                (
                    isFinite(mNormal) && isPositive(mNormal.length())
                ) && (
                    isFinite(mPointOnPlane)
                )
            );
        }
    };
}

#endif
