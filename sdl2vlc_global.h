//code from:
//https://wiki.videolan.org/LibVLC_Users/
//https://wiki.videolan.org/LibVLC_SampleCode_SDL/

#ifndef SDL2VLC_GLOBAL
#define SDL2VLC_GLOBAL

#if defined(SDL2VLC_LIBRARY)
#  define SDL2VLC_SHARED_EXPORT __declspec(dllexport)
#else
#  define SDL2VLC_SHARED_EXPORT __declspec(dllimport)
#endif

#endif // SDL2VLC_GLOBAL

