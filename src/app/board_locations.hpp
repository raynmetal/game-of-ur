#ifndef ZOAPPBOARDLOCATIONS_H
#define ZOAPPBOARDLOCATIONS_H

#include "../engine/sim_system.hpp"

#include "interface_pointer_callback.hpp"

class BoardLocation {
public:
    using LocationDescription=uint8_t;
    enum class CellType: LocationDescription {
        INVALID=0x00,
        PLAYER_ONE=0x40,
        PLAYER_TWO=0x80,
        BATTLEFIELD=0xC0,
    };

    BoardLocation(CellType cellType=CellType::INVALID, uint8_t row=0, uint8_t col=0) {
        setRow(row);
        setCol(col);
        setType(cellType);
    }

    uint8_t getRow() const;
    uint8_t getCol() const;
    CellType getType() const;

    void setRow(uint8_t row);
    void setCol(uint8_t col);
    void setType(CellType type);

private:
    LocationDescription mDescription { 0x0 };

    static const uint8_t kColOffset { 0 };
    static const uint8_t kRowOffset { 4 };

    static const uint8_t kColMask { 0x0E };
    static const uint8_t kRowMask { 0x30 };
    static const uint8_t kCellTypeMask { 0xC0 };
};

class BoardLocations: public ToyMakersEngine::SimObjectAspect<BoardLocations>, public ILeftClickable, public IRightClickable {
public:
    inline static std::string getSimObjectAspectTypeName() { return "BoardLocations"; }
    static std::shared_ptr<BaseSimObjectAspect> create(const nlohmann::json& jsonAspectProperties);
    std::shared_ptr<BaseSimObjectAspect> clone() const override;
    bool onPointerLeftClick(glm::vec4 clickLocation)  override;
    bool onPointerRightClick(glm::vec4 clickLocation) override;

private:
    BoardLocations(): SimObjectAspect<BoardLocations>{0} {}

    std::array<std::array<BoardLocation, 12>, 3> mGrid {{
        { 
            BoardLocation{BoardLocation::CellType::PLAYER_TWO, 0, 0},
            BoardLocation{BoardLocation::CellType::PLAYER_TWO, 0, 1},
            BoardLocation{BoardLocation::CellType::PLAYER_TWO, 0, 2},
            BoardLocation{BoardLocation::CellType::PLAYER_TWO, 0, 3},
            BoardLocation{BoardLocation::CellType::INVALID, 0, 4},
            BoardLocation{BoardLocation::CellType::INVALID, 0, 5},
            BoardLocation{BoardLocation::CellType::INVALID, 0, 6},
            BoardLocation{BoardLocation::CellType::INVALID, 0, 7},
            BoardLocation{BoardLocation::CellType::INVALID, 0, 8},
            BoardLocation{BoardLocation::CellType::INVALID, 0, 9},
            BoardLocation{BoardLocation::CellType::INVALID, 0, 10},
            BoardLocation{BoardLocation::CellType::INVALID, 0, 11},
        },
        {
            BoardLocation{BoardLocation::CellType::BATTLEFIELD, 0, 0},
            BoardLocation{BoardLocation::CellType::BATTLEFIELD, 0, 1},
            BoardLocation{BoardLocation::CellType::BATTLEFIELD, 0, 2},
            BoardLocation{BoardLocation::CellType::BATTLEFIELD, 0, 3},
            BoardLocation{BoardLocation::CellType::BATTLEFIELD, 0, 4},
            BoardLocation{BoardLocation::CellType::BATTLEFIELD, 0, 5},
            BoardLocation{BoardLocation::CellType::BATTLEFIELD, 0, 6},
            BoardLocation{BoardLocation::CellType::BATTLEFIELD, 0, 7},
            BoardLocation{BoardLocation::CellType::BATTLEFIELD, 0, 8},
            BoardLocation{BoardLocation::CellType::BATTLEFIELD, 0, 9},
            BoardLocation{BoardLocation::CellType::BATTLEFIELD, 0, 10},
            BoardLocation{BoardLocation::CellType::BATTLEFIELD, 0, 11},
        },
        { 
            BoardLocation{BoardLocation::CellType::PLAYER_ONE, 0, 0},
            BoardLocation{BoardLocation::CellType::PLAYER_ONE, 0, 1},
            BoardLocation{BoardLocation::CellType::PLAYER_ONE, 0, 2},
            BoardLocation{BoardLocation::CellType::PLAYER_ONE, 0, 3},
            BoardLocation{BoardLocation::CellType::INVALID, 0, 4},
            BoardLocation{BoardLocation::CellType::INVALID, 0, 5},
            BoardLocation{BoardLocation::CellType::INVALID, 0, 6},
            BoardLocation{BoardLocation::CellType::INVALID, 0, 7},
            BoardLocation{BoardLocation::CellType::INVALID, 0, 8},
            BoardLocation{BoardLocation::CellType::INVALID, 0, 9},
            BoardLocation{BoardLocation::CellType::INVALID, 0, 10},
            BoardLocation{BoardLocation::CellType::INVALID, 0, 11},
        },
    }};
};


#endif
