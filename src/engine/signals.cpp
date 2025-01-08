#include "signals.hpp"

SignalTracker::SignalTracker() = default;

// let copy constructor just create its own signal list
SignalTracker::SignalTracker(const SignalTracker& other): SignalTracker{} {}

// move constructor creates own signal list as well
SignalTracker::SignalTracker(SignalTracker&& other): SignalTracker{} {}

SignalTracker& SignalTracker::operator=(const SignalTracker& other) {
    // Note: There is no point doing any work in this
    // case, as it is the responsibility of the inheritor
    // to make sure signals and connections are correctly
    // reconstructed. Dead signals and observers will 
    // automatically be cleaned up
    return *this;
}
SignalTracker& SignalTracker::operator=(SignalTracker&& other) {
    // Note: same as in copy assignment
    return *this;
}

void SignalTracker::connect(const std::string& theirSignalsName, const std::string& ourObserversName, SignalTracker& other) {

    std::shared_ptr<ISignal> otherSignal { other.mSignals.at(theirSignalsName).lock() };
    assert(otherSignal && "No signal of this name found on other");

    std::shared_ptr<ISignalObserver> ourObserver { mObservers.at(ourObserversName).lock() };
    assert(ourObserver && "No observer of this name present on this object");

    otherSignal->registerObserver(ourObserver);

    garbageCollection();
}

void SignalTracker::garbageCollection() {
    std::vector<std::string> signalsToErase {};
    std::vector<std::string> observersToErase{};
    for(auto& pair: mSignals) {
        if(pair.second.expired()) { 
            signalsToErase.push_back(pair.first);
        }
    }
    for(auto& pair: mObservers) {
        if(pair.second.expired()) {
            observersToErase.push_back(pair.first);
        }
    }

    for(auto& signal: signalsToErase) {
        mSignals.erase(signal);
    }
    for(auto& observer: observersToErase) {
        mObservers.erase(observer);
    }
}
