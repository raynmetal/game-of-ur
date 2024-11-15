#ifndef ZORESOURCEDATABASE_H
#define ZORESOURCEDATABASE_H

#include <memory>
#include <string>
#include <vector>
#include <tuple>
#include <map>
#include <iostream>

#include <nlohmann/json.hpp>

// ##########################################################################################
// DECLARATIONS
// ##########################################################################################

class IResource;
class IResourceFactory;
class IResourceFactoryMethod;

class ResourceDatabase;
template <typename TResource> class ResourceFactory;
template <typename TResource> class Resource;
template <typename TResource, typename TMethod> class ResourceFactoryMethod;

// ##########################################################################################
// CLASS DEFINITIONS
// ##########################################################################################

/**  
 *  Forces implementation of static function `registerSelf()` using CRTP, called here.
 * 
 * ## Usage:
   ```c++
   class YourClass {
       // ... the rest of your class definition
       // ...
       YourClass() {} // Explicit constructor definition
  
       static YourReturnType registerSelf() {
           // ... whatever the class needs to do to register itself wherever
           // it needs to be registered
       }
  
       inline static Registrator<YourClass> s_registrator { Registrator<YourClass>{} };
   }
   ```
 */
template<typename TRegisterable>
class Registrator {
public:
    inline static Registrator& getRegistrator() {
        static Registrator reg {};
        return reg;
    };
    void emptyFunc() {}
protected:
    Registrator();
};

class IResource {
public:
    virtual ~IResource()=default;
};

class IResourceFactory {
public:
    virtual ~IResourceFactory()=default;
    virtual std::shared_ptr<IResource> createResource(const nlohmann::json& resourceDescription)=0;
protected:

    std::map<std::string, std::unique_ptr<IResourceFactoryMethod>> mFactoryMethods {};
private:

friend class ResourceDatabase;
};

class IResourceFactoryMethod {
public:
    virtual ~IResourceFactoryMethod()=default;
    virtual std::shared_ptr<IResource> createResource(const nlohmann::json& methodParameters)=0;
private:

friend class ResourceDatabase;
};

class ResourceDatabase {
public:
    static ResourceDatabase& getInstance();

    template <typename TResource>
    static std::shared_ptr<TResource> getResource(const std::string& resourceName);

    static bool hasResourceDescription(const std::string& resourceName);
    template <typename TResource>
    static bool hasResource(const std::string& resourceName);

    static void registerFactory (const std::string& factoryName, std::unique_ptr<IResourceFactory> pFactory);
    static void registerFactoryMethod (const std::string& resourceType, const std::string& methodName, std::unique_ptr<IResourceFactoryMethod> pFactoryMethod);
    static void addResourceDescription (const nlohmann::json& resourceDescription);

private:

    std::map<std::string, std::unique_ptr<IResourceFactory>> mFactories {};    
    std::map<std::string, std::weak_ptr<IResource>> mResources {};
    std::map<std::string, nlohmann::json> mResourceDescriptions {};

    ResourceDatabase() = default;
};

template <typename TDerived>
class Resource: public IResource {
public:

protected:
    Resource(int explicitlyInitializeMe) { s_registrator.emptyFunc(); }
private:

    static void registerSelf();
    inline static Registrator<Resource<TDerived>>& s_registrator { Registrator<Resource<TDerived>>::getRegistrator() };

friend class Registrator<Resource<TDerived>>;
};

template<typename TResource> 
class ResourceFactory: public IResourceFactory {
public:
    ResourceFactory() { /* IMPORTANT: do not remove */ }
private:
    std::shared_ptr<IResource> createResource(const nlohmann::json& resourceDescription) override;
};

template<typename TResource, typename TResourceFactoryMethod>
class ResourceFactoryMethod: public IResourceFactoryMethod {
public:

protected:
    ResourceFactoryMethod(int explicitlyInitializeMe) {
        s_registrator.emptyFunc();
    }

private:
    static void registerSelf();

    inline static Registrator<ResourceFactoryMethod<TResource, TResourceFactoryMethod>> s_registrator {
        Registrator<ResourceFactoryMethod<TResource, TResourceFactoryMethod>>::getRegistrator()
    };

friend class Registrator<ResourceFactoryMethod<TResource, TResourceFactoryMethod>>;
friend class ResourceFactory<TResource>;
};

// ##########################################################################################
// FUNCTION DEFINITIONS
// ##########################################################################################

template <typename TRegisterable>
Registrator<TRegisterable>::Registrator() {
    TRegisterable::registerSelf();
}

template <typename TResource>
std::shared_ptr<TResource> ResourceDatabase::getResource(const std::string& resourceName) {
    ResourceDatabase& resourceDatabase { ResourceDatabase::getInstance() };
    std::shared_ptr<IResource> pResource { nullptr };

    // Search known resources first and validate it against the requested type
    std::map<std::string, nlohmann::json>::iterator resourceDescPair { resourceDatabase.mResourceDescriptions.find(resourceName) };
    assert(
        resourceDescPair != resourceDatabase.mResourceDescriptions.end() 
        && "No resource with this name was found amongst known resources"
    );
    assert(
        resourceDescPair->second["type"].get<std::string>() == TResource::getName()
        && "The type of resource requested does not match the type of resource as declared \
        in its description"
    );

    // Look through resources currently in memory
    {
        std::map<std::string, std::weak_ptr<IResource>>::iterator resourcePair { resourceDatabase.mResources.find(resourceName) };
        if(resourcePair != resourceDatabase.mResources.end()) {
            // If a corresponding entry was found but the resource itself had already been moved
            // out of memory, remove the entry
            if(resourcePair->second.expired()) {
                resourceDatabase.mResources.erase(resourcePair->first);

            // We got lucky, and this resource is still in memory
            } else {
                pResource = resourcePair->second.lock();
            }
        }
    }

    // If we still haven't got an in-memory copy, construct this resource using its description,
    // as registered in our resource description table 
    if(!pResource) {
        pResource = resourceDatabase
            .mFactories.at(resourceDescPair->second["type"].get<std::string>())
            ->createResource(resourceDescPair->second);
        assert(pResource && "Resource was not constructed successfully");
        resourceDatabase.mResources[resourceName] = std::weak_ptr<IResource>(pResource);
    }

    // Finally, cast it down to the requested type and hand it over to whoever asked
    // 
    // TODO: how can we prevent unwanted modification of the resource by one of its 
    // users?
    return std::static_pointer_cast<TResource, IResource>(pResource);
}

template <typename TResource>
std::shared_ptr<IResource> ResourceFactory<TResource>::createResource(const nlohmann::json& resourceDescription) {
    return mFactoryMethods.at(resourceDescription["method"].get<std::string>())->createResource(
        resourceDescription["parameters"].get<nlohmann::json>()
    );
}

template <typename TDerived>
void Resource<TDerived>::registerSelf() {
    ResourceDatabase::registerFactory(TDerived::getName(), std::make_unique<ResourceFactory<TDerived>>());
}

template <typename TResource, typename TResourceFactoryMethod>
void ResourceFactoryMethod<TResource, TResourceFactoryMethod>::registerSelf() {
    // ensure that the associated factory is registered before methods are added to it
    Registrator<Resource<TResource>>& resourceRegistrator { Registrator<Resource<TResource>>::getRegistrator() };
    resourceRegistrator.emptyFunc();
    // actually register this method now
    ResourceDatabase::registerFactoryMethod(TResource::getName(), TResourceFactoryMethod::getName(), std::make_unique<TResourceFactoryMethod>());
}
template<typename TResource>
bool ResourceDatabase::hasResource(const std::string& resourceName) {
    ResourceDatabase& resourceDatabase { getInstance() };
    bool descriptionPresent { hasResourceDescription(resourceName) };
    bool typeMatched;
    bool objectLoaded;
    if(descriptionPresent){
        typeMatched = (
            TResource::getName() 
            == resourceDatabase.mResourceDescriptions.at(resourceName).at("type").get<std::string>()
        );
        objectLoaded = (
            resourceDatabase.mResources.find(resourceName)
            != resourceDatabase.mResources.end()
        );
    }
    return (
        descriptionPresent && typeMatched && objectLoaded
    );
}

#endif
