#pragma once
#include <glad/glad.h>
#include <cmath>
#include <cstdint>
#include <glm/detail/type_vec3.hpp>
#include <glm/fwd.hpp>
#include <numbers>
#include <vector>

struct SphereVertex {
	glm::vec3 position;
	glm::vec3 normal;
};

struct SphereMesh {
	std::vector<SphereVertex> vertices;
	std::vector<uint32_t> indices;
	unsigned int VAO, VBO, EBO;

};

// sets up the buffers for a unit sphere with position and normal for each vertex
inline void generateUnitSphere(SphereMesh& sphere, int stacks, int slices) {
	sphere.vertices.clear();
	sphere.indices.clear();

	// --- VERTICES --- //
	for (int i = 0; i <= stacks; ++i) {
		float V = (float)i / (float)stacks;
		float theta = V * static_cast<float>(std::numbers::pi); // latitude

		for (int j = 0; j <= slices; ++j) {
			float U = (float)j / (float)slices;
			float phi = U * static_cast<float>(2.0 * std::numbers::pi); // longitude

			SphereVertex vertex;
			vertex.position.x = cosf(phi) * sinf(theta);
			vertex.position.y = cosf(theta);
			vertex.position.z = sinf(phi) * sinf(theta);

			vertex.normal = glm::normalize(vertex.position); // normal is just the unit position

			sphere.vertices.push_back(vertex);
		}
	}

	// --- INDICES --- //
	//  (two triangles per quad)
	for (int i = 0; i < stacks; ++i) {
		for (int j = 0; j < slices; ++j) {
			int first = (i * (slices + 1)) + j;
			int second = first + slices + 1;

			sphere.indices.push_back(first);
			sphere.indices.push_back(second);
			sphere.indices.push_back(first + 1);

			sphere.indices.push_back(second);
			sphere.indices.push_back(second + 1);
			sphere.indices.push_back(first + 1);
		}
	}

	// --- BUFFERS --- //
	if (sphere.VAO) {
		glDeleteVertexArrays(1, &sphere.VAO);
		glDeleteBuffers(1, &sphere.VBO);
		glDeleteBuffers(1, &sphere.EBO);
	}

	glGenVertexArrays(1, &sphere.VAO);
	glGenBuffers(1, &sphere.VBO);
	glGenBuffers(1, &sphere.EBO);

	glBindVertexArray(sphere.VAO);

	glBindBuffer(GL_ARRAY_BUFFER, sphere.VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(SphereVertex) * sphere.vertices.size(), sphere.vertices.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphere.EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * sphere.indices.size(), sphere.indices.data(), GL_STATIC_DRAW);

	glEnableVertexAttribArray(0); // position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(SphereVertex), (void*)offsetof(SphereVertex, position));

	glEnableVertexAttribArray(1); // normal
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(SphereVertex), (void*)offsetof(SphereVertex, normal));


	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}