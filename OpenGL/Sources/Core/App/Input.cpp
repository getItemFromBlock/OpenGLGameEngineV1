#include "Core/App/Input.hpp"

using namespace Core::App;

float mouseScroll = 0;
int LastKey = 0;

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	mouseScroll += (float)yoffset;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS)
		LastKey = key;
}

Core::Maths::Int2D screenDim = Core::Maths::Int2D();
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
	screenDim = Core::Maths::Int2D(width, height);
}

void Inputs::InitInputs(GLFWwindow* window, Core::Maths::Int2D defaultSize)
{
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetKeyCallback(window, key_callback);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	screenDim = defaultSize;
	ScreenSize = defaultSize;
}

int Core::App::Inputs::GetLastKey()
{
	return LastKey;
}

void Core::App::Inputs::ClearLastKey()
{
	LastKey = 0;
}

void Inputs::UpdateInputs(GLFWwindow* window)
{
	ScreenSize = screenDim;
	double newMouseX, newMouseY;
	glfwGetCursorPos(window, &newMouseX, &newMouseY);
	deltaMouse.x = (float)(newMouseX - MousePos.x);
	deltaMouse.y = (float)(newMouseY - MousePos.y);
	MousePos.x = (float)newMouseX;
	MousePos.y = (float)newMouseY;

	leftMouse = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1);
	rightMouse = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2);
	scroll = mouseScroll;
	mouseScroll = 0;

	if (rightMouse && !mouseCaptured)
	{
		mouseCaptured = true;
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}
	else if (!rightMouse && mouseCaptured)
	{
		mouseCaptured = false;
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}

	forward = glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS;
	backward = glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS;
	left = glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS;
	right = glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS;
	up = glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS;
	down = glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS;
	shift = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;
	control = glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS;
}