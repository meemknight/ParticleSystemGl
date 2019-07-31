//////////////////////////////////////////////////
//ParticleEditor 1.0
//Luta Vlad - 2019
//https://github.com/meemknight/ParticleSystemGl
//https://github.com/meemknight/OpenGLEngine
//////////////////////////////////////////////////
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
#include "save.h"

int MAIN
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
	bool xyEqual = true;
	bool rotate = true;
	bool smallNumber = false;

	ShaderProgram particleShader({ "particleVert.vert" }, { "particleFrag.frag" });
	
	int count = 40'000;
	ParticleSystem particles(count, 8, particleShader, { 1,0,0 }, { 1,1,0 }, { -1, -5, -1 }, { 1, 5, 1 }, 2, 2.2);
	particles.camera = &camera;
	//particles.light = &light;
	particles.affectedByLight = true;
	particles.fadeColor = { 0,0,1 };
	particles.fadeWeight = -1;
	particles.scale = 0.2f;
	particles.gravity = { 0,0,0 };
	particles.kd = 0.2f;
	bool shouldReset = false;
	char fileName[40] = {};

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
			ImGui::NewLine();

			ImGui::Checkbox("affected by light", &particles.affectedByLight);
			if(particles.affectedByLight)
			{
				ImGui::SliderFloat("kd", &particles.kd, 0, 1);
			}
			ImGui::NewLine();

			ImGui::SliderFloat("size", &particles.scale, 0, 5);
			ImGui::SliderFloat("particle duration", &particles.cicleDuration, 0.1, 20);
			ImGui::NewLine();

			ImGui::Checkbox("X, Z equal", &xyEqual);

			if(xyEqual)
			{
				float dir = particles.direction2.x;
				ImGui::SliderFloat("direction On x, z", &dir, 0, 5);
				particles.direction1.x = -dir;
				particles.direction1.z = -dir;
				particles.direction2.x = dir;
				particles.direction2.z = dir;
				ImGui::SameLine(); if (ImGui::Button("Reset###r11")) { particles.direction1 = { 0,0,0 }; particles.direction2 = { 0,0,0 };}

			}else
			{
				ImGui::SliderFloat3("direction1", &particles.direction1.x, -5, 5);
				ImGui::SameLine(); if (ImGui::Button("Reset###r1")) { particles.direction1 = { 0,0,0 }; }

				ImGui::SliderFloat3("direction2", &particles.direction2.x, -5, 5);
				ImGui::SameLine(); if (ImGui::Button("Reset###r2")) { particles.direction2 = { 0,0,0 }; }

				
			}

			ImGui::SliderFloat("speed1", &particles.speed1, 0, 10);
			ImGui::SliderFloat("speed2", &particles.speed2, 0, 10);
			ImGui::SliderFloat3("gravity", &particles.gravity.x, -10, 10);
			ImGui::SameLine(); if (ImGui::Button("Reset###r3")) { particles.gravity = { 0,0,0 }; }
			ImGui::NewLine();

			ImGui::Text("You have to reset the particle for this effects to be applied");
			ImGui::SameLine();
			if (ImGui::Button("Reset System")) 
			{
				shouldReset = true;				
			};

			ImGui::ColorEdit3("color1", (float*)&particles.color1); // Edit 3 floats representing a color
			ImGui::ColorEdit3("color2", (float*)&particles.color2); // Edit 3 floats representing a color
			if(ImGui::Checkbox("Small number of particles", &smallNumber) && smallNumber)
			{
				count = 250;
			}

			if(!smallNumber)
			{
				ImGui::SliderInt("Particle Count###pc1", &count, 0, 1'000'000);
			}else
			{
				ImGui::SliderInt("Particle Count###pc2", &count, 0, 250);
			}
			ImGui::NewLine();

			ImGui::Checkbox("preview rotation", &rotate);
			ImGui::NewLine();

			ImGui::InputText("fileName", fileName, sizeof(fileName));
			ImGui::SameLine();
			if(ImGui::Button("save"))
			{
				if (std::string(fileName).find('.') != std::string::npos)
				{
					save(fileName, particles);
				}else
				{
					save((std::string(fileName) + ".part").c_str(), particles);
				}
			}
			ImGui::SameLine();
			if (ImGui::Button("load"))
			{
				particles.cleanup();
				if(std::string(fileName).find('.') != std::string::npos)
				{
					particles.loadParticleSystem((std::string(fileName)).c_str());
				}else
				{
					particles.loadParticleSystem((std::string(fileName) + ".part").c_str());
				}
				count = particles.count;
			}

			ImGui::NewLine();

			float f = ImGui::GetIO().Framerate;
			if(f<20)
			{
				ImGui::TextColored({1,0,0,1}, "Application average %.3f ms/frame (%.1f FPS)", 1000.0f / f, f);
			}else
			if(f<40)
			{
				ImGui::TextColored({ 1,1,0,1 }, "Application average %.3f ms/frame (%.1f FPS)", 1000.0f / f, f);
			}else
			{
				ImGui::TextColored({ 0,1,0,1 }, "Application average %.3f ms/frame (%.1f FPS)", 1000.0f / f, f);
			}

			ImGui::End();
		}

		for(int i=0; i<3; i++)
		{

			if(particles.direction1[i] > particles.direction2[i])
			{
				std::swap(particles.direction1[i], particles.direction2[i]);
			}

		}

		if(particles.speed1 > particles.speed2)
		{
			std::swap(particles.speed1, particles.speed2);
		}
	

		if(shouldReset)
		{
			particles.cleanup();
			particles.count = count;
			particles.buildParticleSystem();
			shouldReset = false;
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
			if(w !=0 && h != 0)
			{
				width = w;
				heigth = h;
			}
			
			glViewport(0, 0, width, heigth);

		}

	}

	ImGui_ImplGlfwGL3_Shutdown();
	ImGui::DestroyContext();
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}