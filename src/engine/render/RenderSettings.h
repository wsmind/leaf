#pragma once

#include <engine/glm/glm.hpp>

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
	EnvironmentSettings environment;
	BloomSettings bloom;
};
