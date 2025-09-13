/**
 * @file ur_ui_records_browser.hpp
 * @author Zoheb Shujauddin (zoheb2424@gmail.com)
 * @brief Contains the definition for the aspect class responsible for displaying records for previously completed games.
 * @version 0.3.2
 * @date 2025-09-13
 * 
 * 
 */

#ifndef ZOAPPUIRECORDSBROWSER_H
#define ZOAPPUIRECORDSBROWSER_H

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>
#include "ur_records.hpp"

#include "toymaker/sim_system.hpp"

class UrUIRecordsBrowser: public ToyMaker::SimObjectAspect<UrUIRecordsBrowser> {
public:
    enum class Mode {
        BROWSE,
        DETAIL,
    };
    UrUIRecordsBrowser(): ToyMaker::SimObjectAspect<UrUIRecordsBrowser>{0} {}
    inline static std::string getSimObjectAspectTypeName() { return "UrUIRecordsBrowser"; }
    static std::shared_ptr<BaseSimObjectAspect> create(const nlohmann::json& jsonAspectProperties);
    std::shared_ptr<BaseSimObjectAspect> clone() const override;


    void loadScene(const std::string& sceneResourceName);

private:
    Mode mMode { Mode::BROWSE };
    uint32_t mPage {0};
    std::vector<GameRecord> mFetchedRecords {};

    void refreshRecords();
    void onButtonClicked(const std::string& button);
    void onActivated() override;
    
    bool hasPage(uint32_t page) const;
    void openPage(uint32_t page);

    void openDetailedRecord(uint32_t entry);
    void closeDetailedRecord();

    bool onCancel(const ToyMaker::ActionData& actionData, const ToyMaker::ActionDefinition& actionDefinition);

    std::weak_ptr<ToyMaker::FixedActionBinding> handleCancel { declareFixedActionBinding(
        "General", "Cancel", [this](const ToyMaker::ActionData& actionData, const ToyMaker::ActionDefinition& actionDefinition) {
            return this->onCancel(actionData, actionDefinition);
        }
    )};
    std::weak_ptr<ToyMaker::FixedActionBinding> handlerLeftRelease {
        declareFixedActionBinding(
            "UI", "Untap", [this](const ToyMaker::ActionData& actionData, const ToyMaker::ActionDefinition& actionDefinition) {
                return this->onCancel(actionData, actionDefinition);
            }
        )
    };
public:
    ToyMaker::SignalObserver<const std::string&> mObserveButtonClicked {
        *this, "ButtonClickedObserved",
        [this](const std::string& button) { this->onButtonClicked(button); }
    };
};

#endif
