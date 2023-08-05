#ifndef GSIGNAL_H
#define GSIGNAL_H

#include "public/core/templates/shared_ptr.h"

#include <mutex>
#include <vector>
#include <functional>
//X TODO : When owner slot destructed needed handling for signal

template<typename T>
class GSignal
{
	template<typename T> friend class GSlot;
public:
	typedef T return_input_type;
	typedef std::function<T> func_type;
	typedef GSharedPtr<GSignal<T>> signal_ref;

	~GSignal() = default;

	static GSharedPtr<GSignal<T>> create_signal(func_type callback)
	{
		std::lock_guard<std::mutex> guard(g_globalMutex);
		auto ptr =  GSharedPtr<GSignal<T>>(new GSignal<T>(callback, unique_id++));
		return ptr;
	}

	// Copy Ctor
	GSignal(const GSignal<T>& /*unused*/) {}

	bool is_valid()
	{
		return m_callback;
	}

	template<typename... Args>
	inline void operator()(Args... params)
	{
		if (m_callback)
			m_callback(params...);
	}

private:
	GSignal(func_type callback, long long id) : m_id(id),m_callback(callback)
	{

	}

	long long m_id;
	func_type m_callback;

	inline static long long unique_id = 0;
	inline static std::mutex g_globalMutex;
};
#endif // GSIGNAL_Hc 