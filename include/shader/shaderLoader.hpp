#ifndef LOADSH_H
#define LOADSH_H


#include <string>
#include <vector>
#include <iostream>
#include <fstream>


#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


GLuint LoadShaders(const char* vertex_file_path, const char* fragment_file_path);


#endif