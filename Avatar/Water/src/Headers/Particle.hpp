#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <random>

struct Particle {
	float radius; // directly related to mass
	glm::vec3 position;
	glm::vec3 previousPosition;
	glm::vec3 acceleration;

	bool operator==(const Particle& p2) const {
		return this == &p2;
	}
};

struct PositionAndRadius {
	glm::vec3 position;
	float radius;
};

struct ParticleGPUObject {
	glm::vec4 position;      // xyz = pos, w = mass
	glm::vec4 velocity;
	glm::vec4 acceleration;
	glm::vec4 jerk;
	glm::vec4 predictedPosition;
	glm::vec4 predictedVelocity;
	glm::vec4 newAcceleration;
	glm::vec4 newJerk;
};