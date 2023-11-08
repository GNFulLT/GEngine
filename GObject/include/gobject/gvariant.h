#ifndef GVARIANT_H
#define GVARIANT_H



#include "gobject/GEngine_EXPORT.h"
#include "public/typedefs.h"
#include <string>
#include <cstdint>
#include <utility>
#include <gobject/gobject_db.h>
#include "gobject/gtype.h"

struct GTypeInfo;

template <std::size_t SIZE, std::size_t ALIGNMENT = alignof(std::max_align_t)>
struct AlignedStorage
{
	static_assert(SIZE >= sizeof(void*), "storage must be at least the size of a pointer");

	struct Type
	{
		alignas(ALIGNMENT) unsigned char Storage[SIZE];
	};
};

template <typename T, typename = std::void_t<> /* void */>
struct TypeHelpers
{
	template <typename... Args>
	static void* gnew(void* storage, Args&&... args)
	{
		T* instance = new T(std::forward<Args>(args)...);
		new(storage) T* (instance);

		return instance;
	}

	static void* gcopy(void* to, const void* from)
	{
		T* instance = new T(*static_cast<const T*>(from));
		new(to) T* (instance);

		return instance;
	}

	static void* gmove(void* to, void* from)
	{
		T* instance = static_cast<T*>(from);
		new(to) T* (instance);

		return instance;
	}

	static void gdestroy(void* instance)
	{
		delete static_cast<T*>(instance);
	}
};

class GVariantRef;


class GOBJECT_API GVariant
{
	friend class GVariantRef;
public:
	GVariant(void);

	template <typename T, typename U = typename std::remove_cv<std::remove_reference_t<std::decay_t<T>>>::type, typename = typename std::enable_if<!std::is_same_v<U, GVariant>>::type,typename = typename std::enable_if<!std::is_pointer<T>::value>::type>
	GVariant(T&& val);


	GVariant(GVariantRef& ref);

	template<typename T>
	bool can_cast_to();

	explicit operator bool() const { return is_valid(); }


	bool is(const GType&) const;

	template<typename T>
	T* as();


	template<typename T>
	T* unsafe_as();

	const GType& get_type();

	bool is_valid() const _NO_EXCEPT_;

	// Primitive Types
	uint32_t* convert_to_uint32();
	uint64_t* convert_to_uint64();
	int32_t* convert_to_int32();
	int64_t* convert_to_int64();
	std::string* convert_to_string();
	char* convert_to_char();

	void* get_raw();

private:
	void* m_rawData;
	AlignedStorage<sizeof(void*)> m_storage;
	GType m_type;
};


template<typename T>
T* GVariant::unsafe_as()
{
	return (T*)m_rawData;
}

template<typename T, typename U, typename,typename>
inline GVariant::GVariant(T&& val)
{
	m_rawData = TypeHelpers<U>::gnew(&m_storage, std::forward<T>(val));
	m_type = create_type(add_or_get_type_info<T>());
}


class GVariantRef
{
	friend class GVariant;

public:
	GVariantRef() : m_instance(nullptr) {}

	template <typename T, typename U = std::remove_cv_t<T>, typename = std::enable_if_t<!std::is_same_v<U, GVariantRef>>>
	GVariantRef(T& object) : m_instance(&object), m_type(create_type(GObjectDB::get_object_db().get_type_info(typeid(T).hash_code()))) {}

	GVariantRef(GVariant& any) : m_instance(any.m_rawData), m_type(any.m_type) {}
	
	explicit operator bool() const { return m_instance != nullptr && m_type.is_valid(); }

private:
	void* m_instance;
	GType m_type;
};


#endif // GVARIANT_H

