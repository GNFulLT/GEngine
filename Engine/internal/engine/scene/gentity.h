#ifndef GENTITIY_H
#define GENTITIY_H

#include <entt/entt.hpp>
#include <string>

class GEntity
{
public:
	GEntity();
	GEntity(entt::entity handler,entt::registry* registeryRef);
	GEntity(entt::entity handler, entt::registry* registeryRef,const char* tag);

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

	entt::entity m_entityHandler;
	entt::registry* m_registeryRef = nullptr;
};

#endif // GENTITY_H
