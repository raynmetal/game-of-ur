#ifndef ZOSIGNALS_H
#define ZOSIGNALS_H

#include <memory>
#include <cassert>
#include <functional>
#include <string>
#include <map>
#include <set>

class SignalTracker;
class ISignalObserver;
class ISignal;
template <typename ...TArgs>
class SignalObserver_;
template <typename ...TArgs>
class Signal_;
template <typename ...TArgs>
class SignalObserver;
template <typename ...TArgs>
class Signal;

class ISignalObserver {};
class ISignal {
public:
    virtual void registerObserver(std::weak_ptr<ISignalObserver> observer)=0;
};

template <typename ...TArgs>
class Signal_: public ISignal {
public:
    void emit (TArgs... args);
    void registerObserver(std::weak_ptr<ISignalObserver> observer) override;

private:
    // creation managed through SignalTracker
    Signal_() = default;

    std::set<
        std::weak_ptr<SignalObserver_<TArgs...>>,
        std::owner_less<std::weak_ptr<SignalObserver_<TArgs...>>>
    > mObservers {};

friend class SignalTracker;
friend class Signal<TArgs...>;
};

template <typename ...TArgs>
class SignalObserver_: public ISignalObserver {
public:
    void operator() (TArgs... args);
private:
    SignalObserver_(std::function<void(TArgs...)> callback);
    std::function<void(TArgs...)> mStoredFunction {};
friend class SignalTracker;
friend class SignalObserver<TArgs...>;
};

class SignalTracker {
public:
    SignalTracker();
    // let copy constructor just create its own signal list
    SignalTracker(const SignalTracker& other);
    // copy assignment too
  
    SignalTracker& operator=(const SignalTracker& other);

    SignalTracker(SignalTracker&& other);
    SignalTracker& operator=(SignalTracker&& other);

    void connect(const std::string& theirSignal, const std::string& ourObserver, SignalTracker& other);

private:
    template <typename ...TArgs>
    std::shared_ptr<Signal_<TArgs...>> declareSignal(
        const std::string& signalName
    );

    template <typename ...TArgs>
    std::shared_ptr<SignalObserver_<TArgs...>> declareSignalObserver(
        const std::string& observerName,
        std::function<void(TArgs...)> callbackFunction 
    );

    void garbageCollection();
    std::unordered_map<std::string, std::weak_ptr<ISignalObserver>> mObservers {};
    std::unordered_map<std::string, std::weak_ptr<ISignal>> mSignals {};
friend class ISignal;
friend class IObserver;

template <typename ...TArgs>
friend class SignalObserver;

template <typename ...TArgs>
friend class Signal;
};

template <typename ...TArgs>
class Signal {
public:
    Signal(SignalTracker& owningTracker, const std::string& name); 

    Signal(const Signal& other) = delete;
    Signal(Signal&& other) = delete;
    Signal& operator=(const Signal& other) = delete;
    Signal& operator=(Signal&& other) = delete;

    void emit(TArgs...args);
    void resetSignal(SignalTracker& owningTracker, const std::string& name);

private:
    void registerObserver(SignalObserver<TArgs...>& observer);

    std::shared_ptr<Signal_<TArgs...>> mSignal_;
 
friend class SignalObserver<TArgs...>;
};


template <typename ...TArgs>
class SignalObserver {
public:
    SignalObserver(SignalTracker& owningTracker, const std::string& name, std::function<void(TArgs...)> callback);

    SignalObserver(const SignalObserver& other)=delete;
    SignalObserver(SignalObserver&& other)=delete;
    SignalObserver& operator=(const SignalObserver& other) = delete;
    SignalObserver& operator=(SignalObserver&& other) = delete;

    void resetObserver(SignalTracker& owningTracker, const std::string& name, std::function<void(TArgs...)> callback);

    void connectTo(Signal<TArgs...>& signal);
 
private:
    std::shared_ptr<SignalObserver_<TArgs...>> mSignalObserver_;
 
friend class Signal<TArgs...>;
};

template <typename ...TArgs>
inline void Signal_<TArgs...>::registerObserver(std::weak_ptr<ISignalObserver> observer) {
    assert(!observer.expired() && "Cannot register a null pointer as an observer");
    mObservers.insert(std::static_pointer_cast<SignalObserver_<TArgs...>>(observer.lock()));
}
template <typename ...TArgs>
void Signal_<TArgs...>::emit (TArgs ... args) {
    //observers that will be removed from the list after this signal has been emitted
    std::vector<std::weak_ptr<SignalObserver_<TArgs...>>> expiredObservers {};

    for(auto observer: mObservers) {
        // lock means that this observer is still active
        if(std::shared_ptr<SignalObserver_<TArgs...>> activeObserver = observer.lock()) {
            (*activeObserver)(args...);

        // go to the purge list
        } else {
            expiredObservers.push_back(observer);
        }
    }

    // remove dead observers
    for(auto expiredObserver: expiredObservers) {
        mObservers.erase(expiredObserver);
    }
}

template <typename ...TArgs>
inline SignalObserver_<TArgs...>::SignalObserver_(std::function<void(TArgs...)> callback):
mStoredFunction{ callback }
{}
template <typename ...TArgs>
inline void SignalObserver_<TArgs...>::operator() (TArgs ... args) { 
    mStoredFunction(args...);
}

template <typename ...TArgs>
inline std::shared_ptr<Signal_<TArgs...>> SignalTracker::declareSignal(const std::string& name) {
    std::shared_ptr<ISignal> newSignal { new Signal_<TArgs...>{} };
    mSignals.insert({name, newSignal});
    garbageCollection();
    return std::static_pointer_cast<Signal_<TArgs...>>(newSignal);
}
template <typename ...TArgs>
inline std::shared_ptr<SignalObserver_<TArgs...>> SignalTracker::declareSignalObserver(const std::string& name, std::function<void(TArgs...)> callback) {
    std::shared_ptr<ISignalObserver> newObserver { new SignalObserver_<TArgs...>{callback} };
    mObservers.insert({name, newObserver});
    garbageCollection();
    return std::static_pointer_cast<SignalObserver_<TArgs...>>(newObserver);
}
template <typename ...TArgs>
Signal<TArgs...>::Signal(SignalTracker& owningTracker, const std::string& name) {
    resetSignal(owningTracker, name);
}
template <typename ...TArgs>
void Signal<TArgs...>::emit(TArgs...args) {
    mSignal_->emit(args...);
}
template <typename ...TArgs>
void Signal<TArgs...>::resetSignal(SignalTracker& owningTracker, const std::string& name) {
    mSignal_ = owningTracker.declareSignal<TArgs...>(name);
}
template <typename ...TArgs>
void Signal<TArgs...>::registerObserver(SignalObserver<TArgs...>& observer) {
    mSignal_->registerObserver(observer.mSignalObserver_);
}

template <typename ...TArgs>
SignalObserver<TArgs...>::SignalObserver(SignalTracker& owningTracker, const std::string& name, std::function<void(TArgs...)> callback) {
    resetObserver(owningTracker, name, callback);
};
template <typename ...TArgs>
void SignalObserver<TArgs...>::resetObserver(SignalTracker& owningTracker, const std::string& name, std::function<void(TArgs...)> callback) {
    assert(callback && "Empty callback is not allowed");
    mSignalObserver_ = owningTracker.declareSignalObserver<TArgs...>(name, callback);
}
template <typename ...TArgs>
void SignalObserver<TArgs...>::connectTo(Signal<TArgs...>& signal) {
    signal.registerObserver(*this);
}

#endif
