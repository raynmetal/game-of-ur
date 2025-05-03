#ifndef ZOAPPRENDERDEBUGVIEWER_H
#define ZOAPPRENDERDEBUGVIEWER_H

#include "../engine/sim_system.hpp"
#include "../engine/window_context_manager.hpp"

class RenderDebugViewer: public SimObjectAspect<RenderDebugViewer> {
public:
    inline static std::string getSimObjectAspectTypeName() { return "RenderDebugViewer"; }
    std::shared_ptr<BaseSimObjectAspect> clone() const override;
    static std::shared_ptr<BaseSimObjectAspect> create(const nlohmann::json& jsonAspectProperties);
    SignalObserver<> mObserveWindowResized { *this, "WindowResizedObserved", [this]() { std::cout << "RenderDebugViewer: Window was resized\n"; this->printWindowProps(); } };
    SignalObserver<> mObserveWindowMinimized { *this, "WindowMinimizedObserved", [this]() { std::cout << "RenderDebugViewer: Window was minimized\n"; this->printWindowProps(); } };
    SignalObserver<> mObserveWindowMaximized { *this, "WindowMaximizedObserved", [this]() { std::cout << "RenderDebugViewer: Window was maximized\n"; this->printWindowProps(); } };
    SignalObserver<> mObserveWindowMoved { *this, "WindowMovedObserved", [this]() { std::cout << "RenderDebugViewer: Window was moved\n"; this->printWindowProps(); } };
    SignalObserver<> mObserveWindowMouseEntered { *this, "WindowMouseEnteredObserved", [this]() { std::cout << "RenderDebugViewer: Mouse entered window\n"; this->printWindowProps(); } };
    SignalObserver<> mObserveWindowMouseExited { *this, "WindowMouseExitedObserved", [this]() { std::cout << "RenderDebugViewer: Mouse left window\n"; this->printWindowProps(); } };
    SignalObserver<> mObserveWindowCloseRequested { *this, "WindowCloseRequestedObserved", [this]() { std::cout << "RenderDebugViewer: Window close requested\n"; this->printWindowProps(); } };
    SignalObserver<> mObserveWindowSizeChanged { *this, "WindowSizeChangedObserved", [this]() { std::cout << "RenderDebugViewer: Window's size was changed\n"; this->printWindowProps(); } };
    SignalObserver<> mObserveWindowRestored { *this, "WindowRestoredObserved", [this]() { std::cout << "RenderDebugViewer: Window was restored\n"; this->printWindowProps(); } };
    SignalObserver<> mObserveWindowShown { *this, "WindowShownObserved", [this]() { std::cout << "RenderDebugViewer: Window was shown\n"; this->printWindowProps(); } };
    SignalObserver<> mObserveWindowExposed { *this, "WindowExposedObserved", [this]() { std::cout << "RenderDebugViewer: Window was exposed\n"; this->printWindowProps(); } };
    SignalObserver<> mObserveWindowKeyFocusGained { *this, "WindowKeyFocusGainedObserved", [this]() { std::cout << "RenderDebugViewer: Window gained key focus\n"; this->printWindowProps(); } };
    SignalObserver<> mObserveWindowKeyFocusLost { *this, "WindowKeyFocusLostObserved", [this]() { std::cout << "RenderDebugViewer: Window lost key focus\n"; this->printWindowProps(); } };
    SignalObserver<> mObserveWindowKeyFocusOffered { *this, "WindowKeyFocusOffered", [this]() { std::cout << "RenderDebugViewer: Window was offered key focus\n"; this->printWindowProps(); } };
    void printWindowProps();

protected:
    bool onUpdateGamma(const ActionData& actionData, const ActionDefinition& actionDefinition);
    bool onUpdateExposure(const ActionData& actionData, const ActionDefinition& actionDefinition);
    bool onRenderNextTexture(const ActionData& actionData, const ActionDefinition& actionDefinition);

    std::weak_ptr<FixedActionBinding> handleUpdateGamma{ declareFixedActionBinding (
        "Graphics", "UpdateGamma", [this](const ActionData& actionData, const ActionDefinition& actionDefinition) {
            return this->onUpdateGamma(actionData, actionDefinition);
        }
    )};
    std::weak_ptr<FixedActionBinding> handleUpdateExposure { declareFixedActionBinding (
        "Graphics", "UpdateExposure", [this](const ActionData& actionData, const ActionDefinition& actionDefinition) {
            return this->onUpdateExposure(actionData, actionDefinition);
        }
    )};
    std::weak_ptr<FixedActionBinding> handleRenderNextTexture { declareFixedActionBinding(
        "Graphics", "RenderNextTexture", [this](const ActionData& actionData, const ActionDefinition& actionDefinition) {
            return this->onRenderNextTexture(actionData, actionDefinition);
        }
    )};

private:

    RenderDebugViewer() : SimObjectAspect<RenderDebugViewer>{0} {
        WindowContext& windowContextManager { WindowContext::getInstance() };
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
