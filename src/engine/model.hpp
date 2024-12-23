#ifndef ZOMODEL_H
#define ZOMODEL_H

#include <vector>
#include <string>
#include <map>
#include <unordered_set>

#include <GL/glew.h>
#include <glm/glm.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/material.h>

#include "simple_ecs_resource_component_ext.hpp" // this instead of the regular ECS and resource includes
#include "vertex.hpp"
#include "mesh.hpp"
#include "texture.hpp"
#include "shader_program.hpp"
#include "material.hpp"

/*
 * A class that
 *  a) Loads models from their respective 3D model files
 *  b) Stores references to all the meshes used by the model
 *  c) Stores the hierarchical relationship between the meshes
 *  d) Stores material properties used by shaders for each mesh
 */
class StaticModel : public Resource<StaticModel>{
public:
    inline static std::string getResourceTypeName() { return "StaticModel"; }
    inline static std::string getComponentTypeName() { return "StaticModel"; }
    StaticModel(const std::vector<std::shared_ptr<StaticMesh>>& meshHandles, const std::vector<std::shared_ptr<Material>>& materialHandles);

    /* Model destructor */
    ~StaticModel();

    /* Move constructor */
    StaticModel(StaticModel&& other);
    /* Copy constructor */
    StaticModel(const StaticModel& other);

    /* Move assignment */
    StaticModel& operator=(StaticModel&& other);
    /* Copy assignment */
    StaticModel& operator=(const StaticModel& other);

    std::vector<std::shared_ptr<StaticMesh>> getMeshHandles() const;
    std::vector<std::shared_ptr<Material>> getMaterialHandles() const;

private:
    /*
     * Meshes that make up this model
     */
    std::vector<std::shared_ptr<StaticMesh>> mMeshHandles {};
    /*
     * The materials that correspond to each mesh on this model
     */
    std::vector<std::shared_ptr<Material>> mMaterialHandles {};

    /* 
     * Utility method for destroying resources associated with
     * this model
     */
    void free();

    /*
     * Utility method for taking resources from another instance of
     * this class 
     */
    void stealResources(StaticModel& other);

    /*
     * Utility method for deeply replicating resources from another 
     * instance of this class
     */
    void copyResources(const StaticModel& other);

    void destroyResource();
    void releaseResource();
};

template<>
inline std::shared_ptr<StaticModel> Interpolator<std::shared_ptr<StaticModel>>::operator() (
    const std::shared_ptr<StaticModel>& previousState,
    const std::shared_ptr<StaticModel>& nextState,
    float simulationProgress
) const {
    return nextState;
}

class StaticModelFromFile: public ResourceConstructor<StaticModel, StaticModelFromFile> {
public:
    StaticModelFromFile();
    inline static std::string getResourceConstructorName() { return "fromFile"; }

private:
    std::shared_ptr<IResource> createResource(const nlohmann::json& methodParameters) override;
};

#endif
