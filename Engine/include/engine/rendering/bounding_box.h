#ifndef BOUNDING_BOX_H
#define BOUNDING_BOX_H

#include "glm/glm.hpp"


struct BoundingBox
{
	glm::vec3 min_;
	glm::vec3 max_;
	BoundingBox() = default;
	BoundingBox(const glm::vec3& min, const glm::vec3& max) : min_(glm::min(min, max)), max_(glm::max(min, max)) {}
	BoundingBox(const glm::vec3* points, size_t numPoints)
	{
		glm::vec3 vmin(std::numeric_limits<float>::max());
		glm::vec3 vmax(std::numeric_limits<float>::lowest());

		for (size_t i = 0; i != numPoints; i++)
		{
			vmin = glm::min(vmin, points[i]);
			vmax = glm::max(vmax, points[i]);
		}
		min_ = vmin;
		max_ = vmax;
	}
	glm::vec3 getSize() const { return glm::vec3(max_[0] - min_[0], max_[1] - min_[1], max_[2] - min_[2]); }
	glm::vec3 getCenter() const { return 0.5f * glm::vec3(max_[0] + min_[0], max_[1] + min_[1], max_[2] + min_[2]); }
	void transform(const glm::mat4& t)
	{
		glm::vec3 corners[] = {
			glm::vec3(min_.x, min_.y, min_.z),
			glm::vec3(min_.x, max_.y, min_.z),
			glm::vec3(min_.x, min_.y, max_.z),
			glm::vec3(min_.x, max_.y, max_.z),
			glm::vec3(max_.x, min_.y, min_.z),
			glm::vec3(max_.x, max_.y, min_.z),
			glm::vec3(max_.x, min_.y, max_.z),
			glm::vec3(max_.x, max_.y, max_.z),
		};
		for (auto& v : corners)
			v = glm::vec3(t * glm::vec4(v, 1.0f));
		*this = BoundingBox(corners, 8);
	}
	BoundingBox getTransformed(const glm::mat4& t) const
	{
		BoundingBox b = *this;
		b.transform(t);
		return b;
	}
	void combinePoint(const glm::vec3& p)
	{
		min_ = glm::min(min_, p);
		max_ = glm::max(max_, p);
	}
};



#endif // BOUNDING_BOX_H