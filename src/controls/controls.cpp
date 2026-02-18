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

	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
	{
		yaw += float(dx) * mouseSpeed;
		pitch -= float(dy) * mouseSpeed;

		pitch = glm::clamp(pitch, -1.5f, 1.5f);
	}

	glm::vec3 cameraPos;
	cameraPos.x = target.x + radius * cos(pitch) * sin(yaw);
	cameraPos.y = target.y + radius * sin(pitch);
	cameraPos.z = target.z + radius * cos(pitch) * cos(yaw);

	glm::vec3 forward = glm::normalize(target - cameraPos);
	glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0, 1, 0)));
	glm::vec3 up = glm::normalize(glm::cross(right, forward));

	int width, height;
	glfwGetFramebufferSize(window, &width, &height);

	float aspect = width / float(height);
	float fov = glm::radians(45.0f);

	// World units per screen pixel at target depth
	float worldPerPixel =
		2.0f * radius * tan(fov * 0.5f) / float(height);

	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS ||
		glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
	{
		glm::vec3 panOffset =
			-right * float(dx) * worldPerPixel +
			up * float(dy) * worldPerPixel;

		target += panOffset;
	}

	cameraPos.x = target.x + radius * cos(pitch) * sin(yaw);
	cameraPos.y = target.y + radius * sin(pitch);
	cameraPos.z = target.z + radius * cos(pitch) * cos(yaw);

	ViewMatrix = glm::lookAt(
		cameraPos,
		target,
		glm::vec3(0, 1, 0));

	ProjectionMatrix = glm::perspective(
		fov,
		aspect,
		0.1f,
		100.0f);
}

void registerScrollCallback(GLFWwindow *window)
{
	glfwSetScrollCallback(window, scrollCallback);
}