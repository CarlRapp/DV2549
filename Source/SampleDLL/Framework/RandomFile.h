#ifndef RANDOMFILE_H
#define RANDOMFILE_H

// Add #include <SDL2/SDL.h> here instead
#ifdef SAMPLEDLL_EXPORTS
#    define DECLSPEC __declspec(dllexport)
#else
#    define DECLSPEC __declspec(dllimport)
#endif

namespace SampleDLL
{
	class DECLSPEC RandomFile
	{
	public:
	};
}

#endif