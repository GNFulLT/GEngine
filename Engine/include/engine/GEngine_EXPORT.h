
#ifndef ENGINE_API_H
#define ENGINE_API_H

#ifdef ENGINE_AS_STATIC
#  define ENGINE_API
#  define GENGINE_ENGINE_NO_EXPORT
#else
#  ifndef ENGINE_API
#    ifdef GEngine_Engine_EXPORTS
        /* We are building this library */
#      define ENGINE_API __declspec(dllexport)
#    else
        /* We are using this library */
#      define ENGINE_API __declspec(dllimport)
#    endif
#  endif

#  ifndef GENGINE_ENGINE_NO_EXPORT
#    define GENGINE_ENGINE_NO_EXPORT 
#  endif
#endif

#ifndef GENGINE_ENGINE_DEPRECATED
#  define GENGINE_ENGINE_DEPRECATED __declspec(deprecated)
#endif

#ifndef GENGINE_ENGINE_DEPRECATED_EXPORT
#  define GENGINE_ENGINE_DEPRECATED_EXPORT ENGINE_API GENGINE_ENGINE_DEPRECATED
#endif

#ifndef GENGINE_ENGINE_DEPRECATED_NO_EXPORT
#  define GENGINE_ENGINE_DEPRECATED_NO_EXPORT GENGINE_ENGINE_NO_EXPORT GENGINE_ENGINE_DEPRECATED
#endif

/* NOLINTNEXTLINE(readability-avoid-unconditional-preprocessor-if) */
#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef GENGINE_ENGINE_NO_DEPRECATED
#    define GENGINE_ENGINE_NO_DEPRECATED
#  endif
#endif

#endif /* ENGINE_API_H */
