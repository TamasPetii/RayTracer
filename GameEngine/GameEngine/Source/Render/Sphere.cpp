#include "Sphere.h"
#include <iostream>

void Sphere::UpdatePosition(float deltatime)
{
	static float time = 0;
	time += deltatime;
	float angle = time / 5 * 2 * 3.1415;
	origin = glm::vec3(5 * cosf(angle), 5, 5 * sinf(angle));
}