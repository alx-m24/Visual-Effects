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
// My headers
#include "Headers/Shaders/Shader.hpp"
#include "Headers/IO/Input.hpp"
#include "Headers/Camera.hpp"
#include "Headers/Model.hpp"

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
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Honmoon - Lines", nullptr, nullptr);
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

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
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
	Shader honmoonShader(shaderPath + "Honmoon.vert", shaderPath + "Honmoon.frag");
#pragma endregion

#pragma region Models
	std::string modelPath = currentPath + "\\src\\Models\\";

	Model Terrain(modelPath + "Terrain\\Source\\c8856f5efe0e4f63898d5d5b4afafc11.fbx.fbx");
#pragma endregion

#pragma region Objects
	Camera camera(window);

	glm::vec3 lightDir = { -0.2f, -1.0f, -0.3f };
	float ambient = 0.05f;
	float diffuse = 0.4f;
	float specular = 0.5f;
#pragma endregion

#pragma region G_Buffer
	unsigned int gBuffer;
	glGenFramebuffers(1, &gBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
	unsigned int gPosition, gNormal, gColor;
	// position color buffer
	glGenTextures(1, &gPosition);
	glBindTexture(GL_TEXTURE_2D, gPosition);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gPosition, 0);
	// normal color buffer
	glGenTextures(1, &gNormal);
	glBindTexture(GL_TEXTURE_2D, gNormal);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);
	// color + specular color buffer
	glGenTextures(1, &gColor);
	glBindTexture(GL_TEXTURE_2D, gColor);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gColor, 0);
	// tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
	unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, attachments);
	// create and attach depth buffer (renderbuffer)
	unsigned int rboDepth;
	glGenRenderbuffers(1, &rboDepth);
	glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
	// finally check if framebuffer is complete
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Framebuffer not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
#pragma endregion

#pragma region Honmoon
	struct HonmoonVertex {
		glm::vec2 texCoords;
	};

	std::vector<HonmoonVertex> Honmoon_vertices;
	std::vector<GLuint> indices;
	int width = SCR_WIDTH;
	int height = SCR_HEIGHT;

	// downscaling factor: 1 = full res, 2 = half res, 4 = quarter res, etc.
	int resolution = 1;

	int meshWidth = (width + resolution - 1) / resolution;  // ceil division
	int meshHeight = (height + resolution - 1) / resolution;

	// Generate vertices
	for (int y = 0; y < height; y += resolution) {
		for (int x = 0; x < width; x += resolution) {
			glm::vec2 texCoord = glm::vec2(
				static_cast<float>(x) / (width - 1),
				static_cast<float>(y) / (height - 1)
			);
			Honmoon_vertices.push_back({ texCoord });
		}
	}

	// Generate indices for triangles
	for (int y = 0; y < meshHeight - 1; y++) {
		for (int x = 0; x < meshWidth - 1; x++) {
			int i0 = y * meshWidth + x;
			int i1 = i0 + 1;
			int i2 = i0 + meshWidth;
			int i3 = i2 + 1;

			// two triangles per quad
			indices.push_back(i0);
			indices.push_back(i2);
			indices.push_back(i1);

			indices.push_back(i1);
			indices.push_back(i2);
			indices.push_back(i3);
		}
	}


	GLuint Honmoon_VAO, Honmoon_VBO, Honmoon_EBO;
	glGenVertexArrays(1, &Honmoon_VAO);
	glGenBuffers(1, &Honmoon_VBO);
	glGenBuffers(1, &Honmoon_EBO);

	glBindVertexArray(Honmoon_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, Honmoon_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(HonmoonVertex) * Honmoon_vertices.size(), Honmoon_vertices.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Honmoon_EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint)* indices.size(), indices.data(), GL_STATIC_DRAW);

	glEnableVertexAttribArray(0); // Position
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(HonmoonVertex), (void*)0);

	glBindVertexArray(0);

	honmoonShader.use();

	honmoonShader.setInt("gPosition", 0);
	honmoonShader.setInt("gNormal", 1);
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

		processInput(window);
#pragma endregion

#pragma region Render

#pragma region Terrain
		glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, -1.5f, 0.0f));
		model = glm::scale(model, glm::vec3(0.025f));

		basicShader.use();

		basicShader.setMat4("view", camera.viewMatrix);
		basicShader.setMat4("projection", camera.projectionMatrix);
		basicShader.setMat4("model", model);

		basicShader.setVec3("viewPos", camera.Position);

		basicShader.setVec3("dirLight.direction", lightDir);
		basicShader.setVec3("dirLight.ambient", glm::vec3(ambient));
		basicShader.setVec3("dirLight.diffuse", glm::vec3(diffuse));
		basicShader.setVec3("dirLight.specular", glm::vec3(specular));
		basicShader.setVec3("dirLight.color", glm::vec3(1.0f));

		Terrain.draw(basicShader);
#pragma endregion

#pragma region Honmoon
		glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glBlitFramebuffer(0, 0, SCR_WIDTH, SCR_HEIGHT,
			0, 0, SCR_WIDTH, SCR_HEIGHT,
			GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		honmoonShader.use();

		model = glm::mat4(1.0f);

		honmoonShader.setMat4("view", camera.viewMatrix);
		honmoonShader.setMat4("projection", camera.projectionMatrix);
		honmoonShader.setMat4("model", model);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gPosition);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, gNormal);

		glBindVertexArray(Honmoon_VAO);
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
#pragma endregion

#pragma region GUI
		ImGui::ShowMetricsWindow();

		ImGui::Begin("Light");

		ImGui::SliderFloat3("Light Direction", &lightDir.x, -1.0f, 1.0f, "%.2f");

		ImGui::SliderFloat("Ambient", &ambient, 0.0f, 1.0f, "%.2f");
		ImGui::SliderFloat("Diffuse", &diffuse, 0.0f, 1.0f, "%.2f");
		ImGui::SliderFloat("Specular", &specular, 0.0f, 1.0f, "%.2f");

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
