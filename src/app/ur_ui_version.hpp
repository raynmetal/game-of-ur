/**
 * @file ur_ui_version.hpp
 * @author Zoheb Shujauddin (zoheb2424@gmail.com)
 * @brief Contains the definition for the class responsible for displaying the game version text on the main menu of the game.
 * @version 0.3.2
 * @date 2025-09-13
 * 
 * 
 */

#ifndef ZOAPPUIVERSIONTEXT_H
#define ZOAPPUIVERSIONTEXT_H

#include "toymaker/sim_system.hpp"

class UrUIVersion: public ToyMaker::SimObjectAspect<UrUIVersion> {
public:
    UrUIVersion(): SimObjectAspect<UrUIVersion>{0} {}
    inline static std::string getSimObjectAspectTypeName() { return "UrUIVersion"; }
    static std::shared_ptr<BaseSimObjectAspect> create(const nlohmann::json& jsonAspectProperties);
    std::shared_ptr<BaseSimObjectAspect> clone() const override;

    void onActivated() override;
private:
};


#endif
