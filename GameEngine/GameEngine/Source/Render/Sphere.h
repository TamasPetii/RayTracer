#pragma once
#include <GLM/glm.hpp>

enum MaterialType
{
	REFLECTIVE,
	DIFFUSE,
	TEXTURED
};

struct Material
{
	MaterialType type;
	glm::vec3 color;
	int textureId;
};

class Sphere
{
public:
	Sphere(const glm::vec3& origin, float radius, const glm::vec3& color, MaterialType type, int textureId)
	{
		this->material.color = color;
		this->material.type = type;
		this->material.textureId = textureId;
		this->origin = origin;
		this->radius = radius;
	}

	Material material;
	glm::vec3 origin;
	float radius;
};

