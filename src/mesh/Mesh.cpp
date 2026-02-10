#include <algorithm> /* min/max */

#include "mesh/Mesh.h"
#include "mesh/objLoader.h"

//================================ EDGE FUNCTIONS ==================================

Edge::Edge(Vertex &s, Vertex &e, Triangle *t)
{
	start = &s;
	end = &e;
	triangles.push_back(*t);
}

//==============================MESH FUNCTIONS======================================

// copy constructor
Mesh::Mesh(const Mesh &m)
	: vertices(m.vertices),
	  triangles(m.triangles),
	  indices(m.indices)
{
	setupMesh();
}

Mesh::Mesh(const std::string &path)
{
	loadOBJ(path, vertices, triangles, indices);
	setupMesh();
}

Mesh::~Mesh()
{
	destroyGL();
}

int Mesh::NumVerts() const
{
	int count = 0;
	for (const auto &v : vertices)
		if (v.alive)
			++count;
	return count;
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
}

// should probably remove this
/*
void Mesh::setVBO()
{
	 std::vector<glm::vec3> indexed_verts;

	for(int i=0; i<triangles.size();i++)
	{
		Triangle *t = triangles[i];

		// grab the vertex positions
		glm::vec3 a = (*t)[0]->getPos();
		glm::vec3 b = (*t)[1]->getPos();
		glm::vec3 c = (*t)[2]->getPos();

	}

	glBufferData(GL_ARRAY_BUFFER, indexed_verts.size() * sizeof(glm::vec3), &indexed_verts[0], GL_STATIC_DRAW);
	// the index data (refers to vertex data)
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indeces.size() * sizeof(unsigned int), &indices[0] , GL_STATIC_DRAW);

}
*/

void Mesh::updateVBO()
{
	std::vector<GLuint> activeIndices;

	for (auto &tri : triangles)
	{
		// Only render if the triangle is not degenerate
		if (!tri.isDegenerate())
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

// =====================================================edge collapse and vertex split

void Mesh::edgeCollapse(VertexID u, VertexID v)
{
	if (u < 0 || v < 0)
		return;
	if (!vertices[u].alive || !vertices[v].alive)
		return;

	vertices[u].alive = false;

	for (TriangleID tid : vertices[u].triangles)
	{
		Triangle &t = triangles[tid];

		if (t.contains(v))
		{
			t.verts = {u, u, u}; // force degenerate
		}
		else
		{
			t.replace(u, v);
			vertices[v].triangles.push_back(tid);
		}
	}
}

void Mesh::vertexSplit(VertexID u, VertexID v)
{
	if (u < 0 || v < 0)
		return;

	vertices[u].alive = true;

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

// get the cheapest edge
VertexID Mesh::cheapestVertex()
{
	float minCost = std::numeric_limits<float>::max();
	VertexID best = -1;
	VertexID target = -1;

	for (VertexID u = 0; u < vertices.size(); ++u)
	{
		if (!vertices[u].alive)
			continue;

		for (VertexID v : vertices[u].neighbors)
		{
			if (!vertices[v].alive)
				continue;

			float cost = Cost(u, v, *this);
			if (cost < minCost)
			{
				minCost = cost;
				best = u;
				target = v;
			}
		}
	}

	if (best != -1)
		vertices[best].destiny = target;

	return best;
}
