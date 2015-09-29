#ifndef ICOMPRESSIONHANDLER_H
#define ICOMPRESSIONHANDLER_H

enum CompressionSetting {NO_COMPRESSION = 0, FASTEST_COMPRESSION, MEDIUM_COMPRESSION, MAXIMUM_COMPRESSION};

#include <stdio.h>

namespace Compression
{
	class ICompressionHandler
	{
	public:
		virtual ~ICompressionHandler() {};

		// Compression methods:

		/*	Compresses in-memory data and stores the compressed data in-memory.
		Returns a pointer to the compressed data if the compression of it succeeded.
		Returns a NULL pointer if the compression of the data failed. */
		virtual void* compress_memoryToMemory(void *source, void *dest, unsigned int nBytes = -1) = 0;

		/*	Compresses in-memory data and store the compressed data in a file.
		Returns a pointer to the compressed data if the compression of it succeeded.
		Returns a NULL pointer if the compression of the data failed. */
		virtual FILE* compress_memoryToFile(void *source, unsigned int sizeBytes, FILE *dest, unsigned int nBytes = -1) = 0;

		/*	Loads data from a file, compresses the data, and stores the compressed data in a file.
		Returns a pointer to the compressed data if the compression of it succeeded.
		Returns a NULL pointer if the compression of the data failed. */
		virtual FILE* compress_fileToFile(FILE *source, FILE *dest, unsigned int nBytes = -1) = 0;

		/*	Loads data from a file, compresses the data, and stores the compressed data in the memory.
		Returns a pointer to the compressed data if the compression of it succeeded.
		Returns a NULL pointer if the compression of the data failed. */
		virtual void* compress_fileToMemory(FILE *source, void *dest, unsigned int nBytes = -1) = 0;

		// Decompression methods:

		/*	Decompresses in-memory data and stores the decompressed data in-memory.
		Returns a pointer to the compressed data if the compression of it succeeded.
		Returns a NULL pointer if the compression of the data failed. */
		virtual void* deCompress_memoryToMemory(void *source, void *dest, unsigned int nBytes = -1) = 0;

		/*	Decompresses in-memory data and stores the decompressed data in a file.
		Returns a pointer to the compressed data if the compression of it succeeded.
		Returns a NULL pointer if the compression of the data failed. */
		virtual FILE* deCompress_memoryToFile(void *source, FILE *dest, unsigned int nBytes = -1) = 0;

		/*	Loads data from a file, decompresses the data, and stores the decompressed data in a file.
		Returns a pointer to the compressed data if the compression of it succeeded.
		Returns a NULL pointer if the compression of the data failed. */
		virtual FILE* deCompress_fileToFile(FILE *source, FILE *dest, unsigned int nBytes = -1) = 0;

		/*	Loads data from a file, decompresses the data, and stores the decompressed data in the memory.
		Returns a pointer to the compressed data if the compression of it succeeded.
		Returns a NULL pointer if the compression of the data failed. */
		virtual void* deCompress_fileToMemory(FILE *source, void *dest, unsigned int nBytes = -1) = 0;
	};
}
#endif
