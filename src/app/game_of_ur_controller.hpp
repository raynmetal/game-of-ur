#ifndef ZOAPPGAMEOFURCONTROLLER_H
#define ZOAPPGAMEOFURCONTROLLER_H

#include "glm/gtx/string_cast.hpp"
#include "../engine/sim_system.hpp"

#include "game_of_ur_data/model.hpp"

class GameOfUrController: public ToyMakersEngine::SimObjectAspect<GameOfUrController> {
public:
    GameOfUrController(): SimObjectAspect<GameOfUrController>{0} {}
    inline static std::string getSimObjectAspectTypeName() { return "GameOfUrController"; }
    static std::shared_ptr<BaseSimObjectAspect> create(const nlohmann::json& jsonAspectProperties);
    std::shared_ptr<BaseSimObjectAspect> clone() const override;

    ToyMakersEngine::SignalObserver<glm::u8vec2> mObserveBoardClicked { *this, "BoardClickedObserved", [this](glm::u8vec2 boardLocation) {
        std::cout << "Game of Ur Controller: Board location clicked: \n";
        const HouseData houseData { mModel.getHouseData(boardLocation) };
        std::cout << "\tregion: ";
        switch(houseData.mRegion) {
            case House::Region::BATTLEFIELD: std::cout << "battlefield\n"; break;
            case House::Region::ONE: std::cout << "player one\n"; break;
            case House::Region::TWO: std::cout << "player two\n"; break;
        }
        std::cout << "\tnext cell direction: " << glm::to_string(houseData.mNextCellDirection) << "\n";
        std::cout << "\tlocation: " << glm::to_string(houseData.mLocation) << "\n";
        std::cout << "\toccupant: ";
        if(houseData.mOccupant.mOwner == RoleID::NA) { std::cout << "NA\n"; }
        else {
            std::cout << ((houseData.mOccupant.mOwner==RoleID::ONE)? "p1 ": "p2 ");
            switch(houseData.mOccupant.mType) {
                case PieceTypeID::SWALLOW: std::cout << "swallow\n"; break;
                case PieceTypeID::STORMBIRD: std::cout << "storm bird\n"; break;
                case PieceTypeID::RAVEN: std::cout << "raven\n"; break;
                case PieceTypeID::ROOSTER: std::cout << "rooster\n"; break;
                case PieceTypeID::EAGLE: std::cout << "eagle\n"; break;
                default: assert(false && "We should never get here"); break;
            }
        }
        std::cout << "\ttype: " << ((houseData.mType == House::Type::REGULAR)? "regular": "rosette") << "\n";
    }};
private:
    GameOfUrModel mModel {};
};

#endif
