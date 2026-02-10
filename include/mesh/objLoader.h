#ifndef OBJLOADER_H
#define OBJLOADER_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <set>

#include "mesh/Mesh.h"
#include <glm/glm.hpp>

bool loadOBJ(const std::string &path,
			 std::vector<Vertex> &vertices,
			 std::vector<Triangle> &triangles,
			 std::vector<unsigned int> &indices);
// bool saveOBJ(Mesh *mesh, const std::string path);

#endif