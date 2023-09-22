#ifndef FLIP_BOOK_H
#define FLIP_BOOK_H

#include <glm/glm.hpp>
#include <cstdint>
#include <vector>

struct AnimationState {
	glm::vec2 position = glm::vec2(0);
	double startTime = 0;
	uint32_t textureIndex = 0;
	uint32_t flipbookOffset = 0;
};
std::vector<AnimationState> animations;

#endif // FLIP_BOOK_H