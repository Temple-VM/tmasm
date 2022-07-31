#ifndef PLATFORM_H__HEADER_GUARD__
#define PLATFORM_H__HEADER_GUARD__

/* Windows */
#if defined(WIN32) or defined(_WIN32) or defined(__WIN32__) or defined(__NT__)
#	define PLATFORM_WINDOWS

/* Apple */
#elif defined(__APPLE__)
#	define PLATFORM_APPLE

/* Linux */
#elif defined(__linux__) or defined(__gnu_linux__) or defined(linux)
#	define PLATFORM_LINUX

/* Unix */
#elif defined(__unix__) or defined(unix)
#	define PLATFORM_UNIX

/* Unknown */
#else
#	define PLATFORM_UNKNOWN
#endif

#ifdef PLATFORM_UNKNOWN
#	error "Unknown platform"
#ifdef PLATFORM_WINDOWS
#	error "Windows is not supported for now"
#elif defined(PLATFORM_APPLE)
#	define PLATFORM_LINUX
#elif defined(PLATFORM_UNIX)
#	define PLATFORM_LINUX
#endif

#endif
