// OpenGL
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// Imgui
#include "Headers/imgui/imgui.h"
#include "Headers/imgui/imgui_impl_glfw.h"
#include "Headers/imgui/imgui_impl_opengl3.h"
#include "Headers/imgui/implot.h"
// Other
#include <array>
#include <thread>
#include <iostream>
#include <filesystem>
#include <unordered_map>
// My headers
#include "Headers/Shaders/Shader.hpp"
#include "Headers/Particle.hpp"
#include "Headers/IO/Input.hpp"
#include "Headers/Camera.hpp"
#include "Headers/Model.hpp"
#include "Headers/Sphere.hpp"

using namespace IO;

namespace fs = std::filesystem;

int main() {
#pragma region init
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#pragma endregion

#pragma region Window and Context
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Stellar Nursery", nullptr, nullptr);
	if (window == nullptr) {
		std::cerr << "Failed to create window" << std::endl;
		return EXIT_FAILURE;
	}
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);

	stbi_set_flip_vertically_on_load(true);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cerr << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	framebuffer_size_callback(window, SCR_WIDTH, SCR_HEIGHT);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CW);
#pragma endregion

#pragma region GUI
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImPlot::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	std::array<float, 100> frames;
	frames.fill(0.0f);
	int frameNum = 0;

	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 130");
#pragma endregion

#pragma region Shader
	std::string currentPath = fs::current_path().string();

	std::string shaderPath = currentPath + "\\src\\Shaders\\";

	Shader basicShader(shaderPath + "Basic.vert", shaderPath + "Basic.frag");
	Shader PBRShader(shaderPath + "PBR.vert", shaderPath + "PBR.frag");

	ComputeShader initParticles(shaderPath + "InitParticles.comp");
#pragma endregion

#pragma region Models
	std::string modelPath = currentPath + "\\src\\Models\\";
#pragma endregion

#pragma region Objects
	Camera camera(window);

	constexpr size_t ParticleCount = 1000;

	GLuint particleSSBO;
	glGenBuffers(1, &particleSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, particleSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, ParticleCount * sizeof(ParticleGPUObject), nullptr, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, particleSSBO);

	initParticles.use();

	srand((unsigned int)time(0));  // seed once
	int r = rand();                 // integer [0, RAND_MAX]

	// normalize to 0..1, then scale to a reasonable float seed
	float seed = (float)r / (float)RAND_MAX * 10000.0f;

	initParticles.setFloat("Seed", seed);
	initParticles.setInt("ParticleCount", ParticleCount);
	initParticles.setFloat("maxMass", 0.1f);
	initParticles.setFloat("spaceRange", 10.0f);
	initParticles.setFloat("maxVel", 0.0f);
	initParticles.setFloat("angularSpeed", 10.0f);

	constexpr GLuint numGroups = (ParticleCount + 255) / 256;
	glDispatchCompute(numGroups, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	bool start = false;
	bool lastSpace = false;
#pragma endregion

#pragma region Light
	struct DirLight {
		glm::vec3 direction;
		glm::vec3 color;
		glm::vec3 ambient;
	};

	DirLight dirlight{};
	dirlight.direction = glm::vec3(-0.2f, -1.0f, -0.3f);
	dirlight.color = glm::vec3(160.0f) / 255.0f;
	dirlight.ambient = glm::vec3(45.0f) / 255.0f;

	float maxHeat = 25.0f;
#pragma endregion

#pragma region Sphere
	SphereMesh sphereMesh;
	generateUnitSphere(sphereMesh, 24, 24);
#pragma endregion

#pragma region Quad
	float quadVertices[] = {
		// positions    // texCoords
		-1.0f,  1.0f,   0.0f, 1.0f,
		-1.0f, -1.0f,   0.0f, 0.0f,
		 1.0f, -1.0f,   1.0f, 0.0f,

		-1.0f,  1.0f,   0.0f, 1.0f,
		 1.0f, -1.0f,   1.0f, 0.0f,
		 1.0f,  1.0f,   1.0f, 1.0f
	};

	GLuint quadVAO, quadVBO;
	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &quadVBO);

	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0); // Position
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

	glEnableVertexAttribArray(1); // TexCoords
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

	glBindVertexArray(0);
#pragma endregion

#pragma region Time Variables
	float myTime = 0.0f;
	float lastTime = 0.0f;
	float dt = 0.0f;
#pragma endregion

#pragma region Main Loop
	while (!glfwWindowShouldClose(window)) {
#pragma region Time
		myTime = static_cast<float>(glfwGetTime());
		dt = myTime - lastTime;
		lastTime = myTime;
#pragma endregion

#pragma region Update

#pragma region Inputs
		glfwPollEvents();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
#pragma endregion

#pragma region Camera
		camera.update(window, dt);
#pragma endregion
		bool space = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;
		if (space && !lastSpace) start = !start;
		lastSpace = space;

		processInput(window);
#pragma endregion

#pragma region Render
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	

		basicShader.use();
		basicShader.setVec3("viewPos", camera.Position);

		basicShader.setMat4("view", camera.viewMatrix);
		basicShader.setMat4("projection", camera.projectionMatrix);

		basicShader.setVec3("dirLight.direction", glm::normalize(dirlight.direction));
		basicShader.setVec3("dirLight.color", dirlight.color);
		basicShader.setVec3("dirLight.ambient", dirlight.ambient);

		basicShader.setFloat("maxHeat", maxHeat);

		glBindVertexArray(sphereMesh.VAO);
		glDrawElementsInstanced(GL_TRIANGLES, sphereMesh.indices.size(), GL_UNSIGNED_INT, 0, ParticleCount);
		glBindVertexArray(0);
#pragma region GUI
		ImGui::ShowMetricsWindow();

		if (ImGui::Begin("Lighting")) {
			ImGui::Text("Directional Light");

			// Direction as draggable floats
			ImGui::DragFloat3("Direction", glm::value_ptr(dirlight.direction), 0.1f, -1.0f, 1.0f);

			// Color as RGB color picker
			ImGui::ColorEdit3("Color", glm::value_ptr(dirlight.color));

			// Ambient as RGB sliders or color picker
			ImGui::ColorEdit3("Ambient", glm::value_ptr(dirlight.ambient));

			ImGui::Separator();

			ImGui::DragFloat("Max Heat", &maxHeat, 0.01f, 0.0f);
		}
		ImGui::End();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
#pragma endregion

		glfwSwapBuffers(window);
#pragma endregion
	}
#pragma endregion

#pragma region Terminate

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImPlot::DestroyContext();
	ImGui::DestroyContext();

	glfwDestroyWindow(window);
	glfwTerminate();

	return EXIT_SUCCESS;
#pragma endregion
}
