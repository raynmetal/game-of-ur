#ifndef ZOSHAPEGEN_H
#define ZOSHAPEGEN_H

#include "model.hpp"
#include "mesh.hpp"
#include "mesh_manager.hpp"
#include "model_manager.hpp"

MeshHandle generateSphereMesh(int nLatitude, int nMeridian);
ModelHandle generateSphereModel(int nLatitude, int nMeridian);

MeshHandle generateRectangleMesh(float width=2.f, float height=2.f);
ModelHandle generateRectangleModel(float width=2.f, float height=2.f);

#endif
