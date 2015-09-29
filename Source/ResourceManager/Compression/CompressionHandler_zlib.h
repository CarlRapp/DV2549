#ifndef COMPRESSIONHANDLER_ZLIB
#define COMPRESSIONHANDLER_ZLIB

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "zlib/zlib.h"

#if defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  define SET_BINARY_MODE(file) _setmode(_fileno(file), O_BINARY) // Changed from setmode to _setmode.
#else
#  define SET_BINARY_MODE(file)
#endif

#include "ResourceManager\Compression\ICompressionHandler.h"

#define CHUNK 16384

namespace Compression
{
	class CompressionHandler_zlib : public ICompressionHandler
	{
	public:
		CompressionHandler_zlib();
		~CompressionHandler_zlib();

		// Compression methods:

		virtual void* compress_memoryToMemory(void *source, void *dest, unsigned int nBytes = -1);
		virtual FILE* compress_memoryToFile(void *source, unsigned int sizeBytes, FILE *dest, unsigned int nBytes = -1);
		virtual FILE* compress_fileToFile(FILE *source, FILE *dest, unsigned int nBytes = -1);
		virtual void* compress_fileToMemory(FILE *source, void *dest, unsigned int nBytes = -1);

		// Decompression methods:

		virtual void* deCompress_memoryToMemory(void *source, void *dest, unsigned int nBytes = -1);
		virtual FILE* deCompress_memoryToFile(void *source, FILE *dest, unsigned int nBytes = -1);
		virtual FILE* deCompress_fileToFile(FILE *source, FILE *dest, unsigned int nBytes = -1);
		virtual void* deCompress_fileToMemory(FILE *source, void *dest, unsigned int nBytes = -1);

		// Misc. methods:

		/* Reports a zlib or i/o error. */
		void zerr(int ret);
	};
}
#endif
