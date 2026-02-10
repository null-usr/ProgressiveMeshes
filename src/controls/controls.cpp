#include "controls/controls.hpp"

#include <glm/gtc/matrix_transform.hpp>

// Camera state
static float radius = 5.0f;
static float yaw = 3.14f;
static float pitch = 0.0f;

static float mouseSpeed = 0.005f;
static float zoomSpeed = 1.0f;

static glm::vec3 target = glm::vec3(0.0f);

static glm::mat4 ViewMatrix;
static glm::mat4 ProjectionMatrix;

glm::mat4 getViewMatrix()
{
	return ViewMatrix;
}

glm::mat4 getProjectionMatrix()
{
	return ProjectionMatrix;
}

static void scrollCallback(GLFWwindow *, double, double yoffset)
{
	radius -= float(yoffset) * zoomSpeed;
	if (radius < 1.0f)
		radius = 1.0f;
	if (radius > 50.0f)
		radius = 50.0f;
}

// --------------------
// Public API
// --------------------
void registerScrollCallback(GLFWwindow *window)
{
	glfwSetScrollCallback(window, scrollCallback);
}

void updateOrbitControls(GLFWwindow *window)
{
	static bool firstMouse = true;
	static double lastX = 0.0, lastY = 0.0;

	double x, y;
	glfwGetCursorPos(window, &x, &y);

	if (firstMouse)
	{
		lastX = x;
		lastY = y;
		firstMouse = false;
	}

	double dx = x - lastX;
	double dy = y - lastY;

	lastX = x;
	lastY = y;

	// Rotate when holding left mouse button
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
	{
		yaw += float(dx) * mouseSpeed;
		pitch -= float(dy) * mouseSpeed;

		// Clamp pitch to avoid flipping
		pitch = glm::clamp(pitch, -1.5f, 1.5f);
	}

	// Zoom
	// if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
	// 	radius -= zoomSpeed * 0.1f;
	// if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
	// 	radius += zoomSpeed * 0.1f;

	radius = glm::clamp(radius, 1.0f, 50.0f);

	// Convert spherical → Cartesian
	glm::vec3 cameraPos;
	cameraPos.x = target.x + radius * cos(pitch) * sin(yaw);
	cameraPos.y = target.y + radius * sin(pitch);
	cameraPos.z = target.z + radius * cos(pitch) * cos(yaw);

	ViewMatrix = glm::lookAt(
		cameraPos,
		target,
		glm::vec3(0, 1, 0));

	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	float aspect = width / float(height);

	ProjectionMatrix = glm::perspective(
		glm::radians(45.0f),
		aspect,
		0.1f,
		100.0f);
}
