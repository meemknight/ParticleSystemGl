#define GLFW_EXPOSE_NATIVE_WGL
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <windows.h>
#include "tools.h"
#include "Camera.h"
#include "ParticleSystem.h"
#include "imgui.h"
#include "imgui_impl_glfw_gl3.h"

bool rotate = true;

int main()
{
	if(!glfwInit())
	{
		elog("Coldn't initialize glfw");
	}
	glfwSetErrorCallback([](int r, const char* c) {elog(r, c); });

	float width = 640;
	float heigth = 480;

	GLFWwindow *window = glfwCreateWindow(width, heigth, "Particle Engine", nullptr, nullptr);
	
	if(!window)
	{
		elog("couldn't create windw");
	}

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	glewInit();

	glEnable(GL_DEPTH_TEST);

	ImGui::CreateContext();
	ImGui_ImplGlfwGL3_Init(window, true);
	ImGui::StyleColorsDark();


	float deltaTime = 0, lastTime = GetTickCount();
	auto windowNative = (HWND)glfwGetWin32Window(window);
	Camera camera(windowNative, 85.f, &width, &heigth, 0.1f, 100);
	camera.rSpeed = 0.2;
	float cameraR = 10;
	camera.position = { 0,0, cameraR };
	camera.mSpeed = 10;
	float cameraAnge = 0;

	ShaderProgram particleShader({ "particleVert.vert" }, { "particleFrag.frag" });
	
	ParticleSystem particles(4'000, 8, particleShader, { 1,0,0 }, { 1,1,0 }, { -1, 5,-1 }, { 1,-5 ,1 }, 2, 2.2);
	particles.camera = &camera;
	//particles.light = &light;
	particles.affectedByLight = false;
	particles.fadeColor = { 0,0,1 };
	particles.fadeWeight = -1;
	particles.scale = 0.2f;
	particles.gravity = { 0,0,0 };
	particles.kd = 0.2f;

	while (!glfwWindowShouldClose(window))
	{
		deltaTime = GetTickCount() - lastTime;
		lastTime = GetTickCount();
		deltaTime /= 1000;

		ImGui_ImplGlfwGL3_NewFrame();

		{
			ImGui::Begin("Particle settings");
			ImGui::Text("Particle Editor");                           // Display some text (you can use a format string too)
			ImGui::SliderFloat("fade weight", &particles.fadeWeight, -10.0f, 10.0f);            // Edit 1 float using a slider from 0.0f to 1.0f    
			ImGui::ColorEdit3("fade color", (float*)&particles.fadeColor); // Edit 3 floats representing a color
			ImGui::ColorEdit3("color1", (float*)&particles.fadeColor); // Edit 3 floats representing a color
			ImGui::ColorEdit3("color2", (float*)&particles.fadeColor); // Edit 3 floats representing a color
			ImGui::NewLine();

			ImGui::SliderFloat("size", &particles.scale, 0, 10);
			ImGui::NewLine();

			ImGui::SliderFloat3("direction1", &particles.direction1.x, -5, 5);
			ImGui::SliderFloat3("direction2", &particles.direction2.x, -5, 5);
			ImGui::SliderFloat("speed1", &particles.speed1, 0, 10);
			ImGui::SliderFloat("speed2", &particles.speed2, 0, 10);
			ImGui::SliderFloat3("gravity", &particles.gravity.x, -10, 10);
			ImGui::NewLine();

			ImGui::Checkbox("show rotate", &rotate);
			ImGui::NewLine();

			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::End();
		}



		POINT cursorPos;
		GetCursorPos(&cursorPos);
		RECT windowRect;
		GetWindowRect(windowNative, &windowRect);

		//if (GetAsyncKeyState('W'))
		//{
		//	camera.moveFront( deltaTime);
		//}
		//if (GetAsyncKeyState('S'))
		//{
		//	camera.moveBack(deltaTime);
		//}
		if (GetAsyncKeyState('A'))
		{
			cameraAnge -= deltaTime;
		}
		if (GetAsyncKeyState('D'))
		{
			cameraAnge += deltaTime;
		}

		if (GetAsyncKeyState('W'))
		{
			cameraR -= deltaTime * 10;
		}
		if (GetAsyncKeyState('S'))
		{
			cameraR += deltaTime * 10;
		}

		if (GetAsyncKeyState('R'))
		{
			camera.position.y += deltaTime * 10;
		}
		if (GetAsyncKeyState('F'))
		{
			camera.position.y -= deltaTime * 10;
		}

		if (rotate)
		{
			cameraAnge += deltaTime * 0.5;
		}

		camera.position = { sin(cameraAnge) * cameraR, camera.position.y, cos(cameraAnge) * cameraR };
			

		llog(camera.position.x, camera.position.y, camera.position.z);

		//if (GetAsyncKeyState('R'))
		//{
		//	camera.moveUp(deltaTime);
		//}
		//if (GetAsyncKeyState('F'))
		//{
		//	camera.moveDown(deltaTime);
		//}

		camera.mouseUpdate({ cursorPos.x - windowRect.top, cursorPos.y - windowRect.left });

		particles.draw(deltaTime);


		ImGui::Render();
		ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());

		glfwPollEvents();
		glfwSwapBuffers(window);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		{
			int w;
			int h;
			glfwGetWindowSize(window, &w, &h);
			width = w;
			heigth = h;
			glViewport(0, 0, width, heigth);

		}

	}

	ImGui_ImplGlfwGL3_Shutdown();
	ImGui::DestroyContext();
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}