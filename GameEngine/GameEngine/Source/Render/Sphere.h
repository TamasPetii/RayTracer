#pragma once
#include <GLM/glm.hpp>

enum MaterialType
{
	DIFFUSE,
	REFLECTIVE
};

struct Material
{
	MaterialType type;
	glm::vec3 color;
};

class Sphere
{
public:
	Sphere(const glm::vec3& origin, float radius, const glm::vec3& color, MaterialType type)
	{
		this->material.color = color;
		this->material.type = type;
		this->origin = origin;
		this->radius = radius;
	}

	Material material;
	glm::vec3 origin;
	float radius;
};

