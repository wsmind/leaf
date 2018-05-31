#pragma once

#include <engine/glm/glm.hpp>

class Texture;

struct CameraSettings
{
	glm::mat4 viewMatrix = glm::mat4();
	glm::mat4 projectionMatrix = glm::mat4();
	float shutterSpeed = 0.01f;
	float focusDistance = 1.0f;
	float focalLength = 35.0f;
	float fstop = 16.0f;
};

struct EnvironmentSettings
{
	glm::vec3 ambientColor = glm::vec3(0.0f);
	float mist = 0.0f;
    Texture *environmentMap = nullptr;
};

struct BloomSettings
{
	float threshold = 1.0f;
	float intensity = 1.0f;
    float size = 4.0f;
	bool debug = false;
};

struct PostProcessSettings
{
    float pixellateDivider = 0.0f;
};

struct RenderSettings
{
	int frameWidth;
	int frameHeight;

	CameraSettings camera;
	EnvironmentSettings environment;
	BloomSettings bloom;
    PostProcessSettings postProcess;
};
