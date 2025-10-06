#include "OpenGlDef.h"
#include <GLFW/glfw3.h>
#include <cstdio>

#include "TriangleScene.h"
#include "CubeScene.h"
#include "MeshScene.h"
#include "MultipleObjectScene.h"


void GlMessageCallback(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char* message, const void* userParam)
{
	printf("API: %s\r\n", message);
	if(severity == GL_DEBUG_SEVERITY_HIGH)
	{
		assert(false);
	}
}

int main(int argc, const char** argv)
{
	if(!glfwInit())
	{
		printf("Failed to initialize glfw.\r\n");
		return -1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);

	GLFWwindow* window = glfwCreateWindow(640, 480, "PGJ1303", NULL, NULL);
	if(!window)
	{
		printf("Failed to create window.\r\n");
		return -1;
	}

	glfwMakeContextCurrent(window);

	int version = gladLoadGL(glfwGetProcAddress);
	if(version == 0)
	{
		printf("Failed to initialize OpenGL context\n");
		return -1;
	}

	int flags = 0;
	glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
	if(flags & GL_CONTEXT_FLAG_DEBUG_BIT)
	{
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(GlMessageCallback, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
	}

	//CGenericScene scene;
	CMultipleObjectScene scene;
	//CCubeScene scene;
	double lastTime = glfwGetTime();
	while(!glfwWindowShouldClose(window))
	{
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		scene.SetWindowSize(width, height);

		double currentTime = glfwGetTime();

		scene.Update(currentTime - lastTime);
		scene.Draw();

		glfwSwapBuffers(window);
		glfwPollEvents();

		lastTime = currentTime;
	}

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}
