#ifndef GPHYSICS_MANAGER_H
#define GPHYSICS_MANAGER_H

#include "btBulletDynamicsCommon.h"
#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <utility>

class GPhysicsManager
{
public:
	GPhysicsManager();
	bool init();
	void update_simulation(float deltaTime);
	void destroy();

	uint32_t add_rigidbody(const glm::vec3& halfSize,const glm::vec3& position,const glm::quat& rotation,float mass);
private:
	btDefaultCollisionConfiguration* m_btCollisionConfiguration = nullptr;
	btCollisionDispatcher* m_btDispatcher = nullptr;
	btBroadphaseInterface* m_overlappingPairCache = nullptr;

	//X BulletMultiThreaded Can be used here
	btSequentialImpulseConstraintSolver* m_solver = nullptr;
	btDiscreteDynamicsWorld* m_dynamicsWorld = nullptr;

	std::vector<std::unique_ptr<btRigidBody>>  m_rigidBodies;

	std::vector<std::pair<glm::vec3,glm::mat4>> m_boxSizesTransforms;
};
#endif // GPHYSICS_MANAGER_H