#ifndef MESH_H
#define MESH_H

#include <stdio.h>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <array>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Vertex;
class Triangle;
// class Edge;

//===========================================================================VERTEX
using VertexID = int;
using TriangleID = int;

struct Vertex
{
	glm::vec3 Position{};
	glm::vec3 Normal{};
	glm::vec2 TexCoords{};

	std::vector<TriangleID> triangles; // indices, not pointers
	std::vector<VertexID> neighbors;

	VertexID destiny = -1;
	bool alive = true;
};
//===========================================================================EDGE

// Maybe removing the edge class entirely will make things easier.

class Edge
{
public:
	Edge(Vertex &s, Vertex &e, Triangle *t);
	//~Edge();

	//============================================================Edge Member Variables
	Edge *next;
	Edge *opposite;

	Vertex *start;
	Vertex *end;

	// list of triangles which contain this edge
	std::vector<Triangle> triangles;

	float length;

	//============================================================Edge Member functions
};

//===========================================================================TRIANGLE
struct Triangle
{
	Triangle() = default;

	Triangle(VertexID a, VertexID b, VertexID c)
	{
		verts = {a, b, c};
		originalVerts = verts;
	}

	std::array<VertexID, 3> verts{};
	std::array<VertexID, 3> originalVerts{};
	glm::vec3 normal{};

	bool isDegenerate() const
	{
		return verts[0] == verts[1] ||
			   verts[1] == verts[2] ||
			   verts[2] == verts[0];
	}

	bool contains(VertexID v) const
	{
		return verts[0] == v || verts[1] == v || verts[2] == v;
	}

	void replace(VertexID from, VertexID to)
	{
		for (auto &v : verts)
			if (v == from)
				v = to;
	}
};

//===========================================================================MESH
class Mesh
{
public:
	Mesh();
	Mesh(const Mesh &other);
	Mesh(const std::string &path);
	Mesh &operator=(const Mesh &other);
	~Mesh();

	void Draw(GLuint programID, const glm::mat4 &MVP);
	void updateVBO();

	// Progressive mesh ops
	void edgeCollapse(VertexID u, VertexID v);
	void vertexSplit(VertexID u, VertexID v);

	VertexID cheapestVertex();
	int NumVerts() const;

	const std::vector<Vertex> &getVertices() const { return vertices; }
	std::vector<Vertex> &getVertices() { return vertices; }

	const std::vector<Triangle> &getTriangles() const { return triangles; }
	std::vector<Triangle> &getTriangles() { return triangles; }

private:
	void setupMesh();
	void destroyGL();

	std::vector<Vertex> vertices;
	std::vector<Triangle> triangles;
	std::vector<GLuint> indices;

	GLuint VAO{0}, VBO{0}, EBO{0};
};

//===================================================Helper Functions================
// compute the cost of an edge collapse
/*
cost n
	u,v u v f normal n normal f Tu Tuv ( ) =−× − • { } { }
*/
inline float Cost(VertexID u, VertexID v, const Mesh &mesh)
{
	const auto &verts = mesh.getVertices();
	const auto &tris = mesh.getTriangles();

	const glm::vec3 &A = verts[u].Position;
	const glm::vec3 &B = verts[v].Position;

	float length = glm::distance(A, B);

	float curve = 0.0f;

	// collect triangles sharing edge (u, v)
	std::vector<TriangleID> shared;

	for (TriangleID tid : verts[u].triangles)
	{
		const Triangle &t = tris[tid];
		if (t.contains(v))
			shared.push_back(tid);
	}

	// curvature term
	for (TriangleID tid : verts[u].triangles)
	{
		const Triangle &t = tris[tid];
		float minCurv = 1.0f;

		for (TriangleID sid : shared)
		{
			float dot = glm::dot(t.normal, tris[sid].normal);
			minCurv = std::min(minCurv, (1.0f - dot) * 0.5f);
		}

		curve = std::max(curve, minCurv);
	}

	return length * curve;
}

#endif
