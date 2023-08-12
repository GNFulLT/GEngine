
#ifndef GOBJECT_API_H
#define GOBJECT_API_H

#ifdef GOBJECT_AS_STATIC
#  define GOBJECT_API
#  define GENGINE_GOBJECT_NO_EXPORT
#else
#  ifndef GOBJECT_API
#    ifdef GEngine_GObject_EXPORTS
        /* We are building this library */
#      define GOBJECT_API __declspec(dllexport)
#    else
        /* We are using this library */
#      define GOBJECT_API __declspec(dllimport)
#    endif
#  endif

#  ifndef GENGINE_GOBJECT_NO_EXPORT
#    define GENGINE_GOBJECT_NO_EXPORT 
#  endif
#endif

#ifndef GENGINE_GOBJECT_DEPRECATED
#  define GENGINE_GOBJECT_DEPRECATED __declspec(deprecated)
#endif

#ifndef GENGINE_GOBJECT_DEPRECATED_EXPORT
#  define GENGINE_GOBJECT_DEPRECATED_EXPORT GOBJECT_API GENGINE_GOBJECT_DEPRECATED
#endif

#ifndef GENGINE_GOBJECT_DEPRECATED_NO_EXPORT
#  define GENGINE_GOBJECT_DEPRECATED_NO_EXPORT GENGINE_GOBJECT_NO_EXPORT GENGINE_GOBJECT_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef GENGINE_GOBJECT_NO_DEPRECATED
#    define GENGINE_GOBJECT_NO_DEPRECATED
#  endif
#endif

#endif /* GOBJECT_API_H */
