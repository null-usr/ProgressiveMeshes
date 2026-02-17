#pragma once

#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

// Call once per frame
void updateOrbitControls(GLFWwindow *window);

void registerScrollCallback(GLFWwindow *window);

// Accessors
glm::mat4 getViewMatrix();
glm::mat4 getProjectionMatrix();
