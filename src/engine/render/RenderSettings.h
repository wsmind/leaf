#pragma once

#include <engine/glm/glm.hpp>

struct CameraSettings
{
	glm::mat4 viewMatrix = glm::mat4();
	glm::mat4 projectionMatrix = glm::mat4();
	float shutterSpeed = 0.01f;
	float focusDistance = 1.0f;
};

struct EnvironmentSettings
{
	glm::vec3 ambientColor = glm::vec3(0.0f);
	float mist = 0.0f;
};

struct BloomSettings
{
	float threshold = 1.0f;
	float intensity = 1.0f;
	bool debug = false;
};

struct RenderSettings
{
	int frameWidth;
	int frameHeight;

	CameraSettings camera;
	EnvironmentSettings environment;
	BloomSettings bloom;
};
