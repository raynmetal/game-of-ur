#include <iostream>
#include <random>
#include <sstream>

#include "test_text.hpp"
#include "click_to_roll.hpp"

void ClickToRoll::rollPrimary() {
    mPrimaryDie = mPrimaryDieDistribution(mRandomEngine);
}

void ClickToRoll::rollYesNo() {
    mYesNoDie = mYesNoDieDistribution(mRandomEngine);
}

bool ClickToRoll::onPointerLeftClick() {
    rollPrimary();
    setResultText();
    return true;
}
bool ClickToRoll::onPointerRightClick() {
    rollYesNo();
    setResultText();
    return true;
}

std::shared_ptr<ToyMakersEngine::BaseSimObjectAspect> ClickToRoll::create(const nlohmann::json& jsonAspectProperties) {
    return std::shared_ptr<ClickToRoll>{ new ClickToRoll{} };
}

std::shared_ptr<ToyMakersEngine::BaseSimObjectAspect> ClickToRoll::clone() const {
    return std::shared_ptr<ClickToRoll>{ new ClickToRoll{} };
}

void ClickToRoll::setResultText() {
    std::stringstream stringStream {};
    stringStream << "final result: " << static_cast<int>(getResultScore()) << "\n"
        << "\tprimary roll: " << static_cast<int>(getPrimaryDieState()) << "\n"
        << "\tsecondary roll: " << static_cast<int>(getYesNoDieState()) << "\n"
        << "\troll type: " << static_cast<int>(mRollType);
    getAspect<TestText>().updateText(stringStream.str());
}

uint8_t ClickToRoll::getResultScore() const {
    if(mYesNoDie) {
        if(mPrimaryDie == 4) return 10;
        return mPrimaryDie + 4;
    }
    if(mRollType == RollType::INITIATIVE) {
        return mPrimaryDie;
    }
    return 0;
}
