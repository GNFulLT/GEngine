
#ifndef EDITOR_API_H
#define EDITOR_API_H

#ifdef EDITOR_AS_STATIC
#  define EDITOR_API
#  define GEDITOR_NO_EXPORT
#else
#  ifndef EDITOR_API
#    ifdef GEditor_EXPORTS
        /* We are building this library */
#      define EDITOR_API __declspec(dllexport)
#    else
        /* We are using this library */
#      define EDITOR_API __declspec(dllimport)
#    endif
#  endif

#  ifndef GEDITOR_NO_EXPORT
#    define GEDITOR_NO_EXPORT 
#  endif
#endif

#ifndef GEDITOR_DEPRECATED
#  define GEDITOR_DEPRECATED __declspec(deprecated)
#endif

#ifndef GEDITOR_DEPRECATED_EXPORT
#  define GEDITOR_DEPRECATED_EXPORT EDITOR_API GEDITOR_DEPRECATED
#endif

#ifndef GEDITOR_DEPRECATED_NO_EXPORT
#  define GEDITOR_DEPRECATED_NO_EXPORT GEDITOR_NO_EXPORT GEDITOR_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef GEDITOR_NO_DEPRECATED
#    define GEDITOR_NO_DEPRECATED
#  endif
#endif

#endif /* EDITOR_API_H */
