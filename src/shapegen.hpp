#ifndef ZOSHAPEGEN_H
#define ZOSHAPEGEN_H

#include "vertex.hpp"
#include "model.hpp"
#include "mesh.hpp"

Mesh generateSphereMesh(int nLatitude, int nMeridian);
Model generateSphereModel(int nLatitude, int nMeridian);

Mesh generateRectangleMesh(float width=2.f, float height=2.f);
Model generateRectangleModel(float width=2.f, float height=2.f);

#endif
