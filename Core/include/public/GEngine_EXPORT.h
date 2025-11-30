
#ifndef CORE_API_H
#define CORE_API_H

#ifdef CORE_AS_STATIC
#  define CORE_API
#  define GENGINE_CORE_NO_EXPORT
#else
#  ifndef CORE_API
#    ifdef GEngine_Core_EXPORTS
        /* We are building this library */
#      define CORE_API __declspec(dllexport)
#    else
        /* We are using this library */
#      define CORE_API __declspec(dllimport)
#    endif
#  endif

#  ifndef GENGINE_CORE_NO_EXPORT
#    define GENGINE_CORE_NO_EXPORT 
#  endif
#endif

#ifndef GENGINE_CORE_DEPRECATED
#  define GENGINE_CORE_DEPRECATED __declspec(deprecated)
#endif

#ifndef GENGINE_CORE_DEPRECATED_EXPORT
#  define GENGINE_CORE_DEPRECATED_EXPORT CORE_API GENGINE_CORE_DEPRECATED
#endif

#ifndef GENGINE_CORE_DEPRECATED_NO_EXPORT
#  define GENGINE_CORE_DEPRECATED_NO_EXPORT GENGINE_CORE_NO_EXPORT GENGINE_CORE_DEPRECATED
#endif

/* NOLINTNEXTLINE(readability-avoid-unconditional-preprocessor-if) */
#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef GENGINE_CORE_NO_DEPRECATED
#    define GENGINE_CORE_NO_DEPRECATED
#  endif
#endif

#endif /* CORE_API_H */
