#ifndef ZOSIMPLEECSRESOURCECOMPONENTEXT_H
#define ZOSIMPLEECSRESOURCECOMPONENTEXT_H
#include  <type_traits>

#include <nlohmann/json.hpp>

#include "simple_ecs.hpp"
#include "resource_database.hpp"

template <typename TResource>
struct ComponentFromJSON<std::shared_ptr<TResource>,
    typename std::enable_if<std::is_base_of<
        IResource, TResource
    >::value>::type> {

    static std::shared_ptr<TResource> get(const nlohmann::json& jsonResource) {
        return ResourceDatabase::getResource<TResource>(
            jsonResource.at("resourceName")
        );
    }

};

#endif
