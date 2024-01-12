#include "internal/engine/manager/gphysics_manager.h"

inline btVector3 glm_to_bullet(const glm::vec3& v) {
    return btVector3(v.x, v.y, v.z);
}

inline btQuaternion glm_to_bullet(const glm::quat& v) {
    return btQuaternion(v.x, v.y, v.z,v.w);
}

GPhysicsManager::GPhysicsManager()
{
}

bool GPhysicsManager::init()
{
    m_btCollisionConfiguration = new btDefaultCollisionConfiguration();
    m_btDispatcher = new btCollisionDispatcher(m_btCollisionConfiguration);
    m_overlappingPairCache = new btDbvtBroadphase();
    m_solver = new btSequentialImpulseConstraintSolver();
    m_dynamicsWorld = new btDiscreteDynamicsWorld(m_btDispatcher, m_overlappingPairCache, m_solver, m_btCollisionConfiguration);

    m_dynamicsWorld->setGravity({0,-9.8f,0});
	return true;
}

void GPhysicsManager::update_simulation(float deltaTime)
{
	m_dynamicsWorld->stepSimulation(deltaTime, 10, 0.01f);
    for (size_t i = 0; i != m_rigidBodies.size(); i++) {
        if (!m_rigidBodies[i]->isActive()) continue;
        btTransform trans;
        m_rigidBodies[i]->getMotionState()->getWorldTransform(trans);
    }

}

void GPhysicsManager::destroy()
{
    m_boxSizesTransforms.clear();
    m_rigidBodies.clear();

    //X TODO : NULL CHECK
    delete m_dynamicsWorld;
    delete m_solver;
    delete m_overlappingPairCache;
    delete m_btDispatcher;
    delete m_btCollisionConfiguration;
    btDefaultCollisionConfiguration* m_btCollisionConfiguration = nullptr;
    btCollisionDispatcher* m_btDispatcher = nullptr;
    btBroadphaseInterface* m_overlappingPairCache = nullptr;

    //X BulletMultiThreaded Can be used here
    btSequentialImpulseConstraintSolver* m_solver = nullptr;
}

uint32_t GPhysicsManager::add_rigidbody(const glm::vec3& halfSize, const glm::vec3& position, const glm::quat& rotation, float mass)
{
    uint32_t index = m_rigidBodies.size();
    assert(index == m_boxSizesTransforms.size());
    m_boxSizesTransforms.push_back(std::make_pair(halfSize, glm::mat4(1.f)));
  
    btCollisionShape* collisionShape = new btBoxShape(glm_to_bullet(halfSize));
    btDefaultMotionState* motionState = new btDefaultMotionState(btTransform(glm_to_bullet(rotation), glm_to_bullet(position)));
    btVector3 localInertia(0, 0, 0);
    collisionShape->calculateLocalInertia(mass, localInertia);
    btRigidBody::btRigidBodyConstructionInfo rigidBodyCI(mass, motionState, collisionShape, localInertia);
    rigidBodyCI.m_friction = 0.1f;
    rigidBodyCI.m_rollingFriction = 0.1f;
 
    m_rigidBodies.emplace_back(std::make_unique<btRigidBody>(rigidBodyCI));
    m_dynamicsWorld->addRigidBody(m_rigidBodies.back().get());
   
    return index;
}
