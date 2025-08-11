#include <string>

#include "ui_button.hpp"
#include "ui_text.hpp"
#include "ur_scene_manager.hpp"
#include "ur_records.hpp"

#include "ur_ui_records_browser.hpp"

std::shared_ptr<ToyMakersEngine::BaseSimObjectAspect> UrUIRecordsBrowser::create(const nlohmann::json& jsonAspectProperties) {
    (void)jsonAspectProperties; // prevent unused parameter warnings
    std::shared_ptr<UrUIRecordsBrowser> recordsBrowser { std::make_shared<UrUIRecordsBrowser>() };
    return recordsBrowser;
}

std::shared_ptr<ToyMakersEngine::BaseSimObjectAspect> UrUIRecordsBrowser::clone() const {
    std::shared_ptr<UrUIRecordsBrowser> recordsBrowser { std::make_shared<UrUIRecordsBrowser>() };
    return recordsBrowser;
}

void UrUIRecordsBrowser::onActivated() {
    refreshRecords();
    openPage(mPage);
}

void UrUIRecordsBrowser::onButtonClicked(const std::string& button) {
    if(button == "next") {
        openPage(mPage + 1);
        return;
    }

    if(button == "previous") {
        openPage(mPage - 1);
        return;
    } 

    openDetailedRecord(std::stoi(button));
}

void UrUIRecordsBrowser::openPage(uint32_t page) {
    assert(hasPage(page) && "No such page exists");
    mPage = page;

    const std::size_t baseEntryIndex { mFetchedRecords.size() - page*5 };
    auto entry { mFetchedRecords.rbegin() + 5*page };
    auto end { mFetchedRecords.rend() };
    for(uint32_t entryIndex{0}; entryIndex < 5; ++entryIndex) {
        UIButton& recordButton { getSimObject().getByPath<UIButton&>(
                "/viewport_UI/record_" 
                + std::to_string(entryIndex)
                + "/@UIButton"
            )
        };
        if(entry != end) {
            recordButton.updateText(
                std::to_string(baseEntryIndex - entryIndex) + ". "
                + std::string{"Player A: "}
                + std::to_string(static_cast<int>(entry->mPlayerA.mCounters))
                + (entry->mPlayerA.mIsWinner? " W || ": " L || ")
                + std::string { "Player B: "}
                + std::to_string(static_cast<int>(entry->mPlayerB.mCounters))
                + (entry->mPlayerB.mIsWinner? " W": " L")
            );
            recordButton.enableButton();
            ++entry;
        } else {
            recordButton.updateText("NA");
            recordButton.disableButton();
        }
    }
    
    UIButton& next {
        getSimObject().getByPath<UIButton&>(
            "/viewport_UI/next/@UIButton"
        )
    };
    UIButton& prev {
        getSimObject().getByPath<UIButton&>(
            "/viewport_UI/previous/@UIButton"
        )
    };

    if(hasPage(page+1)) {
        next.enableButton();
    } else {
        next.disableButton();
    }

    if(hasPage(page-1)) {
        prev.enableButton();
    } else {
        prev.disableButton();
    }

}

bool UrUIRecordsBrowser::hasPage(uint32_t page) const {
    const std::size_t totalPages{ 1 + mFetchedRecords.size()/5 };
    return page < totalPages;
}

void UrUIRecordsBrowser::refreshRecords() {
    mFetchedRecords = getSimObject().getWorld().lock()
        ->getSingletonSystem<ToyMakersEngine::SceneSystem>()
        ->getByPath<UrRecords&>(
            "/ur_records/@UrRecords"
        ).getAllRecords();
}


bool UrUIRecordsBrowser::onCancel(const ToyMakersEngine::ActionData& actionData, const ToyMakersEngine::ActionDefinition& actionDefinition) {
    (void)actionDefinition; // prevent unused parameter warnings
    (void)actionData; // prevent unused parameter warnings
    if(mMode == Mode::BROWSE) { return false; };

    closeDetailedRecord();
    return true;
}

void UrUIRecordsBrowser::openDetailedRecord(uint32_t entry) {
    // Disable all the buttons on the records browser
    UIButton& next {
        getSimObject().getByPath<UIButton&>(
            "/viewport_UI/next/@UIButton"
        )
    };
    UIButton& prev {
        getSimObject().getByPath<UIButton&>(
            "/viewport_UI/previous/@UIButton"
        )
    };
    for(uint32_t entryIndex{0}; entryIndex < 5; ++entryIndex) {
        UIButton& recordButton { getSimObject().getByPath<UIButton&>(
                "/viewport_UI/record_" 
                + std::to_string(entryIndex)
                + "/@UIButton"
            )
        };
        recordButton.disableButton();
    }
    next.disableButton();
    prev.disableButton();

    // update record details
    std::shared_ptr<ToyMakersEngine::SceneNode> recordDetails {
        getSimObject().getByPath("/viewport_UI/record_details/")
    };
    const std::size_t selectedEntryIndex { mFetchedRecords.size() - (mPage*5 + entry) };
    UIText& recordTitle {
        recordDetails->getByPath<UIText&>("/title/@UIText")
    };
    recordTitle.updateText(
        std::string{"Game "} + std::to_string(selectedEntryIndex)
    );
    auto selectedRecord { mFetchedRecords.rbegin() + 5*mPage + entry };
    for(uint8_t player {0}; player < 2; ++player) {
        const char playerChar { static_cast<char>('a' + player) };
        const std::string playerDetailPath { std::string{"/player_"} + playerChar + "/" };
        const PlayerData& playerData {
            (playerChar == 'a')? selectedRecord->mPlayerA: selectedRecord->mPlayerB
        };
        std::shared_ptr<ToyMakersEngine::SceneNode> playerSection {
            recordDetails->getByPath(playerDetailPath)
        };
        UIText& name { playerSection->getByPath<UIText&>("/name/@UIText") };
        UIText& role { playerSection->getByPath<UIText&>("/role/@UIText") };
        UIText& counters { playerSection->getByPath<UIText&>("/counters/@UIText") };
        UIText& victoryPieces { playerSection->getByPath<UIText&>("/victory_pieces/@UIText") };
        UIText& onBoardPieces { playerSection->getByPath<UIText&>("/on_board_pieces/@UIText") };
        UIText& unlaunchedPieces { playerSection->getByPath<UIText&>("/unlaunched_pieces/@UIText") };

        name.updateText(
            std::string{"Player "} 
            + static_cast<char>(toupper(playerChar))
            + ((playerData.mIsWinner)? " (Winner)": "")
        );
        role.updateText(
            (playerData.mRole== RoleID::BLACK)? "Black": "White"
        );
        counters.updateText(
            std::string{"Counters: "}
            + std::to_string(static_cast<int>(playerData.mCounters))
        );
        victoryPieces.updateText(
            std::string{"Victory Pieces: "}
            + std::to_string(static_cast<int>(playerData.mNVictoryPieces))
        );
        onBoardPieces.updateText(
            std::string{"Pieces On Board: "}
            + std::to_string(static_cast<int>(playerData.mNBoardPieces))
        );
        unlaunchedPieces.updateText(
            std::string{"Pieces Unlaunched: "}
            + std::to_string(static_cast<int>(playerData.mNUnlaunchedPieces))
        );
    }

    ToyMakersEngine::Placement recordDetailPlacement {
        recordDetails->getComponent<ToyMakersEngine::Placement>()
    };
    recordDetailPlacement.mPosition.z = 10.f;
    recordDetails->updateComponent(recordDetailPlacement);

    mMode = Mode::DETAIL;
}

void UrUIRecordsBrowser::closeDetailedRecord() {
    std::shared_ptr<ToyMakersEngine::SceneNode> recordDetails { getSimObject().getByPath("/viewport_UI/record_details/") };
    ToyMakersEngine::Placement recordDetailsPlacement { recordDetails->getComponent<ToyMakersEngine::Placement>() };
    recordDetailsPlacement.mPosition.z = -10.f;
    recordDetails->updateComponent<ToyMakersEngine::Placement>(recordDetailsPlacement);

    openPage(mPage);
    mMode = Mode::BROWSE;
}
