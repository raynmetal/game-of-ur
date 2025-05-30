#ifndef FOOLSENGINE_SIMPLEECSRESOURCECOMPONENTEXT_H
#define FOOLSENGINE_SIMPLEECSRESOURCECOMPONENTEXT_H
#include  <type_traits>

#include <nlohmann/json.hpp>

#include "ecs_world.hpp"
#include "resource_database.hpp"

namespace ToyMakersEngine {
    template <typename TResource>
    struct ComponentFromJSON<std::shared_ptr<TResource>,
        typename std::enable_if<std::is_base_of<
            IResource, TResource
        >::value>::type> {

        static std::shared_ptr<TResource> get(const nlohmann::json& jsonResource) {
            return ResourceDatabase::GetRegisteredResource<TResource>(
                jsonResource.at("resourceName")
            );
        }

    };
}

#endif
