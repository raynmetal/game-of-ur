#ifndef ZORESOURCEMANAGER_H
#define ZORESOURCEMANAGER_H

#include <stdexcept>
#include <string>
#include <sstream>
#include <vector>
#include <random>
#include <map>

class IResource {
protected:
    /*
    Destroys the resource tied to this object, which could be a file handle,
    dynamically allocated memory, an open file handle, a device, and so on.
    */
    virtual void destroyResource() = 0;

    /*
    Removes reference to (but does NOT destroy) the resource tied to this
    object (such as dynamically allocated memory, an open file handle, 
    a device, and so on)
    */
    virtual void releaseResource() = 0;
};

template <typename T>
class ResourceHandle;

template<typename T>
class ResourceManager {
public:
    /* Destructor for this resource manager */
    virtual ~ResourceManager() = default;

    /* Returns a static reference to the only existing instance of this manager. If it does not exist,
    it will be created here */
    static ResourceManager& getInstance();

    /* Moves management of some raw resource to this manager. The reference will be
    empty after this operation is completed.

    If another resource under this name is already present when this function is called, returns
    a reference to that resource, and destroys the resource passed as argument */
    ResourceHandle<T> registerResource(const std::string& nameOrLocation, T&& resource);

    /* Returns a handle to the resource represented using this name, if it exists somewhere. 
    otherwise, throws an error */
    ResourceHandle<T> getResourceHandle(const std::string& nameOrLocation);

    /* Removes (via their deconstructors) all resources whose reference count is presently 0*/
    void removeUnusedResources();

private:
    /* increment the reference count for this resource. An empty string
    in the argument is a null operation */
    void incrementReferenceCount(const std::string& nameOrResourceLocation);

    /* decrement the reference count for this resource. An empty string
    in the argument is a null operation */
    void decrementReferenceCount(const std::string& nameOrResourceLocation);

    /* Stores counts of all instances of handles pointing to each resource across the 
    application */
    std::map<const std::string, unsigned long long> mReferenceCounts {};
    /* Stores resources themselves, whose creation and destruction is managed by this
    class */
    std::map<const std::string, T> mResources {};

/* Resource managers and resource handle are tightly coupled */
friend class ResourceHandle<T>;
};

template<typename T>
class ResourceHandle {
public:
    /* decrements reference count in corresponding resource manager */
    ~ResourceHandle();

    /* creates an empty resource handle, callable by anybody */
    ResourceHandle() {};

    /* copy constructor */
    ResourceHandle(const ResourceHandle& other);

    /* move constructor */
    ResourceHandle(ResourceHandle&& other);

    /* copy assignment operator, possible decrement in previously held resource's count, and 
    possible increment in assigned resource's count */
    ResourceHandle& operator=(const ResourceHandle& other);
    /* move assignment operator, possible decrement in previously held resource's count */
    ResourceHandle& operator=(ResourceHandle&& other);

    /* compares equality between two handles of the same underlying resource type */
    bool operator==(const ResourceHandle& other) const;
    /* implementation of operator<, for use with various STL containers. Uses resource handle 
    names for comparisons*/
    bool operator<(const ResourceHandle& other) const;

    /* returns a name string for this handle*/
    std::string getName() const;

    /* returns a reference to the resource pointed to by this handle */
    T& getResource() const;

private:
    /* constructor for resource handle, callable only by the resource manager */
    ResourceHandle(const std::string& name);

    /* the unique name of this resource. May be a filepath, or even a URL, depending on 
    the kind of resource being referred to */
    std::string mName {};

/* Resource managers and resource handles are tightly coupled */
friend class ResourceManager<T>;
};

template <typename T>
ResourceManager<T>& ResourceManager<T>::getInstance() {
    static ResourceManager<T> resourceManager {};
    return resourceManager;
}

template <typename T>
ResourceHandle<T> ResourceManager<T>::getResourceHandle(const std::string& nameOrLocation) {
    if(mResources.find(nameOrLocation) == mResources.end()){
        std::string errorString {"Out of Range: no resource named '"};
        errorString += nameOrLocation + std::string("' exists.");
        throw std::out_of_range(errorString);
    }

    return { nameOrLocation };
}

template <typename T>
ResourceHandle<T> ResourceManager<T>::registerResource(const std::string& nameOrLocation, T&& resource) {
    std::string resourceIdentifier = nameOrLocation;
    // a blank string indicates that we want to create a unique name for this resource
    if(resourceIdentifier == "") {
        resourceIdentifier.resize(18);
        do {

            static std::default_random_engine randomGenerator;
            static std::uniform_int_distribution<char> letterDistribution(0, 25);
            for(int i = 0; i < 18; ++i) {
                resourceIdentifier[i] = 'a' + letterDistribution(randomGenerator);
            }
        } 
        while(mResources.find(resourceIdentifier) != mResources.end());
    }

    else if(mResources.find(resourceIdentifier) != mResources.end()) {
        resource.destroyResource();
        return { resourceIdentifier };
    }

    //assumes T has implemented move semantics
    mResources[resourceIdentifier] = std::move(resource);

    //redundant call to release()
    resource.releaseResource();
    return { resourceIdentifier };
}

template <typename T>
void ResourceManager<T>::incrementReferenceCount(const std::string& nameOrLocation) {
    if(nameOrLocation == "") return;

    if(mReferenceCounts.find(nameOrLocation) == mReferenceCounts.end()) {
        mReferenceCounts[nameOrLocation] = 0;
    }
    ++mReferenceCounts[nameOrLocation];
}

template <typename T>
void ResourceManager<T>::decrementReferenceCount(const std::string& nameOrLocation) {
    if(nameOrLocation == "") return;

    // This should never happen
    if(mReferenceCounts.find(nameOrLocation) == mReferenceCounts.end() 
        || mReferenceCounts[nameOrLocation] == 0
    ){ 
        std::string errorString {"Out of Range: no resource named '"};
        errorString += nameOrLocation + std::string("' exists.");
        throw std::out_of_range(errorString);
    }

    --mReferenceCounts[nameOrLocation];
}

template <typename T>
void ResourceManager<T>::removeUnusedResources() {
    //Make a list of unused resources, and then...
    std::vector<std::string> resourcesToRemove {};
    for(std::pair<const std::string&, unsigned long long> refCountPair : mReferenceCounts) {
        if(!refCountPair.second) {
            resourcesToRemove.push_back(refCountPair.first);
        }
    }

    //... remove them
    for(const std::string& resourceName : resourcesToRemove) {
        mReferenceCounts.erase(resourceName);
        mResources.erase(resourceName);
    }
}

template <typename T>
std::string ResourceHandle<T>::getName() const {
    return mName;
}

template <typename T>
T& ResourceHandle<T>::getResource() const {
    return ResourceManager<T>::getInstance().ResourceManager<T>::mResources[mName];
}

template <typename T>
ResourceHandle<T>::ResourceHandle(const std::string& nameOrLocation) : mName {nameOrLocation}
{
    ResourceManager<T>::getInstance().ResourceManager<T>::incrementReferenceCount(mName);
}

template <typename T>
ResourceHandle<T>::ResourceHandle(const ResourceHandle<T>& other) : mName {other.mName}
{
    ResourceManager<T>::getInstance().ResourceManager<T>::incrementReferenceCount(mName);
}

template <typename T>
ResourceHandle<T>::ResourceHandle(ResourceHandle<T>&& other) : mName {other.mName} {
    other.mName = "";
}

template <typename T>
ResourceHandle<T>& ResourceHandle<T>::operator=(const ResourceHandle<T>& other) {
    if(this == &other) return *this;

    mName = other.mName;
    ResourceManager<T>::getInstance().ResourceManager<T>::incrementReferenceCount(mName);

    return *this;
}

template <typename T>
ResourceHandle<T>& ResourceHandle<T>::operator=(ResourceHandle<T>&& other) {
    if(this == &other) return *this;

    ResourceManager<T>::getInstance().ResourceManager<T>::decrementReferenceCount(mName);
    mName = other.mName;
    other.mName = "";

    return *this;
}

template <typename T>
bool ResourceHandle<T>::operator==(const ResourceHandle<T>& other) const {
    return mName == other.mName;
}
template <typename T>
bool ResourceHandle<T>::operator<(const ResourceHandle<T>& other) const {
    return mName < other.mName;
}

template <typename T>
ResourceHandle<T>::~ResourceHandle<T>() {
    ResourceManager<T>::getInstance().ResourceManager<T>::decrementReferenceCount(mName);
}

#endif
