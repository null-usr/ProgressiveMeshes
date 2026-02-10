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
		std::cerr << "Failed to open OBJ file: " << path << "\n";
		return false;
	}

	std::vector<glm::vec2> UVs;
	std::vector<glm::vec3> normals;

	std::string line;
	while (std::getline(file, line))
	{
		if (line.rfind("v ", 0) == 0)
		{
			glm::vec3 pos;
			std::istringstream(line.substr(2)) >> pos.x >> pos.y >> pos.z;
			Vertex vert;
			vert.Position = pos;
			vertices.push_back(vert);
		}
		else if (line.rfind("vt ", 0) == 0)
		{
			glm::vec2 uv;
			std::istringstream(line.substr(3)) >> uv.x >> uv.y;
			UVs.push_back(uv);
		}
		else if (line.rfind("vn ", 0) == 0)
		{
			glm::vec3 norm;
			std::istringstream(line.substr(3)) >> norm.x >> norm.y >> norm.z;
			normals.push_back(norm);
		}
		else if (line.rfind("f ", 0) == 0)
		{
			std::istringstream s(line.substr(2));
			std::string token;
			std::vector<int> vIdx, tIdx, nIdx;

			while (s >> token)
			{
				int vi = -1, ti = -1, ni = -1;

				size_t firstSlash = token.find('/');
				size_t lastSlash = token.rfind('/');

				if (firstSlash == std::string::npos)
				{
					// f v1 v2 v3
					vi = std::stoi(token) - 1;
				}
				else if (firstSlash == lastSlash)
				{
					// f v1/vt1
					vi = std::stoi(token.substr(0, firstSlash)) - 1;
					ti = std::stoi(token.substr(firstSlash + 1)) - 1;
				}
				else
				{
					// f v1/vt1/vn1 or f v1//vn1
					vi = std::stoi(token.substr(0, firstSlash)) - 1;
					if (firstSlash + 1 != lastSlash)
						ti = std::stoi(token.substr(firstSlash + 1, lastSlash - firstSlash - 1)) - 1;
					ni = std::stoi(token.substr(lastSlash + 1)) - 1;
				}

				vIdx.push_back(vi);
				tIdx.push_back(ti);
				nIdx.push_back(ni);
			}

			if (vIdx.size() < 3)
				continue; // skip degenerate faces

			// triangulate polygons (assumes convex)
			for (size_t i = 1; i + 1 < vIdx.size(); ++i)
			{
				int a = vIdx[0], b = vIdx[i], c = vIdx[i + 1];
				TriangleID tid = triangles.size();
				triangles.emplace_back(a, b, c);

				// assign normals/UVs if present
				if (nIdx[0] >= 0 && nIdx[0] < normals.size())
					vertices[a].Normal = normals[nIdx[0]];
				if (nIdx[i] >= 0 && nIdx[i] < normals.size())
					vertices[b].Normal = normals[nIdx[i]];
				if (nIdx[i + 1] >= 0 && nIdx[i + 1] < normals.size())
					vertices[c].Normal = normals[nIdx[i + 1]];

				if (tIdx[0] >= 0 && tIdx[0] < UVs.size())
					vertices[a].TexCoords = UVs[tIdx[0]];
				if (tIdx[i] >= 0 && tIdx[i] < UVs.size())
					vertices[b].TexCoords = UVs[tIdx[i]];
				if (tIdx[i + 1] >= 0 && tIdx[i + 1] < UVs.size())
					vertices[c].TexCoords = UVs[tIdx[i + 1]];

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

				indices.push_back(a);
				indices.push_back(b);
				indices.push_back(c);
			}
		}
	}

	std::cout << "OBJ load complete: " << vertices.size() << " vertices, " << triangles.size() << " triangles\n";
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