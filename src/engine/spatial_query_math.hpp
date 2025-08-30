#ifndef FOOLSENGINE_SPATIALQUERYMATH_H
#define FOOLSENGINE_SPATIALQUERYMATH_H

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "spatial_query_basic_types.hpp"
#include "core/ecs_world.hpp"

namespace ToyMaker {
    struct ObjectBounds;
    class AxisAlignedBounds;

    std::array<AreaTriangle, 12> computeBoxFaceTriangles(const std::array<glm::vec3, 8>& boxCorners);

    /** Returns a bool-vector pair, with bool indicating whether a point of intersection
     * was found, and the vector containing the point of intersection
     * 
     * Error if invalid ray or plane specified (plane with no normal, or ray with no direction)
     */
    std::pair<bool, glm::vec3> computeIntersection(const Ray& ray, const Plane& plane);

    /** Returns a bool-vector pair, with bool indicating whether a point of intersection
     * was found, and the vector containing the point of intersection
     * 
     * Error if invalid ray or triangle specified (triangle with no area, or ray with no direction)
     */
    std::pair<bool, glm::vec3> computeIntersection(const Ray& ray, const AreaTriangle& triangle);

    /** Returns an unsigned int and vector pair pair, with unsigned indicating whether and how many points of intersection
     * were found with the AABB, and the vector containing the points of intersection
     * 
     * (if the ray "glances off" the volume, it does not count as an intersection
     * per my implementation)
     * 
     * Error if invalid ray or AABB specified, which includes an AABB with negative or zero volume
     */
    std::pair<uint8_t, std::pair<glm::vec3, glm::vec3>> computeIntersections(const Ray& ray, const AxisAlignedBounds& axisAlignedBounds);


    /** Returns an unsigned int and vector pair pair, with int indicating whether and how many 
     * points of intersection were found, and the vector containing the points of 
     * intersection
     * 
     * Object bounds only supports convex shapes, and so one can expect that if a
     * point of intersection exists, then there will be a second to go with it. (provided the ray is 
     * long enough) (and also unless the ray "glances off" the volume, which counts as no intersection
     * per my implementation)
     */
    // std::pair<uint8_t, std::pair<glm::vec3, glm::vec3>> computeIntersections(const Ray& ray, const ObjectBounds& objectbounds);
    /**
     * Returns a bool value indicating whether the ray passes through the object volume
     */
    // bool overlaps(const Ray& ray, const ObjectBounds& objectBounds);

    // returns whether point is contained by bounds
    bool overlaps(const glm::vec3& point, const AxisAlignedBounds& bounds);
    // returns whether ray overlaps with bounds
    bool overlaps(const Ray& ray, const AxisAlignedBounds& bounds);
    // returns whether one overlaps two
    bool overlaps(const AxisAlignedBounds& one, const AxisAlignedBounds& two);
    // returns whether point is contained by bounds
    bool contains(const glm::vec3& point, const AxisAlignedBounds& bounds);
    // returns whether ray is contained by bounds
    bool contains(const Ray& ray, const AxisAlignedBounds& bounds);
    // returns whether one is contained by two
    bool contains(const AxisAlignedBounds& one, const AxisAlignedBounds& two);

    struct ObjectBounds {
        inline static std::string getComponentTypeName() { return "ObjectBounds"; }
        enum class TrueVolumeType: uint8_t {
            BOX,
            SPHERE,
            CAPSULE,
        };
        union TrueVolume {
            VolumeBox mBox { .mDimensions{ glm::vec3{0.f} } };
            VolumeCapsule mCapsule;
            VolumeSphere mSphere;
        };
        static ObjectBounds create(const VolumeBox& box, const glm::vec3& positionOffset, const glm::vec3& orientationOffset);
        static ObjectBounds create(const VolumeCapsule& capsule, const glm::vec3& positionOffset, const glm::vec3& orientationOffset);
        static ObjectBounds create(const VolumeSphere& sphere, const glm::vec3& positionOffset, const glm::vec3& orientationOffset);
        TrueVolumeType mType { TrueVolumeType::BOX };
        TrueVolume mTrueVolume {};
        glm::vec3 mPosition { 0.f };
        glm::vec3 mPositionOffset { 0.f };
        glm::quat mOrientation { glm::vec3{ 0.f } };
        glm::quat mOrientationOffset { glm::vec3{ 0.f } };

        void applyModelMatrix(const glm::mat4& modelMatrix);

        inline glm::mat3 getLocalRotationTransform() const { 
            return glm::mat3_cast(glm::normalize(mOrientationOffset));
        }
        inline glm::mat3 getWorldRotationTransform() const {
            return glm::mat3_cast(glm::normalize(mOrientation));
        }

        glm::vec3 getComputedWorldPosition() const;
        glm::quat getComputedWorldOrientation() const;

        std::array<glm::vec3, 8> getVolumeRelativeBoxCorners() const;
        std::array<glm::vec3, 8> getLocalOrientedBoxCorners() const;
        std::array<glm::vec3, 8> getWorldOrientedBoxCorners() const;
        std::array<AreaTriangle, 12> getWorldOrientedBoxFaceTriangles() const { return computeBoxFaceTriangles(getWorldOrientedBoxCorners()); };
    protected:
    };

    class AxisAlignedBounds {
    public:
        inline static std::string getComponentTypeName() { return "AxisAlignedBounds"; }
        /**
         * first: right top front
         * second: left back bottom
         */
        using Extents = std::pair<glm::vec3, glm::vec3>;

        AxisAlignedBounds();
        AxisAlignedBounds(const ObjectBounds& objectBounds);
        AxisAlignedBounds(const Extents& axisAlignedExtents);
        AxisAlignedBounds(const glm::vec3& position, const glm::vec3& dimensions):
        AxisAlignedBounds { Extents{{position + .5f*dimensions}, {position - .5f*dimensions}} }
        {}

        AxisAlignedBounds operator+(const AxisAlignedBounds& other) const;

        std::array<glm::vec3, 8> getAxisAlignedBoxCorners() const;
        inline std::array<AreaTriangle, 12> getAxisAlignedBoxFaceTriangles() const { return computeBoxFaceTriangles(getAxisAlignedBoxCorners()); }
        Extents getAxisAlignedBoxExtents() const;

        inline glm::vec3 getDimensions() const { return mExtents.first - mExtents.second; }
        inline glm::vec3 getPosition() const { return mExtents.second + .5f * getDimensions(); }; 

        inline bool isSensible() const {
            return (
                isFinite(mExtents.first) && isFinite(mExtents.second)
            );
        }
        inline void setPosition(const glm::vec3& position) {
            assert(isFinite(position) && "Invalid position specified. Position must be finite");
            const glm::vec3 deltaPosition { position - getPosition() };
            mExtents.first += deltaPosition;
            mExtents.second += deltaPosition;
        }
        inline void setDimensions(const glm::vec3& dimensions) {
            assert(isNonNegative(dimensions) && isFinite(dimensions) && "Invalid dimensions provided.  Dimensions must be non negative and finite");
            const glm::vec3 position { getPosition() };
            const glm::vec3 deltaDimensions { dimensions - getDimensions() };
            mExtents.first += .5f * deltaDimensions;
            mExtents.second -= .5f * deltaDimensions;
        }
    private:
        void setByExtents(const Extents& axisAlignedExtents);
        Extents mExtents {glm::vec3{0.f}, glm::vec3{0.f}};
    };

    NLOHMANN_JSON_SERIALIZE_ENUM( ObjectBounds::TrueVolumeType, {
        {ObjectBounds::TrueVolumeType::BOX, "box"},
        {ObjectBounds::TrueVolumeType::SPHERE, "sphere"},
        {ObjectBounds::TrueVolumeType::CAPSULE, "capsule"},
    })

    inline void to_json(nlohmann::json& json, const ObjectBounds& objectBounds) {
        json = {
            {"type", ObjectBounds::getComponentTypeName()},
            {"volume_type", objectBounds.mType},
            {"position_offset", { objectBounds.mPositionOffset.x, objectBounds.mPositionOffset.y, objectBounds.mPositionOffset.z}},
            {"orientation_offset", { objectBounds.mOrientationOffset.w, objectBounds.mOrientationOffset.x, objectBounds.mOrientationOffset.y, objectBounds.mOrientationOffset.z }},
        };
        switch(objectBounds.mType) {
            case ObjectBounds::TrueVolumeType::BOX:
                json["volume_properties"] = {
                    {"width", objectBounds.mTrueVolume.mBox.mDimensions.x},
                    {"height", objectBounds.mTrueVolume.mBox.mDimensions.y},
                    {"depth", objectBounds.mTrueVolume.mBox.mDimensions.z},
                };
                break;
            case ObjectBounds::TrueVolumeType::SPHERE:
                json["volume_properties"] = {
                    {"radius", objectBounds.mTrueVolume.mSphere.mRadius},
                };
                break;
            case ObjectBounds::TrueVolumeType::CAPSULE:
                json["volume_properties"] = {
                    {"radius", objectBounds.mTrueVolume.mCapsule.mRadius},
                    {"height", objectBounds.mTrueVolume.mCapsule.mHeight},
                };
                break;
        };
    }

    inline void from_json(const nlohmann::json& json, ObjectBounds& objectBounds) {
        assert(json.at("type") == ObjectBounds::getComponentTypeName() && "Incorrect type property for an objectBounds component");
        const glm::vec3 positionOffset {
            json.at("position_offset")[0],
            json.at("position_offset")[1],
            json.at("position_offset")[2],
        };
        const glm::vec3 orientationOffset { 
            glm::eulerAngles(glm::normalize(glm::quat {
                json.at("orientation_offset")[0],
                json.at("orientation_offset")[1],
                json.at("orientation_offset")[2],
                json.at("orientation_offset")[3]
            }))
        };

        switch (static_cast<ObjectBounds::TrueVolumeType>(json.at("volume_type"))) {
            case ObjectBounds::TrueVolumeType::BOX:
                objectBounds = ObjectBounds::create(
                    VolumeBox{ .mDimensions {
                        json.at("volume_properties").at("width").get<float>(),
                        json.at("volume_properties").at("height").get<float>(),
                        json.at("volume_properties").at("depth").get<float>(),
                    } },
                    positionOffset,
                    orientationOffset
                );
                break;

            case ObjectBounds::TrueVolumeType::SPHERE:
                objectBounds = ObjectBounds::create(
                    VolumeSphere { .mRadius { json.at("volume_properties").at("radius").get<float>() }},
                    positionOffset,
                    orientationOffset
                );
                break;

            case ObjectBounds::TrueVolumeType::CAPSULE:
                objectBounds = ObjectBounds::create(
                    VolumeCapsule {
                        .mHeight { json.at("volume_properties").at("height").get<float>() },
                        .mRadius { json.at("volume_properties").at("radius").get<float>() },
                    },
                    positionOffset,
                    orientationOffset
                );
                break;
        }
    }

    inline void to_json(nlohmann::json& json, const AxisAlignedBounds& axisAlignedBounds) { /* never used, so pass */
        (void)json; // prevent unused parameter warnings
        (void)axisAlignedBounds; // prevent unused parameter warnings
    }
    inline void from_json(const nlohmann::json& json, AxisAlignedBounds& objectBounds) { /* never used, so pass */ 
        (void)json; // prevent unused parameter warnings
        (void)objectBounds; // prevent unused parameter warnings
    }

}

#endif
