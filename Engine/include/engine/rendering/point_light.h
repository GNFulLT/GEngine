#ifndef POINT_LIGHT_H
#define POINT_LIGHT_H

#include "glm/glm.hpp"

struct GPointLight
{
	glm::vec3 position = glm::vec3(0,0,0);
	glm::vec3 color = glm::vec3(1,1,1);

	float intensity = 1.f;
	float radius = 1.f;
	float quadraticFalloff = 0;
	float linearFalloff = 0;
};

#endif // POINT_LIGHT_H