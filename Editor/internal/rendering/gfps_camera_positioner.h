#ifndef GVULKAN_FPS_CAMERA_POSITIONER_H
#define GVULKAN_FPS_CAMERA_POSITIONER_H

#include "engine/rendering/icamera_positioner.h"
#include "public/math/gmat4.h"
#include "public/math/gquat.h"
#include <glm/gtx/euler_angles.hpp>
#include <utility>

class IKeyboardManager;
class ImGuiWindowManager;
class IMouseManager;

class GEditorFPSCameraPositioner : public ICameraPositioner
{
public:
	GEditorFPSCameraPositioner(ImGuiWindowManager* windowManager);

	virtual void update(float deltaTime) override;

	virtual const float* get_position() override;

	virtual bool init() override;

	virtual const float* get_view_proj_matrix() const noexcept override;

	virtual const float* get_view_matrix() const noexcept override;

	virtual const float* get_proj_matrix() const noexcept override;

	virtual const CameraData* get_camera_data() noexcept override;
private:

	void setup_up_vector();

	glm::vec2 mousePos_ = glm::vec2(0);
	glm::vec3 cameraPosition_ = glm::vec3(0.0f, 10.0f, 10.0f);
	glm::quat cameraOrientation_ = glm::quat(glm::vec3(0));
	glm::vec3 moveSpeed_ = glm::vec3(0.0f);
	glm::vec3 up_ = glm::vec3(0.0f, 0.0f, 1.0f);
	glm::mat4 m_projection;
	glm::mat4 m_view;

	float m_damping = 0.1f;
	float m_fastCoef = 3.f;
	float m_acceleration = 150.f;
	float m_maxSpeed = 10.0f;
	float mouseSpeed_ = 0.01f;

	mutable glm::mat4 m_viewProj;

	bool m_firstClick = true;
	std::pair<int, int> m_mousePos;
	
	IKeyboardManager* p_keyboardManager;
	IMouseManager* p_mouseManager;
	ImGuiWindowManager* p_windowManager;

	CameraData m_cameraData;
};

#endif // GVULKAN_FPS_CAMERA_CONTROLLER_H