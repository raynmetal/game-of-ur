#ifndef ZOAPPRENDERDEBUGVIEWER_H
#define ZOAPPRENDERDEBUGVIEWER_H

#include "../engine/sim_system.hpp"
#include "../engine/window_context_manager.hpp"

class RenderDebugViewer: public ToyMakersEngine::SimObjectAspect<RenderDebugViewer> {
public:
    inline static std::string getSimObjectAspectTypeName() { return "RenderDebugViewer"; }
    std::shared_ptr<BaseSimObjectAspect> clone() const override;
    static std::shared_ptr<BaseSimObjectAspect> create(const nlohmann::json& jsonAspectProperties);
    ToyMakersEngine::SignalObserver<> mObserveWindowResized { *this, "WindowResizedObserved", [this]() { std::cout << "RenderDebugViewer: Window was resized\n"; this->printWindowProps(); } };
    ToyMakersEngine::SignalObserver<> mObserveWindowMinimized { *this, "WindowMinimizedObserved", [this]() { std::cout << "RenderDebugViewer: Window was minimized\n"; this->printWindowProps(); } };
    ToyMakersEngine::SignalObserver<> mObserveWindowMaximized { *this, "WindowMaximizedObserved", [this]() { std::cout << "RenderDebugViewer: Window was maximized\n"; this->printWindowProps(); } };
    ToyMakersEngine::SignalObserver<> mObserveWindowMoved { *this, "WindowMovedObserved", [this]() { std::cout << "RenderDebugViewer: Window was moved\n"; this->printWindowProps(); } };
    ToyMakersEngine::SignalObserver<> mObserveWindowMouseEntered { *this, "WindowMouseEnteredObserved", [this]() { std::cout << "RenderDebugViewer: Mouse entered window\n"; this->printWindowProps(); } };
    ToyMakersEngine::SignalObserver<> mObserveWindowMouseExited { *this, "WindowMouseExitedObserved", [this]() { std::cout << "RenderDebugViewer: Mouse left window\n"; this->printWindowProps(); } };
    ToyMakersEngine::SignalObserver<> mObserveWindowCloseRequested { *this, "WindowCloseRequestedObserved", [this]() { std::cout << "RenderDebugViewer: Window close requested\n"; this->printWindowProps(); } };
    ToyMakersEngine::SignalObserver<> mObserveWindowSizeChanged { *this, "WindowSizeChangedObserved", [this]() { std::cout << "RenderDebugViewer: Window's size was changed\n"; this->printWindowProps(); } };
    ToyMakersEngine::SignalObserver<> mObserveWindowRestored { *this, "WindowRestoredObserved", [this]() { std::cout << "RenderDebugViewer: Window was restored\n"; this->printWindowProps(); } };
    ToyMakersEngine::SignalObserver<> mObserveWindowShown { *this, "WindowShownObserved", [this]() { std::cout << "RenderDebugViewer: Window was shown\n"; this->printWindowProps(); } };
    ToyMakersEngine::SignalObserver<> mObserveWindowExposed { *this, "WindowExposedObserved", [this]() { std::cout << "RenderDebugViewer: Window was exposed\n"; this->printWindowProps(); } };
    ToyMakersEngine::SignalObserver<> mObserveWindowKeyFocusGained { *this, "WindowKeyFocusGainedObserved", [this]() { std::cout << "RenderDebugViewer: Window gained key focus\n"; this->printWindowProps(); } };
    ToyMakersEngine::SignalObserver<> mObserveWindowKeyFocusLost { *this, "WindowKeyFocusLostObserved", [this]() { std::cout << "RenderDebugViewer: Window lost key focus\n"; this->printWindowProps(); } };
    ToyMakersEngine::SignalObserver<> mObserveWindowKeyFocusOffered { *this, "WindowKeyFocusOffered", [this]() { std::cout << "RenderDebugViewer: Window was offered key focus\n"; this->printWindowProps(); } };
    void printWindowProps();

protected:
    bool onUpdateGamma(const ToyMakersEngine::ActionData& actionData, const ToyMakersEngine::ActionDefinition& actionDefinition);
    bool onUpdateExposure(const ToyMakersEngine::ActionData& actionData, const ToyMakersEngine::ActionDefinition& actionDefinition);
    bool onRenderNextTexture(const ToyMakersEngine::ActionData& actionData, const ToyMakersEngine::ActionDefinition& actionDefinition);

    std::weak_ptr<ToyMakersEngine::FixedActionBinding> handleUpdateGamma{ declareFixedActionBinding (
        "Graphics", "UpdateGamma", [this](const ToyMakersEngine::ActionData& actionData, const ToyMakersEngine::ActionDefinition& actionDefinition) {
            return this->onUpdateGamma(actionData, actionDefinition);
        }
    )};
    std::weak_ptr<ToyMakersEngine::FixedActionBinding> handleUpdateExposure { declareFixedActionBinding (
        "Graphics", "UpdateExposure", [this](const ToyMakersEngine::ActionData& actionData, const ToyMakersEngine::ActionDefinition& actionDefinition) {
            return this->onUpdateExposure(actionData, actionDefinition);
        }
    )};
    std::weak_ptr<ToyMakersEngine::FixedActionBinding> handleRenderNextTexture { declareFixedActionBinding(
        "Graphics", "RenderNextTexture", [this](const ToyMakersEngine::ActionData& actionData, const ToyMakersEngine::ActionDefinition& actionDefinition) {
            return this->onRenderNextTexture(actionData, actionDefinition);
        }
    )};

private:

    RenderDebugViewer() : SimObjectAspect<RenderDebugViewer>{0} {
        ToyMakersEngine::WindowContext& windowContextManager { ToyMakersEngine::WindowContext::getInstance() };
        mObserveWindowMoved.connectTo(windowContextManager.mSigWindowMoved);
        mObserveWindowResized.connectTo(windowContextManager.mSigWindowResized);
        mObserveWindowMinimized.connectTo(windowContextManager.mSigWindowMinimized);
        mObserveWindowMaximized.connectTo(windowContextManager.mSigWindowMaximized);
        mObserveWindowMouseEntered.connectTo(windowContextManager.mSigWindowMouseEntered);
        mObserveWindowMouseExited.connectTo(windowContextManager.mSigWindowMouseExited);
        mObserveWindowShown.connectTo(windowContextManager.mSigWindowShown);
        mObserveWindowSizeChanged.connectTo(windowContextManager.mSigWindowSizeChanged);
        mObserveWindowCloseRequested.connectTo(windowContextManager.mSigWindowCloseRequested);
        mObserveWindowRestored.connectTo(windowContextManager.mSigWindowRestored);
        mObserveWindowExposed.connectTo(windowContextManager.mSigWindowExposed);
        mObserveWindowKeyFocusGained.connectTo(windowContextManager.mSigWindowKeyFocusGained);
        mObserveWindowKeyFocusLost.connectTo(windowContextManager.mSigWindowKeyFocusLost);
        mObserveWindowKeyFocusOffered.connectTo(windowContextManager.mSigWindowKeyFocusOffered);
    }

    float mGammaStep { .1f };
    float mExposureStep { .1f };
};

#endif
