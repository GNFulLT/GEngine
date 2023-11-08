#ifndef GENTITIY_H
#define GENTITIY_H

#include <entt/entt.hpp>
#include <string>
#include <concepts>
#include "engine/scene/serializable_component.h"
#include <unordered_map>
#include "engine/GEngine_EXPORT.h"

template<typename T>
concept SerializableComp = std::derived_from<T, SerializableComponent<T>>;

class ENGINE_API GEntity
{
public:
	GEntity();
	GEntity(entt::entity handler,entt::registry* registeryRef);
	GEntity(entt::entity handler, entt::registry* registeryRef,const char* tag);

	template<SerializableComp T,typename... Args>
	T& emplace_component(Args&&... args)
	{
		assert(!has_component<T>());
		T& component = m_registeryRef->emplace<T>(m_entityHandler, std::forward<Args>(args)...);
		m_serializableComponents.emplace(GTypeUtils::add_or_get_type_info<T>()->m_id, (ISerializable*)&component);
		m_serializables.push_back((ISerializable*)&component);
		return component;
	}
	
	const std::vector<ISerializable*>* get_serializable_components();

	template<typename T,typename... Args>
	T& emplace_component(Args&&... args)
	{
		assert(!has_component<T>());
		return m_registeryRef->emplace<T>(m_entityHandler, std::forward<Args>(args)...);
	}

	template<typename T>
	bool has_component()
	{
		assert(is_valid());
		return m_registeryRef->any_of<T>(m_entityHandler);
	}

	template<typename T>
	void remove_component()
	{
		assert(has_component<T>());
		m_registeryRef->remove<T>(m_entityHandler);
	}

	template<typename T>
	T& get_component()
	{
		assert(has_component<T>());
		return m_registeryRef->get<T>(m_entityHandler);
	}

	template<typename T>
	T* get_or_null_component()
	{
		if (!has_component<T>())
		{
			return nullptr;
		}
		return &m_registeryRef->get<T>(m_entityHandler);
	}
	bool is_valid() const noexcept;
private:
	std::string m_tag;
	std::unordered_map<uint64_t, ISerializable*> m_serializableComponents;
	std::vector<ISerializable*> m_serializables;
	entt::entity m_entityHandler;
	entt::registry* m_registeryRef = nullptr;
};

#endif // GENTITY_H
