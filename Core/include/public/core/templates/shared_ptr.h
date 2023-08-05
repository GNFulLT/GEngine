#ifndef SHARED_PTR_H
#define SHARED_PTR_H

#include "smart_ptr_internal.h"

template<typename Object,GSHARED_PTR_INTERNAL_MODE InternalMode = (GSHARED_PTR_INTERNAL_MODE)0>
class GSharedPtr
{
	Object* m_rawPtr;
	SharedReferenceCounter<InternalMode> m_sharedReferenceCounter;

	template<typename Object, GSHARED_PTR_INTERNAL_MODE InternalMode> friend class GWeakPtr;
public:
	typedef Object type;
	static constexpr GSHARED_PTR_INTERNAL_MODE Mode = InternalMode;

	//X Althought class have default constructor it doesn't support usage with that.
	_F_INLINE_ GSharedPtr() : m_rawPtr(nullptr)
	{

	}

	_F_INLINE_ explicit GSharedPtr(Object* obj) : m_rawPtr(obj),m_sharedReferenceCounter(create_default_ref_controller<Object,InternalMode>(obj))
	{

	}
	
	//X Copy Constructor

	_F_INLINE_ GSharedPtr(const GSharedPtr<Object, InternalMode>& sharedPtr) : m_rawPtr(sharedPtr.m_rawPtr),m_sharedReferenceCounter(sharedPtr.m_sharedReferenceCounter)
	{

	}



	_F_INLINE_ explicit GSharedPtr(GWeakPtr<Object, InternalMode>& weakPtr) : m_rawPtr(nullptr), m_sharedReferenceCounter(weakPtr.m_sharedReferenceCounter)
	{
		if (m_sharedReferenceCounter.is_valid())
		{
			m_rawPtr = weakPtr.m_rawPtr;
		}
	}
	//X Move Constructor

	_F_INLINE_ GSharedPtr(GSharedPtr<Object, InternalMode>&& sharedPtr) : m_rawPtr(sharedPtr.m_rawPtr), m_sharedReferenceCounter(std::move(sharedPtr.m_sharedReferenceCounter))
	{
		//X Moved so it setted nullptr
		sharedPtr.m_rawPtr = nullptr;
	}

	//X Assignment Operator

	_F_INLINE_ GSharedPtr& operator=(const GSharedPtr& sharedPtr)
	{
		GSharedPtr ptr = sharedPtr;
		std::swap(ptr, *this);
		return *this;
	}


	//X Move Assignment Operator

	_F_INLINE_ GSharedPtr& operator=(GSharedPtr&& sharedPtr)
	{
		if (this != &sharedPtr)
		{
			m_rawPtr = sharedPtr.m_rawPtr;
			sharedPtr.m_rawPtr = nullptr;
			m_sharedReferenceCounter = std::move(sharedPtr.m_sharedReferenceCounter);
		}
		return *this;
	}


	//X Operators
	_F_INLINE_ Object* operator->() {
		return m_rawPtr;
	}

	_F_INLINE_ const Object* operator->() const {
		return m_rawPtr;
	}

	_F_INLINE_ explicit operator bool() const
	{
		return m_rawPtr != nullptr;
	}


	//X METHODS

	_F_INLINE_ Object* get() noexcept
	{
		return m_rawPtr;
	}

	_F_INLINE_ const Object* get() const noexcept
	{
		return m_rawPtr;
	}

	_F_INLINE_ void reset()
	{
		*this = GSharedPtr<Object, InternalMode>();
	}
	
	_F_INLINE_ bool is_valid() const
	{
		return m_rawPtr != nullptr;
	}

	_F_INLINE_ uint32_t get_shared_ref_count() const
	{
		assert(is_valid());
		return m_sharedReferenceCounter.get_shared_count();
	}

	_F_INLINE_ uint32_t get_weak_ref_count() const
	{
		assert(is_valid());
		return m_sharedReferenceCounter.get_weak_count();
	}

	//X TODO : TO WEAK PTR

	_F_INLINE_ GWeakPtr<Object, InternalMode> to_weak_ptr() const
	{
		return GWeakPtr<Object, InternalMode>(*this);
	}
};

template<typename Object, GSHARED_PTR_INTERNAL_MODE InternalMode = (GSHARED_PTR_INTERNAL_MODE)0>
class GWeakPtr
{
	Object* m_rawPtr;
	WeakReferenceCounter<InternalMode> m_sharedReferenceCounter;

	template<typename Object, GSHARED_PTR_INTERNAL_MODE> friend class GSharedPtr;
public:
	typedef Object type;
	static constexpr GSHARED_PTR_INTERNAL_MODE Mode = InternalMode;

	_F_INLINE_ GWeakPtr() : m_rawPtr(nullptr), m_sharedReferenceCounter()
	{

	}

	_F_INLINE_ GWeakPtr(const GSharedPtr<Object,InternalMode>& sharedPtr) : m_rawPtr(sharedPtr.m_rawPtr), m_sharedReferenceCounter(sharedPtr.m_sharedReferenceCounter)
	{

	}

	_F_INLINE_ bool is_valid() const noexcept
	{
		return m_sharedReferenceCounter.is_valid();
	}

	_F_INLINE_ GSharedPtr<Object,InternalMode> as_shared()
	{
		return GSharedPtr<Object, InternalMode>(*this);
	}

};

#endif // SHARED_PTR_H