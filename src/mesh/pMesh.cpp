#include <vector>

#include "mesh/pMesh.h"
#include <algorithm>

pMesh::pMesh(const Mesh &source, float distance)
	: original(source)
{
	for (auto &v : original.getVertices())
		v.alive = true;

	maxVerts = original.NumVerts();
	progressive = std::make_unique<Mesh>(original);
	Initialize();
}

// pMesh::~pMesh() = default;

void pMesh::Initialize()
{
	history.clear();
	currentHistoryIndex = 0;

	while (progressive->NumVerts() > 3)
	{
		VertexID u = progressive->cheapestVertex();
		if (u < 0) {
			break;
		}

		VertexID v = progressive->getVertices()[u].destiny;
		history.push_back({u, v});
		progressive->edgeCollapse(u, v);
	}

	Reset();
}

void pMesh::Draw(GLuint programID, const glm::mat4 &MVP)
{
	progressive->Draw(programID, MVP);
}

// for split and collapse
void pMesh::Update(int targetVerts)
{
	while (progressive->NumVerts() > targetVerts &&
		   currentHistoryIndex < history.size())
	{
		auto &h = history[currentHistoryIndex++];
		progressive->edgeCollapse(h.from, h.to);
	}

	while (progressive->NumVerts() < targetVerts &&
		   currentHistoryIndex > 0)
	{
		auto &h = history[--currentHistoryIndex];
		progressive->vertexSplit(h.from, h.to);
	}

	progressive->updateVBO();
}

void pMesh::Reset()
{
	progressive = std::make_unique<Mesh>(original);
	currentHistoryIndex = 0;

	for (auto &tri : progressive->getTriangles())
		tri.verts = tri.originalVerts;

	progressive->updateVBO();
}

void pMesh::UpdateToStep(int stepIndex)
{
	// clamp index
	stepIndex = std::clamp(stepIndex, 0, static_cast<int>(history.size()));

	currentHistoryIndex = 0;
	progressive = std::make_unique<Mesh>(original);

	// apply collapses up to stepIndex
	for (int i = 0; i < stepIndex; ++i)
	{
		auto &h = history[i];
		progressive->edgeCollapse(h.from, h.to);
		currentHistoryIndex++;
	}

	progressive->updateVBO();
}