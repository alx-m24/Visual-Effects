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
#include "Headers/IO/Input.hpp"
#include "Headers/Camera.hpp"
#include "Headers/Model.hpp"

using namespace IO;

namespace fs = std::filesystem;

struct ModelTransforms {
	glm::vec3 position;
	glm::vec3 rotation;
	float scale;
};

struct ModelInfo {
	Model model;
	ModelTransforms transforms;
};

using Models = std::vector<Model>;
using ModelsProperties = std::vector<ModelTransforms>;
using ModelIndex = std::unordered_map<std::string, size_t>;

void AddModel(std::string name, std::string path, const ModelTransforms& transforms, Models& models, ModelIndex& indexes, ModelsProperties& properties) {
	indexes[name] = models.size();

	models.push_back(Model(path));
	properties.push_back(transforms);
}

glm::mat4 applyRotationQuat(glm::mat4 modelMatrix, const glm::vec3& rotation) {
	glm::quat q = glm::quat(glm::radians(rotation)); // expects radians
	modelMatrix *= glm::mat4_cast(q);
	return modelMatrix;
}

void DrawScene(Models& models, ModelIndex& indexes, ModelsProperties& properties, Shader& shader) {
	for (const auto& model_pair : indexes) {
		size_t index = model_pair.second;

		glm::mat4 modelMatrix = glm::mat4(1.0f);
		
		modelMatrix = glm::translate(modelMatrix, properties[index].position);
		modelMatrix = applyRotationQuat(modelMatrix, properties[index].rotation);
		modelMatrix = glm::scale(modelMatrix, glm::vec3(properties[index].scale));

		shader.setMat4("model", modelMatrix);

		models[index].Draw(shader);
	}
}

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
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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

	Shader heightShader(shaderPath + "Height.vert", shaderPath + "Height.frag");
	Shader basicShader(shaderPath + "Basic.vert", shaderPath + "Basic.frag");
	Shader honmoonShader(shaderPath + "Honmoon.vert", shaderPath + "Honmoon.frag");
#pragma endregion

#pragma region Models
	std::string modelPath = currentPath + "\\src\\Models\\";

	Models models;
	ModelsProperties modelProperties;
	ModelIndex indexes;

	//AddModel("Terrain", modelPath + "Terrain\\Source\\c8856f5efe0e4f63898d5d5b4afafc11.fbx.fbx", { glm::vec3(0.0f), glm::vec3(0.0f), 0.01f }, models, indexes, modelProperties);
	AddModel("Village", modelPath + "Village\\source\\Scena_05.fbx", { glm::vec3(0.0f), glm::vec3(0.0f), 0.005f }, models, indexes, modelProperties);
#pragma endregion

#pragma region Objects
	Camera camera(window);

	glm::vec3 lightDir = { -0.2f, -1.0f, -0.3f };
	float ambient = 0.05f;
	float diffuse = 0.4f;
	float specular = 0.5f;
#pragma endregion

#pragma region Honmoon
	struct HonmoonVertex {
		glm::vec2 texCoords;
	};

	std::vector<HonmoonVertex> Honmoon_vertices;
	std::vector<GLuint> indices;
	int width = SCR_WIDTH;
	int height = SCR_HEIGHT;

	int gridSizeX = 100;
	int gridSizeZ = 100;
	float spacing = 1.0f;

	glm::vec3 HonmoonScale = glm::vec3(0.5f);
	glm::vec3 HonmoonSize = glm::vec3(gridSizeX, 0.0f, gridSizeZ) * spacing * HonmoonScale;
	glm::vec3 HonmoonPosition = glm::vec3(-HonmoonSize / 2.0f);
	glm::vec3 HonmoonCenter = HonmoonPosition + HonmoonSize / 2.0f;

	glm::vec2 Honmoon_GlobalOrigin = glm::vec2(HonmoonCenter.x, HonmoonCenter.z);

	for (int z = 0; z <= gridSizeZ; z++) {
		for (int x = 0; x <= gridSizeX; x++) {
			glm::vec2 texCoord = glm::vec2(x, z) / glm::vec2(gridSizeX, gridSizeZ);
			Honmoon_vertices.push_back({ texCoord });
		}
	}

	int meshWidth = gridSizeX + 1;
	int meshHeight = gridSizeZ + 1;

	for (int z = 0; z < gridSizeZ - 1; z++) {  // skip last row
		for (int x = 0; x < gridSizeX - 1; x++) {  // skip last column
			int i0 = z * meshWidth + x;
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

	float hoverHeight = 0.0f;
	float thickness = 1.0f;
	float ringspacing = 1.0f;

	float progress = 0.0f;

	honmoonShader.use();
	honmoonShader.setInt("terrrainHeight", 0);
#pragma endregion

#pragma region Height map
	unsigned int depthMapFBO;
	glGenFramebuffers(1, &depthMapFBO);

	// create depth texture
	unsigned int depthMap;
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_DEPTH_COMPONENT,
		gridSizeX, gridSizeZ,
		0,
		GL_DEPTH_COMPONENT,
		GL_FLOAT,
		NULL
	);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	// attach to framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);

	// no color buffer
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Framebuffer not complete!" << std::endl;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
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

#pragma region Height map
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glViewport(0, 0, gridSizeX, gridSizeZ);
		glClear(GL_DEPTH_BUFFER_BIT);
		
		float near_plane = 0.1f, far_plane = 100.0f;
		float yCamOffset = 50.0;

		// Top-down view: camera above the scene, looking down the -Y axis
		glm::vec3 camPos = HonmoonCenter + glm::vec3(0.0f, yCamOffset, 0.0f);  // move this up if scene is taller
		glm::vec3 target = HonmoonCenter;
		glm::vec3 upVector = glm::vec3(0.0f, 0.0f, -1.0f); // "up" is -Z when looking down Y

		float orthoSizeX = HonmoonSize.x / 2.0f;
		float orthoSizeZ = HonmoonSize.z / 2.0f;

		glm::mat4 view = glm::lookAt(camPos, target, upVector);
		glm::mat4 ortho = glm::ortho(
			-orthoSizeX, orthoSizeX,   // left, right
			-orthoSizeZ, orthoSizeZ,   // bottom, top
			near_plane, far_plane
		);

		heightShader.use();

		heightShader.setMat4("view", view);
		heightShader.setMat4("projection", ortho);

		DrawScene(models, indexes, modelProperties, heightShader);
#pragma endregion

#pragma region Terrain
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		basicShader.use();

		basicShader.setMat4("view", camera.viewMatrix);
		basicShader.setMat4("projection", camera.projectionMatrix);

		basicShader.setVec3("viewPos", camera.Position);

		basicShader.setVec3("dirLight.direction", lightDir);
		basicShader.setVec3("dirLight.ambient", glm::vec3(ambient));
		basicShader.setVec3("dirLight.diffuse", glm::vec3(diffuse));
		basicShader.setVec3("dirLight.specular", glm::vec3(specular));
		basicShader.setVec3("dirLight.color", glm::vec3(1.0f));

		DrawScene(models, indexes, modelProperties, basicShader);
#pragma endregion

#pragma region Honmoon
		honmoonShader.use();

		honmoonShader.setMat4("view", camera.viewMatrix);
		honmoonShader.setMat4("projection", camera.projectionMatrix);

		honmoonShader.setFloat("hoverHeight", hoverHeight);
		honmoonShader.setFloat("yCamOffset", yCamOffset);
		honmoonShader.setVec3("origin", HonmoonPosition);
		honmoonShader.setVec3("size", HonmoonSize);
		honmoonShader.setFloat("far", far_plane);
		honmoonShader.setFloat("near", near_plane);

		honmoonShader.setVec2("patternOrigin", Honmoon_GlobalOrigin);
		honmoonShader.setFloat("spacing", spacing);
		honmoonShader.setFloat("thickness", thickness);
		honmoonShader.setVec4("color1", glm::vec4(35, 218, 215, 255) / 255.0f); // primary color
		honmoonShader.setVec4("color2", glm::vec4(4, 90, 107, 10) / 255.0f); // secondary color

		honmoonShader.setFloat("progress", progress);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, depthMap);

		glBindVertexArray(Honmoon_VAO);
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, nullptr);
#pragma endregion

#pragma region GUI
		ImGui::ShowMetricsWindow();

		ImGui::Begin("Light");

		ImGui::SliderFloat3("Light Direction", &lightDir.x, -1.0f, 1.0f, "%.2f");

		ImGui::SliderFloat("Ambient", &ambient, 0.0f, 1.0f, "%.2f");
		ImGui::SliderFloat("Diffuse", &diffuse, 0.0f, 1.0f, "%.2f");
		ImGui::SliderFloat("Specular", &specular, 0.0f, 1.0f, "%.2f");

		ImGui::Separator();

		ImGui::DragFloat2("Center", &Honmoon_GlobalOrigin[0], 0.01f);

		ImGui::Separator();

		ImGui::SliderFloat("hoverHeight", &hoverHeight, 0.0f, 10.0f, "%.2f");
		ImGui::SliderFloat("thickness", &thickness, 0.0f, 10.0f, "%.2f");
		ImGui::SliderFloat("spacing", &spacing, 0.0f, 10.0f, "%.2f");

		ImGui::End();

		ImGui::Begin("Model Transform");

		for (auto& index : indexes) {
			ImGui::SeparatorText(index.first.c_str());

			ImGui::BeginChild(index.first.c_str());

			ModelTransforms& transforms = modelProperties[index.second];

			ImGui::DragFloat3("Position", &transforms.position.x, 0.01f);
			ImGui::DragFloat3("Rotation", &transforms.rotation.x, 0.1f);
			ImGui::DragFloat("Scale", &transforms.scale, 0.001f);

			ImGui::EndChild();
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
