#include <stdio.h>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <filesystem>

// GLAD & GLFW
#include <glad/glad.h> // glad must be included before glfw
#include <GLFW/glfw3.h>

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// imgui
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

// #define SDL_MAIN_HANDLED
// #include <SDL.h>

#include "controls/controls.hpp"
#include "mesh/Mesh.h"
#include "mesh/pMesh.h"
#include "shader/shaderLoader.hpp"

using std::cout;

namespace fs = std::filesystem;

/*
	Progressive meshes
	Read:
		http://hhoppe.com/proj/pm/
		https://www.scribd.com/document/3124116/A-Simple-Fast-Effective-Polygon-Reduction-Algorithm

	Neil Clarke
*/

glm::mat4 ViewMatrix;
glm::mat4 ProjectionMatrix;

// Initial position : on +Z
glm::vec3 position = glm::vec3(0, 0, 5);
// Initial horizontal angle : toward -Z
float horizontalAngle = 3.14f;
// Initial vertical angle : none
float verticalAngle = 0.0f;
// Initial Field of View
float initialFoV = 45.0f;

float speed = 3.0f; // 3 units / second
float mouseSpeed = 0.005f;

int main(int argc, char *argv[])
{
	// set_root_path(argv[0]);

	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);			   // 4x antialiasing
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // We want OpenGL 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);		   // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // We don't want the old OpenGL

	// Open a window and create its OpenGL context
	GLFWwindow *window;
	window = glfwCreateWindow(1024, 768, "Progressive Meshes", NULL, NULL);

	if (window == NULL)
	{
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	registerScrollCallback(window);

	// nitialize glad before any gl function call
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// dearimgui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO();
	(void)io;

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");

	// glewExperimental=true; // Needed in core profile

	// Create and compile our GLSL program from the shaders, from openGL tutorial
	GLuint programID = LoadShaders("./data/shaders/ga_constant_color_vert.glsl",
								   "./data/shaders/ga_constant_color_frag.glsl");

	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

	std::vector<std::string> modelFiles;
	for (const auto &entry : fs::directory_iterator("./data/models"))
	{
		if (entry.is_regular_file())
		{
			std::string path = entry.path().string();
			if (path.size() >= 4 && path.substr(path.size() - 4) == ".obj")
				modelFiles.push_back(path);
		}
	}

	// Keep track of the current selection
	static int currentModelIndex = 0;

	// Create meshes
	Mesh mesh = Mesh(modelFiles[currentModelIndex]);
	pMesh progressive = pMesh(mesh);
	int max = /*mesh->NumVerts()*/ mesh.NumVerts();
	int current = max;

	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 2000.0f);

	do
	{

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Compute the MVP matrix from keyboard and mouse input
		// computeMatricesFromInputs(window);

		// updateOrbitControls(window);

		if (!ImGui::GetIO().WantCaptureMouse)
		{
			updateOrbitControls(window);
		}

		glm::mat4 ProjectionMatrix = getProjectionMatrix();
		glm::mat4 ViewMatrix = getViewMatrix();
		glm::mat4 ModelMatrix = glm::mat4(1.0);
		glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

		// decrease the vertex count with spacebar
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS && current > 0)
		{
			current--;
			progressive.Update(current);
		}

		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS && current < max)
		{
			current++;
			progressive.Update(current);
		}

		// draw imgui
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		int targetVerts = progressive.CurrentVerts();

		ImGui::Begin("Progressive Mesh Control");

		if (ImGui::BeginCombo("Model", fs::path(modelFiles[currentModelIndex]).filename().string().c_str()))
		{
			for (int n = 0; n < modelFiles.size(); n++)
			{
				bool isSelected = (currentModelIndex == n);
				if (ImGui::Selectable(fs::path(modelFiles[n]).filename().string().c_str(), isSelected))
				{
					currentModelIndex = n;

					mesh = Mesh(modelFiles[n]);
					progressive = pMesh(mesh);
					max = mesh.NumVerts();
					current = max;
				}

				if (isSelected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}

		static int collapseStep = progressive.HistorySize();
		int historySize = progressive.HistorySize();

		static int sliderValue = 0;				   
		int stepIndex = historySize - sliderValue;
		if (ImGui::SliderInt("LOD", &sliderValue, 0, historySize))
		{
			progressive.UpdateToStep(stepIndex);
		}

		// Display current vertex count
		ImGui::Text("Current vertices: %d / %d", targetVerts, max);

		ImGui::End();

		// Render ImGui
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// Wireframe on
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		progressive.Draw(programID, MVP);
		// Back to normal (optional)
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		// Swap buffers, apparently this is important
		glfwSwapBuffers(window);
		glfwPollEvents();

	} while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
			 !glfwWindowShouldClose(window));

	glfwTerminate();

	// free imgui resources
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	return 0;
}
