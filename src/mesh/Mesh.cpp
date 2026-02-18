#include <algorithm> /* min/max */

#include "mesh/Mesh.h"
#include "mesh/objLoader.h"

//================================ EDGE FUNCTIONS ==================================
#pragma region Edge
Edge::Edge(Vertex &s, Vertex &e, Triangle *t)
{
	start = &s;
	end = &e;
	triangles.push_back(*t);
}
#pragma endregion

#pragma region Triangle
glm::vec3 Triangle::getNormal(const Mesh &m) const
{
    if (isDegenerate()) return glm::vec3(0.0f);

	const auto &vertices = m.getVertices();

    const glm::vec3 &p0 = vertices[verts[0]].Position;
    const glm::vec3 &p1 = vertices[verts[1]].Position;
    const glm::vec3 &p2 = vertices[verts[2]].Position;

    // Standard cross product of two edges
    glm::vec3 edge1 = p1 - p0;
    glm::vec3 edge2 = p2 - p0;

    // Use normalize to get a unit vector; length might be zero if colinear
    glm::vec3 n = glm::cross(edge1, edge2);
    float len = glm::length(n);
    
    return (len > 1e-6f) ? n / len : glm::vec3(0.0f);
}

#pragma endregion

#pragma region Mesh

// copy constructor
Mesh::Mesh(const Mesh &m)
	: vertices(m.vertices),
	  triangles(m.triangles),
	  indices(m.indices)
{
	for (auto &v : vertices)
		v.alive = true;

	aliveCount = vertices.size();
	computeInitialQuadrics();
	setupMesh();
}

Mesh::Mesh(const std::string &path)
{
	loadOBJ(path, vertices, triangles, indices);

	for (auto &v : vertices)
		v.alive = true;

	aliveCount = vertices.size();
	computeInitialQuadrics();
	setupMesh();
}

Mesh &Mesh::operator=(const Mesh &m)
{
	if (this == &m)
		return *this;

	this->vertices = m.vertices;
	this->triangles = m.triangles;

	for (auto &tri : this->triangles)
	{
		for (int i = 0; i < tri.verts.size(); i++)
		{
			VertexID id = tri.verts[i];
			tri.verts[i] = id;
		}
	}
	this->indices = m.indices;

	this->setupMesh();

	return *this;
}

void Mesh::initCollapseQueue()
{
	collapseQueue = std::priority_queue<VertexCost>();
	cachedCosts.resize(vertices.size(), std::numeric_limits<float>::max());

	for (VertexID u = 0; u < vertices.size(); ++u)
	{
		if (!vertices[u].alive)
			continue;

		VertexID bestV = -1;
		float minCost = std::numeric_limits<float>::max();

		for (VertexID v : vertices[u].neighbors)
		{
			if (!vertices[v].alive)
				continue;
			float c = Cost(u, v, *this);
			if (c < minCost)
			{
				minCost = c;
				bestV = v;
			}
		}

		if (bestV != -1)
		{
			cachedCosts[u] = minCost;
			vertices[u].destiny = bestV;
			collapseQueue.push({u, bestV, minCost});
		}
	}
}

Mesh::~Mesh()
{
	destroyGL();
}

int Mesh::NumVerts() const
{
	return aliveCount;
}

void Mesh::setupMesh()
{
	glGenVertexArrays(1, &this->VAO);
	glGenBuffers(1, &this->VBO);
	glGenBuffers(1, &this->EBO);

	glBindVertexArray(this->VAO);
	glBindBuffer(GL_ARRAY_BUFFER, this->VBO);

	glBufferData(GL_ARRAY_BUFFER, this->vertices.size() * sizeof(Vertex),
				 &this->vertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->indices.size() * sizeof(GLuint),
				 &this->indices[0], GL_STATIC_DRAW);

	// Vertex Positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
						  (GLvoid *)0);
	// Vertex Normals
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
						  (GLvoid *)offsetof(Vertex, Normal));
	// Vertex Texture Coords
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
						  (GLvoid *)offsetof(Vertex, TexCoords));

	glBindVertexArray(0);

	initCollapseQueue();
}

void Mesh::updateVBO()
{
	std::vector<GLuint> activeIndices;

	for (auto &tri : triangles)
	{
		// Only render if the triangle is not degenerate and all verts are alive
		if (tri.isDegenerate())
			continue;

		bool allAlive = true;
		for (int i = 0; i < 3; ++i)
		{
			if (!vertices[tri.verts[i]].alive)
			{
				allAlive = false;
				break;
			}
		}

		if (allAlive)
		{
			activeIndices.push_back(tri.verts[0]);
			activeIndices.push_back(tri.verts[1]);
			activeIndices.push_back(tri.verts[2]);
		}
	}

	this->indices = activeIndices;

	// Update the existing EBO on the GPU
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint),
				 indices.data(), GL_DYNAMIC_DRAW);
}

void Mesh::destroyGL()
{
	if (VAO)
		glDeleteVertexArrays(1, &VAO);
	if (VBO)
		glDeleteBuffers(1, &VBO);
	if (EBO)
		glDeleteBuffers(1, &EBO);
}

void Mesh::Draw(GLuint programID, const glm::mat4 &MVP)
{
	// glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
	glUseProgram(programID);
	GLint loc = glGetUniformLocation(programID, "u_mvp");
	if (loc != -1)
	{
		glUniformMatrix4fv(loc, 1, GL_FALSE, &MVP[0][0]);
	}

	// update indices in here (probably) ====================================================================
	for (int i = 0; i < 0; i++)
	{
	}

	// Draw mesh
	glBindVertexArray(this->VAO);
	glDrawElements(GL_TRIANGLES, this->indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

bool Mesh::isBoundaryEdge(VertexID u, VertexID v) const
{
	int sharedCount = 0;

	// only need to check triangles attached to one of the vertices
	for (TriangleID tid : vertices[u].triangles)
	{
		const Triangle &t = triangles[tid];

		if (!t.isDegenerate() && t.contains(v))
		{
			sharedCount++;
		}
	}

	/*standard manifold mesh:
		sharedCount == 2: interior edge
		sharedCount == 1: boundary
		sharedCount > 2: very bad for simplification
	*/

	return sharedCount == 1;
}

// =====================================================edge collapse and vertex split

void Mesh::edgeCollapse(VertexID u, VertexID v)
{
	if (!vertices[u].alive || !vertices[v].alive)
		return;

	vertices[u].alive = false;
	aliveCount--;

	for (TriangleID tid : vertices[u].triangles)
	{
		Triangle &t = triangles[tid];
		if (t.contains(v))
			t.verts = {u, u, u};
		else
			t.replace(u, v);
	}

	// update neighbors
	std::vector<VertexID> affected;
	affected.push_back(v);
	for (VertexID n : vertices[u].neighbors)
	{
		auto &nbrs = vertices[n].neighbors;
		nbrs.erase(std::remove(nbrs.begin(), nbrs.end(), u), nbrs.end());

		if (n != v &&
			std::find(vertices[v].neighbors.begin(), vertices[v].neighbors.end(), n) == vertices[v].neighbors.end())
		{
			vertices[v].neighbors.push_back(n);
		}
		affected.push_back(n);
	}

	for (VertexID id : affected)
	{
		updateVertexCost(id);
	}

	vertices[u].neighbors.clear();
}

void Mesh::vertexSplit(VertexID u, VertexID v)
{
	if (u < 0 || v < 0)
		return;

	vertices[u].alive = true;
	aliveCount++;

	for (TriangleID tid : vertices[u].triangles)
	{
		Triangle &t = triangles[tid];

		if (t.isDegenerate())
		{
			t.verts = t.originalVerts;
		}
		else
		{
			t.replace(v, u);
		}
	}
}

void Mesh::updateVertexCost(VertexID u)
{
	if (!vertices[u].alive)
		return;

	float minCost = std::numeric_limits<float>::max();
	VertexID bestV = -1;

	for (VertexID v : vertices[u].neighbors)
	{
		if (!vertices[v].alive)
			continue;

		float c = Cost(u, v, *this);
		if (c < minCost)
		{
			minCost = c;
			bestV = v;
		}
	}

	if (bestV != -1)
	{
		vertices[u].destiny = bestV;
		// We push a new entry. The priority queue will handle the sorting.
		// The cheapestVertex() function already handles skipping "stale" entries
		// by checking if .alive is true.
		collapseQueue.push({u, bestV, minCost});
	}
}

void Mesh::computeInitialQuadrics()
{
	// initialize Q for a vertex based on its neighbor triangles
	for (auto &v : vertices)
	{
		v.Q = glm::mat4(0.0f);
		for (TriangleID tid : v.triangles)
		{
			Triangle &t = triangles[tid];
			// calculate plane equation: ax + by + cz + d = 0
			glm::vec3 n = t.getNormal(*this);
			float d = -glm::dot(n, vertices[t.verts[0]].Position);

			// Fundamental error quadric for this plane
			float a = n.x, b = n.y, c = n.z;
			glm::mat4 K(
				a * a, a * b, a * c, a * d,
				a * b, b * b, b * c, b * d,
				a * c, b * c, c * c, c * d,
				a * d, b * d, c * d, d * d);
			v.Q += K;
		}
	}
}

// get the cheapest edge
VertexID Mesh::cheapestVertex()
{
	while (!collapseQueue.empty())
	{
		VertexCost top = collapseQueue.top();
		collapseQueue.pop();

		if (!vertices[top.u].alive || !vertices[top.v].alive)
			continue;

		// if we find cheaper destiny, skip
		if (vertices[top.u].destiny != top.v)
			continue;

		return top.u;
	}
	return -1;
}
