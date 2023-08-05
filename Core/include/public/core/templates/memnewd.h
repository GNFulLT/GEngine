#ifndef MEMNEWD_H
#define MEMNEWD_H

#include "public/typedefs.h"

template<typename Object>
struct DefaultDeleteOperation
{
	_F_INLINE_ void operator()(Object* obj)
	{
		delete obj;
	}
};

template<typename Object>
struct DefaultNewOperation
{
	_F_INLINE_ Object* operator()()
	{
		return new Object();
	}
};

template<typename Object,typename... Args>
struct DefaultNewOperationArgs
{
	_F_INLINE_ Object* operator()(Args... args)
	{
		return new Object(args);
	}
};


template<typename Object>
_F_INLINE_ Object* __new_tmpl()
{
	return new Object();
}

template<typename Object,typename... Args>
_F_INLINE_ Object* __new_tmpl(Args... args)
{
	return new Object(args...);
}

template<typename Object,typename Newer>
_F_INLINE_ Object* __new_tmpl_default(Newer&& newer)
{
	//X TODO: Should there be a backtrace logger think about it
	return newer();
}

//X It doesn't work
template<typename Object, typename Newer,typename... Args>
_F_INLINE_ Object* __new_tmpl_default(Newer&& newer,Args... args)
{
	//X TODO: Should there be a backtrace logger think about it
	return newer(args...);
}

template<typename Object>
_F_INLINE_ void __del_tmpl_default(Object* ptr)
{
	delete ptr;
}

#define gdnew(type) __new_tmpl<type>(); 

#define gdnewa(type,...) __new_tmpl<type>(__VA_ARGS__); 

#define gdnewd(type) __new_tmpl_default<type,DefaultNewOperation<type>>();

#define gdnewda(type,...) __new_tmpl_default<type,DefaultNewOperationArgs<type>>(DefaultNewOperationArgs<type>(),__VA_ARGS__);

#define gddel(ptr) __del_tmpl_default(ptr);

#endif // MEMNEWD_H