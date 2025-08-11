#ifndef FOOLSENGINE_RESOURCEDATABASE_H
#define FOOLSENGINE_RESOURCEDATABASE_H

#include <memory>
#include <string>
#include <vector>
#include <tuple>
#include <map>
#include <iostream>
#include <type_traits>

#include <nlohmann/json.hpp>

#include "../registrator.hpp"

namespace ToyMakersEngine {

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
        virtual std::string getResourceTypeName_() const=0;
    protected:
        IResource()=default;
        template <typename TResource>
        static void RegisterResource();
    private:
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
        virtual std::string getResourceConstructorName_() const=0;
        virtual std::shared_ptr<IResource> createResource(const nlohmann::json& methodParameters)=0;
        virtual ~IResourceConstructor()=default;
    protected:
        IResourceConstructor()=default;
        template <typename TResource, typename TResourceConstructor>
        static void RegisterResourceConstructor();
    private:
    friend class ResourceDatabase;
    };

    class ResourceDatabase {
    public:
        static ResourceDatabase& GetInstance();

        template <typename TResource>
        static std::shared_ptr<TResource> GetRegisteredResource(const std::string& resourceName);
        template <typename TResource>
        static std::shared_ptr<TResource> ConstructAnonymousResource(const nlohmann::json& resourceDescription);

        static bool HasResourceDescription(const std::string& resourceName);
        template <typename TResource>
        static bool HasResource(const std::string& resourceName);

        template <typename TResource>
        void registerFactory (const std::string& factoryName, std::unique_ptr<IResourceFactory> pFactory);

        template <typename TResource, typename TResourceConstructor>
        void registerResourceConstructor (const std::string& resourceType, const std::string& methodName, std::unique_ptr<IResourceConstructor> pFactoryMethod);

        static void AddResourceDescription (const nlohmann::json& resourceDescription);


    private:
        static void AssertResourceDescriptionValidity(const nlohmann::json& resourceDescription);

        std::map<std::string, std::unique_ptr<IResourceFactory>> mFactories {};    
        std::map<std::string, std::weak_ptr<IResource>> mResources {};
        std::map<std::string, nlohmann::json> mResourceDescriptions {};

        ResourceDatabase() = default;
    };

    template <typename TDerived>
    class Resource: public IResource {
    public:
        inline std::string getResourceTypeName_() const override { return TDerived::getResourceTypeName(); }
    protected:
        explicit Resource(int explicitlyInitializeMe) { (void)explicitlyInitializeMe/* prevent unused parameter warnings */; s_registrator.emptyFunc(); }
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
        inline std::string getResourceConstructorName_() const override { return TResourceFactoryMethod::getResourceConstructorName(); }
    protected:
        explicit ResourceConstructor(int explicitlyInitializeMe) {
            (void)explicitlyInitializeMe; // prevent unused parameter warnings
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
    std::shared_ptr<TResource> ResourceDatabase::GetRegisteredResource(const std::string& resourceName) {
        ResourceDatabase& resourceDatabase { ResourceDatabase::GetInstance() };
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
                ResourceDatabase::ConstructAnonymousResource<TResource>(resourceDescPair->second)
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
    std::shared_ptr<TResource> ResourceDatabase::ConstructAnonymousResource(const nlohmann::json& resourceDescription) {
        ResourceDatabase& resourceDatabase { ResourceDatabase::GetInstance() };
        std::shared_ptr<IResource> pResource { nullptr };

        AssertResourceDescriptionValidity(resourceDescription);

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
    bool ResourceDatabase::HasResource(const std::string& resourceName) {
        ResourceDatabase& resourceDatabase { GetInstance() };
        bool descriptionPresent { HasResourceDescription(resourceName) };
        bool typeMatched { false };
        bool objectLoaded { false };
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
        std::cout << "Loading resource (" << TResource::getResourceTypeName() << ") : " << nlohmann::to_string(resourceDescription["parameters"]) << "\n";
        return mFactoryMethods.at(resourceDescription["method"].get<std::string>())->createResource(
            resourceDescription["parameters"].get<nlohmann::json>()
        );
    }

    template <typename TDerived>
    void Resource<TDerived>::registerSelf() {
        IResource::RegisterResource<TDerived>();
    }

    template <typename TResource, typename TResourceFactoryMethod>
    void ResourceConstructor<TResource, TResourceFactoryMethod>::registerSelf() {
        // ensure that the associated factory is registered before methods are added to it
        Registrator<Resource<TResource>>& resourceRegistrator { Registrator<Resource<TResource>>::getRegistrator() };
        resourceRegistrator.emptyFunc();
        // actually register this method now
        IResourceConstructor::RegisterResourceConstructor<TResource, TResourceFactoryMethod>();
    }

    template <typename TResource>
    void IResource::RegisterResource() {
        ResourceDatabase::GetInstance().registerFactory<TResource>(TResource::getResourceTypeName(), std::make_unique<ResourceFactory<TResource>>());
    }

    template <typename TResource, typename TResourceConstructor>
    void IResourceConstructor::RegisterResourceConstructor() {
        ResourceDatabase::GetInstance().registerResourceConstructor<TResource, TResourceConstructor>(TResource::getResourceTypeName(), TResourceConstructor::getResourceConstructorName(), std::make_unique<TResourceConstructor>());
    }

    template <typename TResource>
    void ResourceDatabase::registerFactory(const std::string& factoryName, std::unique_ptr<IResourceFactory> pFactory) {
        assert((std::is_base_of<IResource, TResource>::value) && "Resource must be subclass of IResource");
        mFactories[factoryName] = std::move(pFactory);
    }

    template <typename TResource, typename TResourceConstructor>
    void ResourceDatabase::registerResourceConstructor(const std::string& resourceType, const std::string& methodName, std::unique_ptr<IResourceConstructor> pFactoryMethod) {
        assert((std::is_base_of<IResource, TResource>::value) && "Resource must be subclass of IResource");
        assert((std::is_base_of<IResourceConstructor, TResourceConstructor>::value) && "Resource must be subclass of IResourceConstructor");
        mFactories.at(resourceType)->mFactoryMethods[methodName] = std::move(pFactoryMethod);
    }

}

#endif
