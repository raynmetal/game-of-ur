#ifndef ZOAPPUIRECORDSBROWSER_H
#define ZOAPPUIRECORDSBROWSER_H

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>
#include "ur_records.hpp"

#include "../engine/sim_system.hpp"

class UrUIRecordsBrowser: public ToyMakersEngine::SimObjectAspect<UrUIRecordsBrowser> {
public:
    enum class Mode {
        BROWSE,
        DETAIL,
    };
    UrUIRecordsBrowser(): ToyMakersEngine::SimObjectAspect<UrUIRecordsBrowser>{0} {}
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

    bool onCancel(const ToyMakersEngine::ActionData& actionData, const ToyMakersEngine::ActionDefinition& actionDefinition);

    std::weak_ptr<ToyMakersEngine::FixedActionBinding> handleCancel { declareFixedActionBinding(
        "General", "Cancel", [this](const ToyMakersEngine::ActionData& actionData, const ToyMakersEngine::ActionDefinition& actionDefinition) {
            return this->onCancel(actionData, actionDefinition);
        }
    )};
    std::weak_ptr<ToyMakersEngine::FixedActionBinding> handlerLeftRelease {
        declareFixedActionBinding(
            "UI", "Untap", [this](const ToyMakersEngine::ActionData& actionData, const ToyMakersEngine::ActionDefinition& actionDefinition) {
                return this->onCancel(actionData, actionDefinition);
            }
        )
    };
public:
    ToyMakersEngine::SignalObserver<const std::string&> mObserveButtonClicked {
        *this, "ButtonClickedObserved",
        [this](const std::string& button) { this->onButtonClicked(button); }
    };
};

#endif
