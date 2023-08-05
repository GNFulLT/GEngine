#ifndef SMART_PTR_INTERNAL_H
#define SMART_PTR_INTERNAL_H

#include <atomic>
#include <type_traits>
#include <cassert>

#include "public/typedefs.h"
#include "public/core/templates/memnewd.h"

enum GSHARED_PTR_INTERNAL_MODE : uint8_t
{
	GSHARED_PTR_INTERNAL_MODE_N_THREAD_SAFE = 0,
	GSHARED_PTR_INTERNAL_MODE_THREAD_SAFE
};

template<GSHARED_PTR_INTERNAL_MODE InternalMode>
class ReferenceCounterBase
{
public:
	typedef std::conditional_t<InternalMode == GSHARED_PTR_INTERNAL_MODE::GSHARED_PTR_INTERNAL_MODE_THREAD_SAFE, std::atomic<uint32_t>, uint32_t> RefCounterType;

private:
	RefCounterType m_sharedRefCount{ 1 };
	RefCounterType m_weakRefCount{ 1 };
protected:
	//X This is for two types that inherits from this class. 1 of them is for custom destructor and another is for normal destructing
	virtual void call_object_destructor() = 0;
public:
	virtual ~ReferenceCounterBase() = default;
	ReferenceCounterBase() = default;
	_F_INLINE_ uint32_t get_shared_ref_count() const _NO_EXCEPT_
	{
		if constexpr (InternalMode == GSHARED_PTR_INTERNAL_MODE_THREAD_SAFE)
		{
			/*
			* Thread safe uses internal locks. No need for sync between threads
			*/
			//X TODO : Should be tried for other order types
			return m_sharedRefCount.load(std::memory_order_relaxed);
		}
		else
		{
			return m_sharedRefCount;
		}
	}

	_F_INLINE_ uint32_t get_weak_ref_count() const _NO_EXCEPT_
	{
		if constexpr (InternalMode == GSHARED_PTR_INTERNAL_MODE_THREAD_SAFE)
		{
			/*
			* Thread safe uses internal locks. No need for sync between threads
			*/
			//X TODO : Should be tried for other order types
			return m_weakRefCount.load(std::memory_order_relaxed);
		}
		else
		{
			return m_weakRefCount;
		}
	}

	_F_INLINE_ bool is_unique() const _NO_EXCEPT_
	{
		if constexpr (InternalMode == GSHARED_PTR_INTERNAL_MODE_THREAD_SAFE)
		{
			/*
			* Thread safe uses internal locks. No need for sync between threads
			*/
			//X TODO : Should be tried for other order types
			return m_sharedRefCount.load(std::memory_order_relaxed) == 1;
		}
		else
		{
			return m_sharedRefCount == 1;
		}
	}

	
	_F_INLINE_ void increment_shared_ref() _NO_EXCEPT_
	{
		if constexpr (InternalMode == GSHARED_PTR_INTERNAL_MODE_THREAD_SAFE)
		{
			//X TODO : MSVC SPECIFIC
			m_sharedRefCount.fetch_add(1, std::memory_order_relaxed);
			return;
		}
		else
		{
			m_sharedRefCount++;
		}

	}

	_F_INLINE_ bool add_shared_reference_if_any_exist()
	{
		if constexpr (InternalMode == GSHARED_PTR_INTERNAL_MODE_THREAD_SAFE)
		{
			//X Doesn't need to restrict any order operation
			int count = m_sharedRefCount.load(std::memory_order_relaxed);
			for (;;)
			{
				if (count == 0)
					return false;


				if (m_sharedRefCount.compare_exchange_weak(count, count + 1, std::memory_order_relaxed));
				{
					return true;
				}
			}
		}
		else
		{
			if (m_sharedRefCount == 0)
			{
				return false;
			}

			m_sharedRefCount++;
			return true;

		}
	}

	_INLINE_ void decrement_shared_ref() _NO_EXCEPT_
	{
		if constexpr (InternalMode == GSHARED_PTR_INTERNAL_MODE_THREAD_SAFE)
		{
			//X TODO : MSVC SPECIFIC
			/*
			* Thread safe uses internal locks. Need sync to decide should we call destruct object
			*/
			uint32_t refCount = m_sharedRefCount.fetch_sub(1, std::memory_order_acq_rel);

			assert(refCount > 0);
			
			if (refCount == 1)
			{
				call_object_destructor();

				decrement_weak_ref();
			}

			return;
		}
		else
		{
			assert(m_sharedRefCount > 0);

			if (--m_sharedRefCount == 0)
			{
				call_object_destructor();

				decrement_weak_ref();
			}
		}
	}

	//X TODO : ADD ONLY INCREMENT WHEN ITS NOT EXPIRED

	_F_INLINE_ void increment_weak_ref() _NO_EXCEPT_
	{
		if constexpr (InternalMode == GSHARED_PTR_INTERNAL_MODE_THREAD_SAFE)
		{
			//X TODO : MSVC SPECIFIC
			m_weakRefCount.fetch_add(1, std::memory_order_relaxed);
		}
		else
		{
			m_weakRefCount++;
		}
	}
	
	_F_INLINE_ void decrement_weak_ref() _NO_EXCEPT_
	{
		if constexpr (InternalMode == GSHARED_PTR_INTERNAL_MODE_THREAD_SAFE)
		{
			//X TODO : MSVC SPECIFIC
			uint32_t refCount = m_weakRefCount.fetch_sub(1, std::memory_order_acq_rel);
			
			assert(refCount > 0);

			if (refCount == 1)
			{
				//X DELETE OP
				delete this;
			}
		}
		else
		{
			assert(m_weakRefCount > 0);

			//X IF WEAK REF COUNT IS 0 DELETE THIS SHARED CONTROLLER
			if (--m_weakRefCount == 0)
			{
				delete this;
			}
		}
	}


	// Non-copyable
	ReferenceCounterBase(const ReferenceCounterBase&) = delete;
	ReferenceCounterBase& operator=(const ReferenceCounterBase&) = delete;
};

	
template<typename Object,typename Deleter, GSHARED_PTR_INTERNAL_MODE InternalMode>
class ReferenceCounterWithDeleter : public ReferenceCounterBase<InternalMode>
{
public:
	explicit ReferenceCounterWithDeleter(Object* obj, Deleter&& deleter) : m_obj(obj),m_deleter(deleter)
	{

	}
protected:
	void call_object_destructor() override
	{
		m_deleter(m_obj);
	}

private:
	Object* m_obj;
	Deleter m_deleter;
};

template<typename Object, GSHARED_PTR_INTERNAL_MODE InternalMode>
inline ReferenceCounterBase<InternalMode>* create_default_ref_controller(Object* obj)
{
	//X TODO : CUSTOM NEW GOES HERE
	return new ReferenceCounterWithDeleter<Object,DefaultDeleteOperation<Object>,InternalMode>(obj,DefaultDeleteOperation<Object>());
}

//X Shared Pointer Reference Controller

template<GSHARED_PTR_INTERNAL_MODE InternalMode>
class SharedReferenceCounter
{
	template<GSHARED_PTR_INTERNAL_MODE InternalMode> friend class WeakReferenceCounter;
public:
	_F_INLINE_ GSHARED_PTR_INTERNAL_MODE get_type() const noexcept
 	{
		return InternalMode;
	}

	_F_INLINE_ SharedReferenceCounter() : m_refCounter(nullptr)
	{

	}

	_F_INLINE_ SharedReferenceCounter(ReferenceCounterBase<InternalMode>* referenceCounter)
	{
		m_refCounter = referenceCounter;
	}

	//X Copy constructor creates new reference

	_F_INLINE_ SharedReferenceCounter(const SharedReferenceCounter& sharedRefCounter) : m_refCounter(sharedRefCounter.m_refCounter)
	{
		if (m_refCounter != nullptr)
		{
			m_refCounter->increment_shared_ref();
		}
	}

	_F_INLINE_ SharedReferenceCounter(const WeakReferenceCounter<InternalMode>& weakRefCounter) : m_refCounter(weakRefCounter.m_refCounter)
	{
		if (m_refCounter != nullptr)
		{
			m_refCounter->increment_shared_ref();
		}
	}

	//X Move constructor doesn't create new reference
	_F_INLINE_ SharedReferenceCounter(SharedReferenceCounter&& sharedRefCounter) : m_refCounter(nullptr) {	}


	//X Weak Counter Move Constructor
	_F_INLINE_ SharedReferenceCounter(WeakReferenceCounter<InternalMode>&& weakRefCounter) : m_refCounter(weakRefCounter.m_refCounter)
	{
		if (m_refCounter != nullptr)
		{
			if (!m_refCounter->add_shared_reference_if_any_exist())
			{
				m_refCounter = nullptr;
			}

			weakRefCounter.m_refCounter->decrement_weak_ref();
			weakRefCounter.m_refCounter = nullptr;
		}
	}
	
	

	//X TODO : Ctor for weak ptr

	_F_INLINE_ ~SharedReferenceCounter()
	{
		if (m_refCounter != nullptr)
		{
			m_refCounter->decrement_shared_ref();
		}
	}

	//X ASSIGNMENT OPERATOR
	_INLINE_ SharedReferenceCounter& operator=(const SharedReferenceCounter& sharedRefCounter)
	{
		if (sharedRefCounter.m_refCounter != m_refCounter)
		{
			if (sharedRefCounter.m_refCounter != nullptr)
			{
				sharedRefCounter.m_refCounter->increment_shared_ref();
			}

			if (m_refCounter != nullptr)
			{
				m_refCounter->decrement_shared_ref();
			}

			m_refCounter = sharedRefCounter.m_refCounter;
		}
		return *this;
	}

	//X MOVE ASSIGNMENT OPERATOR

	_INLINE_ SharedReferenceCounter& operator=(SharedReferenceCounter&& sharedRefCounter)
	{
		//X They should be not same
		if (m_refCounter != sharedRefCounter.m_refCounter)
		{
			//X Swap the counters and release the old one	
			ReferenceCounterBase<InternalMode>* nrefCounter = m_refCounter;
			m_refCounter = sharedRefCounter.m_refCounter;
			sharedRefCounter.m_refCounter = nullptr;

			if (nrefCounter != nullptr)
			{
				nrefCounter->decrement_shared_ref();
			}
		}
		return *this;
	}

	_F_INLINE_ bool is_valid() const noexcept
	{
		return m_refCounter != nullptr;
	}

	_F_INLINE_ uint32_t get_shared_count() const
	{
		return m_refCounter == nullptr ? 0 : m_refCounter->get_shared_ref_count();
	}

	_F_INLINE_ bool is_unique() const
	{
		return m_refCounter != nullptr && m_refCounter->is_unique();
	}

	_F_INLINE_ uint32_t get_weak_count() const
	{
		return m_refCounter == nullptr ? 0 : m_refCounter->get_weak_ref_count();
	}

private:
	ReferenceCounterBase<InternalMode>* m_refCounter;

	template<GSHARED_PTR_INTERNAL_MODE InternalMode> friend class WeakReferenceCounter;

};

template<GSHARED_PTR_INTERNAL_MODE InternalMode>
class WeakReferenceCounter
{
	template<GSHARED_PTR_INTERNAL_MODE InternalMode> friend class SharedReferenceCounter;
public:
	_F_INLINE_ WeakReferenceCounter() : m_refCounter(nullptr)
	{
	}

	_F_INLINE_ WeakReferenceCounter(const WeakReferenceCounter<InternalMode>& weakReferenceCounter) : m_refCounter(weakReferenceCounter.m_refCounter)
	{
		if (m_refCounter != nullptr)
		{
			m_refCounter->increment_weak_ref();
		}
	}

	_F_INLINE_ WeakReferenceCounter(const SharedReferenceCounter<InternalMode>& sharedRefCounter) : m_refCounter(sharedRefCounter.m_refCounter)
	{
		if (m_refCounter != nullptr)
		{
			m_refCounter->increment_weak_ref();
		}
	}

	_F_INLINE_ WeakReferenceCounter& operator=(WeakReferenceCounter&& weakReferenceCounter)
	{
		ReferenceCounterBase<InternalMode>* old_controller = m_refCounter;
		m_refCounter = weakReferenceCounter.m_refCounter;
		weakReferenceCounter.m_refCounter = nullptr;
		if (old_controller != nullptr)
		{
			old_controller->decrement_weak_ref();
		}

		return *this;
	}

	_F_INLINE_ bool is_valid() const noexcept
	{
		return m_refCounter != nullptr && m_refCounter->get_shared_ref_count() > 0;
	}

	_F_INLINE_ ~WeakReferenceCounter()
	{
		if (m_refCounter != nullptr)
		{
			m_refCounter->decrement_weak_ref();
		}
	}
private:
	ReferenceCounterBase<InternalMode>* m_refCounter;
};

#endif // SMART_PTR_INTERNAL_H