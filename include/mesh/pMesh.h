#ifndef PMESH_H
#define PMESH_H

#include <stdio.h>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <memory>

#include "Mesh.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

//====================================================================pEdge Class
struct pVert
{
	VertexID from;
	VertexID to;
};

//=====================================================================pMESH CLASS
class pMesh
{
public:
	explicit pMesh(const Mesh &source, float maxDistance = 100.0f);

	void Initialize();
	void Update(int targetVerts);
	void Reset();

	void Draw(GLuint programID, const glm::mat4 &MVP);

private:
	Mesh original;
	std::unique_ptr<Mesh> progressive;

	float maxD;
	int currentHistoryIndex = 0;

	std::vector<pVert> history;
};
#endif
