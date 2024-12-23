#ifndef ZORESOURCEDATABASE_H
#define ZORESOURCEDATABASE_H

#include <memory>
#include <string>
#include <vector>
#include <tuple>
#include <map>
#include <iostream>

#include <nlohmann/json.hpp>

#include "registrator.hpp"

// ##########################################################################################
// DECLARATIONS
// ##########################################################################################

class IResource;
class IResourceFactory;
class IResourceConstructor;

class ResourceDatabase;
template <typename TResource> class ResourceFactory;
template <typename TResource> class Resource;
template <typename TResource, typename TMethod> class ResourceConstructor;

// ##########################################################################################
// CLASS DEFINITIONS
// ##########################################################################################
class IResource {
public:
    virtual ~IResource()=default;
};

class IResourceFactory {
public:
    virtual ~IResourceFactory()=default;
    virtual std::shared_ptr<IResource> createResource(const nlohmann::json& resourceDescription)=0;
protected:

    std::map<std::string, std::unique_ptr<IResourceConstructor>> mFactoryMethods {};
private:

friend class ResourceDatabase;
};

class IResourceConstructor {
public:
    virtual ~IResourceConstructor()=default;
    virtual std::shared_ptr<IResource> createResource(const nlohmann::json& methodParameters)=0;
private:

friend class ResourceDatabase;
};

class ResourceDatabase {
public:
    static ResourceDatabase& getInstance();

    template <typename TResource>
    static std::shared_ptr<TResource> getRegisteredResource(const std::string& resourceName);
    template <typename TResource>
    static std::shared_ptr<TResource> constructAnonymousResource(const nlohmann::json& resourceDescription);

    static bool hasResourceDescription(const std::string& resourceName);
    template <typename TResource>
    static bool hasResource(const std::string& resourceName);

    static void registerFactory (const std::string& factoryName, std::unique_ptr<IResourceFactory> pFactory);
    static void registerFactoryMethod (const std::string& resourceType, const std::string& methodName, std::unique_ptr<IResourceConstructor> pFactoryMethod);
    static void addResourceDescription (const nlohmann::json& resourceDescription);


private:
    static void assertResourceDescriptionValidity(const nlohmann::json& resourceDescription);

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
class ResourceConstructor: public IResourceConstructor {
public:

protected:
    ResourceConstructor(int explicitlyInitializeMe) {
        s_registrator.emptyFunc();
    }

private:
    static void registerSelf();

    inline static Registrator<ResourceConstructor<TResource, TResourceFactoryMethod>> s_registrator {
        Registrator<ResourceConstructor<TResource, TResourceFactoryMethod>>::getRegistrator()
    };

friend class Registrator<ResourceConstructor<TResource, TResourceFactoryMethod>>;
friend class ResourceFactory<TResource>;
};

// ##########################################################################################
// FUNCTION DEFINITIONS
// ##########################################################################################
template <typename TResource>
std::shared_ptr<TResource> ResourceDatabase::getRegisteredResource(const std::string& resourceName) {
    ResourceDatabase& resourceDatabase { ResourceDatabase::getInstance() };
    std::shared_ptr<IResource> pResource { nullptr };

    // Search known resources first and validate it against the requested type
    std::map<std::string, nlohmann::json>::iterator resourceDescPair { resourceDatabase.mResourceDescriptions.find(resourceName) };
    assert(
        resourceDescPair != resourceDatabase.mResourceDescriptions.end() 
        && "No resource with this name was found amongst known resources"
    );
    assert(
        resourceDescPair->second["type"].get<std::string>() == TResource::getResourceTypeName()
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
        pResource = std::static_pointer_cast<typename Resource<TResource>::IResource, TResource>(
            ResourceDatabase::constructAnonymousResource<TResource>(resourceDescPair->second)
        );
        resourceDatabase.mResources[resourceName] = std::weak_ptr<IResource>{ pResource };
    }

    // Finally, cast it down to the requested type and hand it over to whoever asked
    // 
    // TODO: how can we prevent unwanted modification of the resource by one of its 
    // users?
    return std::static_pointer_cast<TResource>(std::static_pointer_cast<Resource<TResource>>(pResource));
}

template<typename TResource>
std::shared_ptr<TResource> ResourceDatabase::constructAnonymousResource(const nlohmann::json& resourceDescription) {
    ResourceDatabase& resourceDatabase { ResourceDatabase::getInstance() };
    std::shared_ptr<IResource> pResource { nullptr };

    assertResourceDescriptionValidity(resourceDescription);

    // Construct this resource using its description
    pResource = resourceDatabase
        .mFactories.at(resourceDescription.at("type").get<std::string>())
        ->createResource(resourceDescription);
    assert(pResource && "Resource could not be constructed");

    // Finally, cast it down to the requested type and hand it over to whoever asked
    // 
    // TODO: how can we prevent unwanted modification of the resource by one of its 
    // users?
    return std::static_pointer_cast<TResource>(std::static_pointer_cast<Resource<TResource>>(pResource));
}

template<typename TResource>
bool ResourceDatabase::hasResource(const std::string& resourceName) {
    ResourceDatabase& resourceDatabase { getInstance() };
    bool descriptionPresent { hasResourceDescription(resourceName) };
    bool typeMatched;
    bool objectLoaded;
    if(descriptionPresent){
        typeMatched = (
            TResource::getResourceTypeName() 
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

template <typename TResource>
std::shared_ptr<IResource> ResourceFactory<TResource>::createResource(const nlohmann::json& resourceDescription) {
    return mFactoryMethods.at(resourceDescription["method"].get<std::string>())->createResource(
        resourceDescription["parameters"].get<nlohmann::json>()
    );
}

template <typename TDerived>
void Resource<TDerived>::registerSelf() {
    ResourceDatabase::registerFactory(TDerived::getResourceTypeName(), std::make_unique<ResourceFactory<TDerived>>());
}

template <typename TResource, typename TResourceFactoryMethod>
void ResourceConstructor<TResource, TResourceFactoryMethod>::registerSelf() {
    // ensure that the associated factory is registered before methods are added to it
    Registrator<Resource<TResource>>& resourceRegistrator { Registrator<Resource<TResource>>::getRegistrator() };
    resourceRegistrator.emptyFunc();
    // actually register this method now
    ResourceDatabase::registerFactoryMethod(TResource::getResourceTypeName(), TResourceFactoryMethod::getResourceConstructorName(), std::make_unique<TResourceFactoryMethod>());
}

#endif
