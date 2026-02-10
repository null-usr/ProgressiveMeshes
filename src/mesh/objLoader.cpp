#include "mesh/objLoader.h"

/*
	for loading the obj files
	getting their edges, faces & verts
*/

bool loadOBJ(const std::string &path,
			 std::vector<Vertex> &vertices,
			 std::vector<Triangle> &triangles,
			 std::vector<GLuint> &indices)
{
	std::ifstream file(path);
	if (!file)
	{
		std::cout << "Failed to open OBJ file\n";
		return false;
	}

	std::vector<glm::vec2> UVs;
	std::vector<glm::vec3> normals;

	std::string line;
	double x, y, z;
	double U, V;

	while (std::getline(file, line))
	{
		// vertex position
		if (line.rfind("v ", 0) == 0)
		{
			std::istringstream v(line.substr(2));
			v >> x >> y >> z;

			Vertex vert;
			vert.Position = glm::vec3(x, y, z);
			vertices.push_back(vert);
		}
		// texture coords
		else if (line.rfind("vt ", 0) == 0)
		{
			std::istringstream v(line.substr(3));
			v >> U >> V;
			UVs.emplace_back(U, V);
		}
		// normals
		else if (line.rfind("vn ", 0) == 0)
		{
			std::istringstream v(line.substr(3));
			v >> x >> y >> z;
			normals.emplace_back(x, y, z);
		}
		// faces
		else if (line.rfind("f ", 0) == 0)
		{
			int a, b, c;
			int t1, t2, t3;
			int n1, n2, n3;

			sscanf(line.c_str(),
				   "f %d/%d/%d %d/%d/%d %d/%d/%d",
				   &a, &t1, &n1,
				   &b, &t2, &n2,
				   &c, &t3, &n3);

			// OBJ is 1-based
			a--; b--; c--;
			t1--; t2--; t3--;
			n1--; n2--; n3--;

			TriangleID tid = triangles.size();
			triangles.emplace_back(a, b, c);

			// per-vertex attributes
			vertices[a].Normal = normals[n1];
			vertices[b].Normal = normals[n2];
			vertices[c].Normal = normals[n3];

			vertices[a].TexCoords = UVs[t1];
			vertices[b].TexCoords = UVs[t2];
			vertices[c].TexCoords = UVs[t3];

			// triangle membership
			vertices[a].triangles.push_back(tid);
			vertices[b].triangles.push_back(tid);
			vertices[c].triangles.push_back(tid);

			// adjacency
			vertices[a].neighbors.push_back(b);
			vertices[a].neighbors.push_back(c);

			vertices[b].neighbors.push_back(a);
			vertices[b].neighbors.push_back(c);

			vertices[c].neighbors.push_back(a);
			vertices[c].neighbors.push_back(b);

			// index buffer
			indices.push_back(a);
			indices.push_back(b);
			indices.push_back(c);
		}
	}

	std::cout << "OBJ load complete\n";
	return true;
}


// save the OBJ
// bool saveOBJ(Mesh *mesh, char *path)
// {
// 	std::ofstream file;
// 	file.open(path);

// 	// write theheader
// 	file << "# Blender3D v249 OBJ File: " << mesh->name << ".blend\n";
// 	file << "# www.blender3d.org\n";

// 	// write the vertices
// 	for (int i = 0; i < mesh->vertices.size(); i++)
// 	{
// 		file << "v " << mesh->vertices[i].Position.x << " " << mesh->vertices[i].Position.y << " " << mesh->vertices[i].Position.z << "\n";
// 	}

// 	// write the normals
// 	for (int i = 0; i < mesh->vertices.size(); i++)
// 	{
// 		file << "vn " << mesh->vertices[i].Normal.x << " " << mesh->vertices[i].Normal.y << " " << mesh->vertices[i].Normal.z << "\n";
// 	}

// 	// write the faces
// 	/*
// 	for(int i=0; i<mesh->triangles.size(); i++)
// 	{
// 		file<<"f "<<mesh->triangles[0]<<"//"
// 	}*/

// 	return true;
// }