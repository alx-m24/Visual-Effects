#pragma once
// OpenGL
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// GLM
#include <glm/glm.hpp>
// Other
#include "IO/Input.hpp"
#include "Shaders/Shader.hpp"

class Camera {
private:
    enum Movement {
        FORWARD,
        BACKWARD,
        LEFT,
        RIGHT
    };

public:
    glm::vec3 Position = { 0.0f, 0.0f, 0.0f };
    glm::vec3 WorldUp = { 0.0f, 1.0f, 0.0f };
    glm::vec3 front = { 1.0f, 0.0f, 0.0f };
    glm::vec3 right = { 0.0f, 0.0f, 1.0f };
    float Yaw = 87.0f;
    float Pitch = -4.0f;
    float FOV = 75.0f;
    float mouseSens = 0.08f;
    float aspectRatio;
    float far = 80.0f;
    float near = 0.001f;
    const float MovementSpeed = 8.0f; 
    glm::mat4 viewMatrix;
    glm::mat4 projectionMatrix;

public:
    Camera(GLFWwindow* window);
    void update(GLFWwindow* window, float dt);
    void move(Movement movement, float dt);
};