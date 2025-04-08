#ifndef ZOSPATIALQUERYOCTREE_H
#define ZOSPATIALQUERYOCTREE_H

#include <memory>
#include <array>

#include "spatial_query_math.hpp"
#include "spatial_query_basic_types.hpp"
#include "ecs_world.hpp"

class Octree;
class OctreeNode: public std::enable_shared_from_this<OctreeNode> {
public:
    using Octant = uint8_t;
    using Depth = uint8_t;
    using Address = uint64_t;

    enum OctantSpecifier: Octant {
        RIGHT=0x1,
        TOP=0x2,
        FRONT=0x4,
    };

    static constexpr uint8_t knDepthBits { 5 };
    static constexpr uint8_t kDepthBitOffset { sizeof(Address)*8 - knDepthBits };
    static constexpr uint8_t knRouteBits { (kDepthBitOffset/3) * 3};
    static constexpr Depth kMaxDepthInclusive { 1 + knRouteBits/3 }; // +1 as depth 0 needs no bits
    static constexpr Address kNoAddress { 0x0 };

    static Address MakeAddress(Octant childOctant, Address parentAddress);
    static Depth GetDepth(Address address);
    static Octant ToGrowthDirection(Octant octant);
    static Octant ToOctant(Octant growthDirection);
    static Octant GetOctant(Address address);
    static Address GetBaseRouteMask(Depth baseDepth);
    static Address GetBaseRoute(Address address, Depth baseDepth);
    static Octant GetOctantAt(Address address, Depth depth);
    static Address GrowAddress(Address address, Address rootAddress);
    static Address ShrinkAddress(Address address, Depth depthRemoved);
    static bool SharesBranch(Address one, Address two);

    enum AddressMasks: Address {
        DEPTH_MASK = -(1ull << kDepthBitOffset), // trick to turn on first n bits of an integer (see 2's complement)
        ROUTE_MASK = ~(-(1ull << knRouteBits)), // avoid erroneous addresses caused by shifting unused bits in address
    };
    static_assert(DEPTH_MASK != 0 && "Depth mask cannot be zero");
    static_assert(ROUTE_MASK != 0 && "Route mask cannot be zero");
    static_assert(knDepthBits + kDepthBitOffset == sizeof(Address)*8 && "sum of depth bits and depth offset must add up to the size of the address in bits");
    static_assert(kDepthBitOffset >= knRouteBits && "There must be at least as many bits in the depth bit offset as those used to make the route");
    static_assert(kNoAddress == 0 && "NoAddress must correspond with 0");

    static std::shared_ptr<OctreeNode> CreateRootNode(
        uint8_t subdivisionThreshold, AxisAlignedBounds boundRegion
    );

    static std::shared_ptr<OctreeNode> GrowTreeAndCreateRoot(
        std::shared_ptr<OctreeNode> oldRoot,
        const AxisAlignedBounds& regionToCover
    );

    std::vector<std::pair<EntityID, AxisAlignedBounds>> findAllMemberEntities() const;
    std::vector<std::pair<EntityID, AxisAlignedBounds>> findEntitiesOverlapping(const AxisAlignedBounds& searchBounds) const;
    std::vector<std::pair<EntityID, AxisAlignedBounds>> findEntitiesOverlapping(const Ray& searchRay) const;

    uint8_t getChildCount() const;
    Address getAddress() const { return mAddress; }
    inline AxisAlignedBounds getWorldBounds() const { return mWorldBounds; }
    Address insertEntity(EntityID entityID, const AxisAlignedBounds& entityWorldBounds);
    std::shared_ptr<OctreeNode> removeEntity(EntityID entityID, Address entityAddressHint=kNoAddress);
    std::shared_ptr<OctreeNode> getNode(Address octantAddress);
    std::shared_ptr<OctreeNode> nextNodeInAddress(Address octantAddress);
    std::shared_ptr<OctreeNode> getSmallestNodeContaining(const AxisAlignedBounds& entityWorldBounds);
    std::shared_ptr<OctreeNode> findCandidateRoot();
    Address getBaseRoute(Address address) const;
    Depth getDepth() const;
    Octant getOctant() const;
    Octant nextOctant(Address address) const;
    void shrinkTreeAndBecomeRoot();

private:
    OctreeNode(
        Address octantAddress,
        uint8_t subdivisionThreshold,
        AxisAlignedBounds worldBounds,
        std::shared_ptr<OctreeNode> parent
    ):
    mAddress { octantAddress },
    mSubdivisionThreshold { subdivisionThreshold },
    mWorldBounds { worldBounds },
    mParent { parent }
    {}

    Address mAddress { 0x0 };
    uint8_t mSubdivisionThreshold { 40 };
    AxisAlignedBounds mWorldBounds {};
    std::weak_ptr<OctreeNode> mParent {};
    std::array<std::shared_ptr<OctreeNode>, 8> mChildren {};
    std::map<EntityID, AxisAlignedBounds> mEntities{};
};

class Octree {
public:
    // the ratio between any two dimensions of an octree can be 
    // no more than this
    static constexpr float kMaxDimensionRatio { 20.f };

    using EntityAddressPair = std::pair<EntityID, OctreeNode::Address>;
    Octree(uint8_t subdivisionThreshold, const AxisAlignedBounds& totalWorldBounds):
    mRootNode { OctreeNode::CreateRootNode(subdivisionThreshold, totalWorldBounds) }
    {}

    void insertEntity(EntityID entityID, const AxisAlignedBounds& entityWorldBounds);
    void removeEntity(EntityID entityID);

private:
    std::shared_ptr<OctreeNode> mRootNode;
    std::map<EntityID, OctreeNode::Address> mEntityAddresses {};
};

#endif
