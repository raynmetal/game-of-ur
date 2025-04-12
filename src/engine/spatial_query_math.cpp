#include <cmath>
#include "spatial_query_math.hpp"

inline float squareDistance(const glm::vec3& vector) { return glm::dot(vector, vector); }

std::pair<bool, glm::vec3> computeIntersection(const Ray& ray, const Plane& plane) {
    assert(ray.isSensible() && "Invalid ray provided");
    assert(plane.isSensible() && "Invalid plane provided");

    // ray is parallel to the plane (i.e., perpendicular to the plane's normal)
    if(glm::dot(plane.mNormal, ray.mDirection) == 0.f)  {
        // if start point does not lie on the plane, there is no point of intersection
        if(glm::dot(plane.mPointOnPlane - ray.mStart, plane.mNormal)) {
            return {false, glm::vec3{std::numeric_limits<float>::infinity()}};
        }

        // start point does lie on the plane, and therefore is the (first) point of intersection
        return {true, ray.mStart};
    }

    // work out point of intersection parametrically
    const glm::vec3 rayDirection { glm::normalize(ray.mDirection) };
    const glm::vec3 planeNormal { glm::normalize(glm::dot(plane.mNormal, rayDirection)<0.f? -plane.mNormal: plane.mNormal) };
    const float rayIntersectionDistance {
        glm::dot(planeNormal,  (plane.mPointOnPlane - ray.mStart))
        / glm::dot(planeNormal, rayDirection)
    };

    // If rayIntersectionDistance does not fall within the length specified for the ray, there is
    // no point of intersection
    if(rayIntersectionDistance < 0 || rayIntersectionDistance > ray.mLength) {
        return { false, glm::vec3(std::numeric_limits<float>::infinity()) };
    }

    return { true, ray.mStart + rayIntersectionDistance * rayDirection };
}

std::pair<bool, glm::vec3> computeIntersection(const Ray& ray, const AreaTriangle& triangle) {
    assert(ray.isSensible() && "Invalid ray provided");
    assert(triangle.isSensible() && "Invalid triangle provided");

    const glm::vec3 triangleNormal { glm::cross(
        triangle.mPoints[2] - triangle.mPoints[0],
        triangle.mPoints[1] - triangle.mPoints[0]
    ) };
    // Find point of intersection with triangle's plane
    std::pair<bool, glm::vec3> planeIntersection { 
        computeIntersection(ray, Plane{ .mPointOnPlane{ triangle.mPoints[0] }, .mNormal{ triangleNormal } })
    };

    // no intersection with plane found, return false (whatever the plane intersection
    // computation returned)
    if(!planeIntersection.first) return planeIntersection;

    // plane intersection found, see if intersection point lies within triangle
    // Sum of areas of triangles formed between each pair of triangle points and the point of intersection
    // will be the same as the area of the triangle iff the point lies within the triangle

    const float triangleAreaDoubledSquared { 
        .25f * squareDistance(glm::cross(triangle.mPoints[1] - triangle.mPoints[0], triangle.mPoints[2] - triangle.mPoints[0]))
    };
    const float quadAreaDoubledSquared { .25f * (
        squareDistance(glm::cross(triangle.mPoints[0] - planeIntersection.second, triangle.mPoints[1] - planeIntersection.second))
        + squareDistance(glm::cross(triangle.mPoints[0] - planeIntersection.second, triangle.mPoints[2] - planeIntersection.second))
        + squareDistance(glm::cross(triangle.mPoints[1] - planeIntersection.second, triangle.mPoints[2] - planeIntersection.second))
    )};

    if(
        // either quad area is obviously lesser than triangle area
        quadAreaDoubledSquared <= triangleAreaDoubledSquared

        // or they're approximately equal, and we need to do a relative epsilon
        // based equality comparison
        || (
            std::abs(quadAreaDoubledSquared - triangleAreaDoubledSquared) 
            <= (std::numeric_limits<float>::epsilon() * std::max(quadAreaDoubledSquared, triangleAreaDoubledSquared))
        )
    ) {
        return planeIntersection;
    }

    /** If the ray and the triangle happen to lie on the same plane, we still have a shot.
     * TODO: Honestly, this is a bit more studying than I'm willing to do at this point.  Just let
     * this edge case be.
     * NOTE: We can determine whether an intersection took place easily enough with the code commented
     * out.  It's slightly more complicated to determine the location of the intersection
     */
    // if(!glm::dot(triangleNormal, ray.mDirection)) {
    //     const glm::vec3 rayUnitDirection { glm::normalize(ray.mDirection) };
    //     const std::array<std::pair<uint8_t, uint8_t>, 3> trianglePointIndexPairs {{ { 0, 1 }, { 0, 2 }, { 1, 2 } }};
    //     // check all edges against projections on triangle edges
    //     for(const auto& indexPair: trianglePointIndexPairs) {
    //         const glm::vec3 triangleEdgeUnitDirection { glm::normalize(triangle.mPoints[indexPair.first] - triangle.mPoints[indexPair.second]) };
    //         const std::array<float, 3> trianglePointProjections {{
    //             glm::dot(triangle.mPoints[0] - triangle.mPoints[1], triangleEdgeUnitDirection),
    //             glm::dot(triangle.mPoints[0] - triangle.mPoints[2], triangleEdgeUnitDirection),
    //             glm::dot(triangle.mPoints[1] - triangle.mPoints[2], triangleEdgeUnitDirection),
    //         }};

    //         std::array<float, 2> rayPointProjections {
    //             glm::dot(ray.mStart - ray.mStart, triangleEdgeUnitDirection),
    //             glm::dot((ray.mStart + ray.mLength * rayUnitDirection) - ray.mStart, triangleEdgeUnitDirection),
    //         };
    //         const float triangleMax { *std::max_element(trianglePointProjections.begin(), trianglePointProjections.end()) };
    //         const float triangleMin { *std::min_element(trianglePointProjections.begin(), trianglePointProjections.end()) };
    //         const float rayMax { std::max(rayPointProjections[0], rayPointProjections[1]) };
    //         const float rayMin { std::min(rayPointProjections[0], rayPointProjections[1]) };

    //         if(triangleMax < rayMin || triangleMin > rayMax) { 
    //             return { false, glm::vec3{ std::numeric_limits<float>::infinity() }};
    //         }
    //     }

    //     // check all edges against projection on ray edge
    //     const std::array<float, 3> trianglePointProjections {{
    //         glm::dot(triangle.mPoints[0] - triangle.mPoints[1], rayUnitDirection),
    //         glm::dot(triangle.mPoints[0] - triangle.mPoints[2], rayUnitDirection),
    //         glm::dot(triangle.mPoints[1] - triangle.mPoints[2], rayUnitDirection),
    //     }};
    //     std::array<float, 2> rayPointProjections {
    //         glm::dot(ray.mStart - ray.mStart, rayUnitDirection),
    //         glm::dot((ray.mStart + ray.mLength * rayUnitDirection) - ray.mStart, rayUnitDirection),
    //     };
    //     const float triangleMax { *std::max_element(trianglePointProjections.begin(), trianglePointProjections.end()) };
    //     const float triangleMin { *std::min_element(trianglePointProjections.begin(), trianglePointProjections.end()) };
    //     const float rayMax { std::max(rayPointProjections[0], rayPointProjections[1]) };
    //     const float rayMin { std::min(rayPointProjections[0], rayPointProjections[1]) };

    //     // no overlap on any axis, so we're done
    //     if(triangleMax < rayMin || triangleMin > rayMax) {
    //         return { false, glm::vec3{ std::numeric_limits<float>::infinity() } };
    //     }

    //     // find point of intersection between ray and triangle
    //     static_assert(false && "Finding point of intersection b/w line and triangle lying on a single plane unimplemented");
    // }

    // No point of intersection found
    return { false, glm::vec3{std::numeric_limits<float>::infinity()} };
}

std::pair<uint8_t, std::pair<glm::vec3, glm::vec3>> computeIntersections(const Ray& ray, const AxisAlignedBounds& bounds) {
    assert(ray.isSensible() && "Invalid ray provided");
    assert(
        bounds.isSensible() && "Invalid axis-aligned box provided"
    );

    // box with no volume provided
    if(!isPositive(bounds.getDimensions())) {
        return {false, {glm::vec3{std::numeric_limits<float>::infinity()}, glm::vec3{std::numeric_limits<float>::infinity()}}};
    }

    const std::array<AreaTriangle, 12> boxTriangles { bounds.getAxisAlignedBoxFaceTriangles() };
    std::array<glm::vec3, 2> intersectionPoints { glm::vec3{std::numeric_limits<float>::infinity()}, glm::vec3{std::numeric_limits<float>::infinity()} };
    uint8_t nIntersections { 0 };
    for(uint8_t i{0}; i < 12 && nIntersections < 2; ++i) {
        const AreaTriangle& triangle { boxTriangles[i] };
        std::pair<bool, glm::vec3> possibleIntersection { computeIntersection(ray, triangle) };
        if(possibleIntersection.first) {
            intersectionPoints[nIntersections++] = possibleIntersection.second;
            // skip the next triangle on this face
            if(i % 12 == 0) {
                ++i;
            }
        }
    }
    return { nIntersections, std::pair<glm::vec3, glm::vec3>(intersectionPoints[0], intersectionPoints[1]) };
}

bool overlaps(const glm::vec3& point, const AxisAlignedBounds& bounds) { return contains(point, bounds); }

bool overlaps(const Ray& ray, const AxisAlignedBounds& bounds) {
    assert(ray.isSensible() && "Invalid ray provided");
    assert(bounds.isSensible() && "Invalid axis aligned box provided");

    // box with no volume provided
    if(!isPositive(bounds.getDimensions())) {
        return false;
    }

    return (
        // either ray begins within the box, or...
        (
            contains(ray.mStart, bounds)
        // the ray intersects with the box
        ) || (
            static_cast<bool>(computeIntersections(ray, bounds).first)
        )
    );
}

bool overlaps(const AxisAlignedBounds& one, const AxisAlignedBounds& two) {
    assert (
        one.isSensible() && two.isSensible() && "Invalid axis aligned box provided"
    );

    const AxisAlignedBounds::Extents ourExtents { two.getAxisAlignedBoxExtents() };
    const glm::vec3& bottomCorner { ourExtents.second };
    const glm::vec3& topCorner { ourExtents.first };

    const AxisAlignedBounds::Extents otherExtents { one.getAxisAlignedBoxExtents() };
    const glm::vec3& otherBottomCorner { otherExtents.second };
    const glm::vec3& otherTopCorner { otherExtents.first };

    return (
        // overlap in x
        otherBottomCorner.x <= topCorner.x && otherTopCorner.x >= bottomCorner.x 
        // overlap in y
        && otherBottomCorner.y <= topCorner.y && otherTopCorner.y >= bottomCorner.y
        // overlap in z
        && otherBottomCorner.z <= topCorner.z && otherTopCorner.z >= bottomCorner.z
    );
}

bool contains(const glm::vec3& point, const AxisAlignedBounds& bounds) {
    assert(
        bounds.isSensible() && "Invalid axis aligned box provided"
    );
    assert(isFinite(point) && "Invalid point provided");

    const AxisAlignedBounds::Extents boxExtents { bounds.getAxisAlignedBoxExtents() };

    return point.x <= boxExtents.first.x
        && point.y <= boxExtents.first.y
        && point.z <= boxExtents.first.z

        && point.x >= boxExtents.second.x
        && point.y >= boxExtents.second.y
        && point.z >= boxExtents.second.z;
}

bool contains(const Ray& ray, const AxisAlignedBounds& bounds) {
    assert(ray.isSensible() && "Invalid ray provided");
    assert(bounds.isSensible() && "Invalid axis-aligned box provided");
    if(!isFinite(ray.mLength)) return false;
    const AxisAlignedBounds::Extents boxExtents { bounds.getAxisAlignedBoxExtents() };
    const glm::vec3 rayEnd { ray.mStart + glm::normalize(ray.mDirection) * ray.mLength };
    return contains(ray.mStart, bounds) && contains(rayEnd, bounds);
}

bool contains(const AxisAlignedBounds& one, const AxisAlignedBounds& two) {
    assert(one.isSensible() && two.isSensible() && "Invalid axis-aligned box provided");

    const AxisAlignedBounds::Extents ourExtents { two.getAxisAlignedBoxExtents() };
    const glm::vec3& bottomCorner { ourExtents.second };
    const glm::vec3& topCorner { ourExtents.first };

    const AxisAlignedBounds::Extents otherExtents { one.getAxisAlignedBoxExtents() };
    const glm::vec3& otherBottomCorner { otherExtents.second };
    const glm::vec3& otherTopCorner { otherExtents.first };

    return (
        // contained in x
        bottomCorner.x <= otherBottomCorner.x && topCorner.x >= otherTopCorner.x
        // contained in y
        && bottomCorner.y <= otherBottomCorner.y && topCorner.y >= otherTopCorner.y
        // contained in z
        && bottomCorner.z <= otherBottomCorner.z && topCorner.z >= otherTopCorner.z
    );
}

ObjectBounds ObjectBounds::create(const VolumeBox& box, const glm::vec3& positionOffset, const glm::vec3& orientationOffset) {
    return ObjectBounds {
        .mType { TrueVolumeType::BOX },
        .mTrueVolume { .mBox { box } },
        .mPositionOffset { positionOffset },
        .mOrientationOffset{ orientationOffset },
    };
}

ObjectBounds ObjectBounds::create(const VolumeSphere& sphere, const glm::vec3& positionOffset, const glm::vec3& orientationOffset) {
    return ObjectBounds {
        .mType { TrueVolumeType::SPHERE },
        .mTrueVolume { .mSphere { sphere } },
        .mPositionOffset { positionOffset },
        .mOrientationOffset { orientationOffset },
    };
}

ObjectBounds ObjectBounds::create(const VolumeCapsule& capsule, const glm::vec3& positionOffset, const glm::vec3& orientationOffset) {
    return ObjectBounds {
        .mType { TrueVolumeType::CAPSULE },
        .mTrueVolume { .mCapsule { capsule } },
        .mPositionOffset { positionOffset },
        .mOrientationOffset { orientationOffset },
    };
}

void ObjectBounds::applyModelMatrix(const glm::mat4& modelMatrix) {
    mPosition = static_cast<glm::vec3>(modelMatrix * glm::vec4{0.f, 0.f, 0.f, 1.f});
    mOrientation = glm::quat_cast(glm::transpose(glm::inverse(modelMatrix)));
}

std::array<AreaTriangle, 12> computeBoxFaceTriangles(const std::array<glm::vec3, 8>& boxCorners) {
    return std::array<AreaTriangle, 12> {{
        // left faces
        AreaTriangle{.mPoints{
            boxCorners[0],
            boxCorners[BoxCornerSpecifier::FRONT],
            boxCorners[BoxCornerSpecifier::FRONT|BoxCornerSpecifier::TOP]}},
        AreaTriangle{.mPoints{
            boxCorners[0],
            boxCorners[BoxCornerSpecifier::FRONT|BoxCornerSpecifier::TOP],
            boxCorners[BoxCornerSpecifier::TOP]
        }},

        // right faces
        AreaTriangle{.mPoints{
            boxCorners[BoxCornerSpecifier::RIGHT|BoxCornerSpecifier::FRONT],
            boxCorners[BoxCornerSpecifier::RIGHT],
            boxCorners[BoxCornerSpecifier::RIGHT|BoxCornerSpecifier::TOP]
        }},
        AreaTriangle{.mPoints{
            boxCorners[BoxCornerSpecifier::RIGHT|BoxCornerSpecifier::FRONT],
            boxCorners[BoxCornerSpecifier::RIGHT|BoxCornerSpecifier::TOP],
            boxCorners[BoxCornerSpecifier::RIGHT|BoxCornerSpecifier::TOP|BoxCornerSpecifier::FRONT]
        }},

        // bottom faces
        AreaTriangle{.mPoints{
            boxCorners[BoxCornerSpecifier::RIGHT|BoxCornerSpecifier::FRONT],
            boxCorners[BoxCornerSpecifier::FRONT],
            boxCorners[0]
        }},
        AreaTriangle{.mPoints{
            boxCorners[BoxCornerSpecifier::RIGHT|BoxCornerSpecifier::FRONT],
            boxCorners[0],
            boxCorners[BoxCornerSpecifier::RIGHT]
        }},

        // top faces
        AreaTriangle{.mPoints{
            boxCorners[BoxCornerSpecifier::TOP],
            boxCorners[BoxCornerSpecifier::RIGHT|BoxCornerSpecifier::TOP],
            boxCorners[BoxCornerSpecifier::RIGHT|BoxCornerSpecifier::TOP|BoxCornerSpecifier::FRONT]
        }},
        AreaTriangle{.mPoints{
            boxCorners[BoxCornerSpecifier::TOP],
            boxCorners[BoxCornerSpecifier::RIGHT|BoxCornerSpecifier::TOP|BoxCornerSpecifier::FRONT],
            boxCorners[BoxCornerSpecifier::TOP|BoxCornerSpecifier::FRONT]
        }},

        // back faces
        AreaTriangle{.mPoints{
            boxCorners[0],
            boxCorners[BoxCornerSpecifier::RIGHT],
            boxCorners[BoxCornerSpecifier::RIGHT|BoxCornerSpecifier::TOP]
        }},
        AreaTriangle{.mPoints{
            boxCorners[0],
            boxCorners[BoxCornerSpecifier::RIGHT|BoxCornerSpecifier::TOP],
            boxCorners[BoxCornerSpecifier::TOP]
        }},

        // front faces
        AreaTriangle{.mPoints{
            boxCorners[BoxCornerSpecifier::RIGHT|BoxCornerSpecifier::FRONT],
            boxCorners[BoxCornerSpecifier::FRONT],
            boxCorners[BoxCornerSpecifier::TOP|BoxCornerSpecifier::FRONT]
        }},
        AreaTriangle{.mPoints{
            boxCorners[BoxCornerSpecifier::RIGHT|BoxCornerSpecifier::FRONT],
            boxCorners[BoxCornerSpecifier::TOP|BoxCornerSpecifier::FRONT],
            boxCorners[BoxCornerSpecifier::RIGHT|BoxCornerSpecifier::TOP|BoxCornerSpecifier::FRONT]
        }}
    }};
}

std::array<glm::vec3, 8> ObjectBounds::getVolumeRelativeBoxCorners() const {
    switch(mType) {
        case TrueVolumeType::BOX:
            return mTrueVolume.mBox.getVolumeRelativeBoxCorners();
        case TrueVolumeType::SPHERE:
            return mTrueVolume.mSphere.getVolumeRelativeBoxCorners();
        case TrueVolumeType::CAPSULE:
            return mTrueVolume.mCapsule.getVolumeRelativeBoxCorners();
        default:
            assert(false && "Unrecognized true volume type specified");
            return mTrueVolume.mBox.getVolumeRelativeBoxCorners();
    }
}

glm::vec3 ObjectBounds::getComputedWorldPosition() const {
    return mPosition + getWorldRotationTransform() * mPositionOffset;
}
glm::quat ObjectBounds::getComputedWorldOrientation() const {
    return glm::normalize(glm::quat_cast(getWorldRotationTransform() * getLocalRotationTransform()));
}

std::array<glm::vec3, 8> ObjectBounds::getLocalOrientedBoxCorners() const {
    std::array<glm::vec3, 8> orientedCorners { getVolumeRelativeBoxCorners() };
    for(glm::vec3& localCorner: orientedCorners) {
        localCorner = mPositionOffset + getLocalRotationTransform() * localCorner;
    }
    return orientedCorners;
}

std::array<glm::vec3, 8> ObjectBounds::getWorldOrientedBoxCorners() const {
    std::array<glm::vec3, 8> worldCorners { getLocalOrientedBoxCorners() };
    for(glm::vec3& orientedCorner: worldCorners) {
        orientedCorner = mPosition + getWorldRotationTransform() * orientedCorner;
    }
    return worldCorners;
}

AxisAlignedBounds::AxisAlignedBounds(): 
mExtents { glm::vec3{0.f}, glm::vec3{0.f} }
{}

AxisAlignedBounds::AxisAlignedBounds(const ObjectBounds& objectBounds): AxisAlignedBounds{} {
    Extents axisAlignedExtents{
        glm::vec3{ -std::numeric_limits<float>::infinity() },
        glm::vec3{ std::numeric_limits<float>::infinity()  },
    };
    for(auto& corner: objectBounds.getWorldOrientedBoxCorners()) {
        axisAlignedExtents.first = glm::max(axisAlignedExtents.first, corner);
        axisAlignedExtents.second = glm::min(axisAlignedExtents.second, corner);
    }
    setByExtents(axisAlignedExtents);
}

AxisAlignedBounds::AxisAlignedBounds(const Extents& axisAlignedExtents): AxisAlignedBounds{} {
    setByExtents(axisAlignedExtents);
}

std::array<glm::vec3, 8> AxisAlignedBounds::getAxisAlignedBoxCorners() const {
    std::array<glm::vec3, 8> axisAlignedBoxCorners {};
    /** least significant 3 bits represent corners of a box, where
     * * 0th bit represents x, 1 is right, 0 is left
     * * 1st bit represents y, 1 is up, 0 is down
     * * 2nd bit represents z, 1 is front, 0 is back
     */
    for(uint8_t corner {0}; corner < 8; ++corner) {
        axisAlignedBoxCorners[corner].x = corner&BoxCornerSpecifier::RIGHT? mExtents.first.x: mExtents.second.x;
        axisAlignedBoxCorners[corner].y = corner&BoxCornerSpecifier::TOP? mExtents.first.y: mExtents.second.y;
        axisAlignedBoxCorners[corner].z = corner&BoxCornerSpecifier::FRONT? mExtents.first.z: mExtents.second.z;
    }
    return axisAlignedBoxCorners;
}

AxisAlignedBounds::Extents AxisAlignedBounds::getAxisAlignedBoxExtents() const {
    return mExtents;
}

AxisAlignedBounds AxisAlignedBounds::operator+(const AxisAlignedBounds& other) const {
    Extents ourExtents { getAxisAlignedBoxExtents() };
    const glm::vec3& bottomCorner { ourExtents.second };
    const glm::vec3& topCorner { ourExtents.first };

    Extents otherExtents { other.getAxisAlignedBoxExtents() };
    const glm::vec3& otherBottomCorner { otherExtents.second };
    const glm::vec3& otherTopCorner { otherExtents.first };

    Extents extents {
        {glm::max(topCorner, otherTopCorner)}, {glm::min(bottomCorner, otherBottomCorner)} 
    };

    return { extents };
}

void AxisAlignedBounds::setByExtents(const Extents& axisAlignedExtents) {
    assert(
        axisAlignedExtents.first.x >= axisAlignedExtents.second.x
        && axisAlignedExtents.first.y >= axisAlignedExtents.second.y
        && axisAlignedExtents.first.z >= axisAlignedExtents.second.z
        && "First of extents pair must be right top front corner, and second must be left back right corner"
    );
    mExtents = axisAlignedExtents;
}
