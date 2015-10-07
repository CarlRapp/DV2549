#include "CompressionHandler_zlib.h"



Compression::CompressionHandler_zlib::CompressionHandler_zlib()
{
}


Compression::CompressionHandler_zlib::~CompressionHandler_zlib()
{
}

void* Compression::CompressionHandler_zlib::compress_memoryToMemory(void *source, void *dest, unsigned int nBytes)
{
	// TODO Write the compress_memoryToMemory function.

	void *compressedData = NULL;

	return compressedData;
}

FILE* Compression::CompressionHandler_zlib::compress_memoryToFile(void *source, unsigned int sizeBytes, FILE *dest, unsigned int nBytes)
{
	int zResult = Z_OK;

	unsigned nBytesCompToFile = 0;
	unsigned nBytesCopiedForComp = 0;

	int flush;
	unsigned have;
	z_stream strm;
	unsigned char in[CHUNK];
	unsigned char out[CHUNK];

	/* allocate deflate state */
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	

	zResult = deflateInit(&strm, Z_BEST_COMPRESSION);
	if (zResult == Z_OK)
	{
		/* compress until end of byte buffer */
		do {
			unsigned int actualCHUNK;
			if (nBytesCopiedForComp + CHUNK < sizeBytes)
				actualCHUNK = CHUNK;
			else
				actualCHUNK = sizeBytes - nBytesCopiedForComp; // CHUNK - (nBytesCopiedForComp + CHUNK - sizeBytes);

			memcpy(in, (char*)source + nBytesCopiedForComp, actualCHUNK);
			nBytesCopiedForComp += CHUNK;
			strm.avail_in = actualCHUNK; //fread(in, 1, CHUNK, source);
			//if (ferror(source)) {
				//(void)deflateEnd(&strm);
				//zResult = Z_ERRNO;
				//break;
			//}
			flush = /*feof(source)*/ (actualCHUNK < CHUNK) ? Z_FINISH : Z_NO_FLUSH;
			strm.next_in = in;

			//if (zResult == Z_ERRNO)
			//	break;

			/* run deflate() on input until output buffer not full, finish
			compression if all of source has been read in */
			do {
				//if (nBytes == -1)
					strm.avail_out = CHUNK;
				//else if (nBytesCompToFile + CHUNK > nBytes)
				//	strm.avail_out = CHUNK - ((nBytesCompToFile + CHUNK) - nBytes);

				strm.next_out = out;
				zResult = deflate(&strm, flush);    /* no bad return value */
				assert(zResult != Z_STREAM_ERROR);  /* state not clobbered */
				have = CHUNK - strm.avail_out;
				if (fwrite(out, 1, have, dest) != have || ferror(dest)) {
					(void)deflateEnd(&strm);
					zResult = Z_ERRNO;
					break;
				}

				if (zResult == Z_ERRNO)
					break;

				nBytesCompToFile += have;
			} while (strm.avail_out == 0);
			assert(strm.avail_in == 0);     /* all input will be used */

			if (zResult == Z_ERRNO)
				break;
			/* done when last data in file processed */
		} while (flush != Z_FINISH && (nBytes == -1 || nBytesCompToFile < nBytes));
		assert(zResult == Z_STREAM_END);        /* stream will be complete */

												/* clean up and return */
		(void)deflateEnd(&strm);
		return Z_OK;
	}

	if (zResult != Z_OK)
	{
		zerr(zResult);
		dest = NULL;
	}

	return dest;
}

int Compression::CompressionHandler_zlib::compress_fileToFile(FILE *source, FILE *dest, unsigned int nBytes)
{
	int zResult = Z_OK;

	unsigned int nBytesCompToFile = 0;
	int nBytesWrittenToFile = 0;

	int flush;
	unsigned have;
	z_stream strm;
	unsigned char in[CHUNK];
	unsigned char out[CHUNK];

	/* allocate deflate state */
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;

	zResult = deflateInit(&strm, Z_BEST_COMPRESSION);
	if (zResult == Z_OK)
	{
		/* compress until end of file */
		do {
			strm.avail_in = fread(in, 1, CHUNK, source);
			if (ferror(source)) {
				(void)deflateEnd(&strm);
				zResult = Z_ERRNO;
				break;
			}
			flush = feof(source) ? Z_FINISH : Z_NO_FLUSH;
			strm.next_in = in;

			if (zResult == Z_ERRNO)
				break;

			/* run deflate() on input until output buffer not full, finish
			compression if all of source has been read in */
			do {
				if (nBytes == -1)
					strm.avail_out = CHUNK;
				else if (nBytesCompToFile + CHUNK > nBytes)
					strm.avail_out = CHUNK - ((nBytesCompToFile + CHUNK) - nBytes);

				strm.next_out = out;
				zResult = deflate(&strm, flush);    /* no bad return value */
				assert(zResult != Z_STREAM_ERROR);  /* state not clobbered */
				have = CHUNK - strm.avail_out;
				int bytesWritten = fwrite(out, 1, have, dest);
				if (bytesWritten != have || ferror(dest)) {
					(void)deflateEnd(&strm);
					zResult = Z_ERRNO;
					break;
				}

				if (zResult == Z_ERRNO)
					break;

				nBytesCompToFile += have; // Should it not be the return value of fwrite rather?
				nBytesWrittenToFile += bytesWritten;
			} while (strm.avail_out == 0);
			assert(strm.avail_in == 0);     /* all input will be used */

			if (zResult == Z_ERRNO)
				break;
			/* done when last data in file processed */
		} while (flush != Z_FINISH && (nBytes == -1 || nBytesCompToFile < nBytes));
		assert(zResult == Z_STREAM_END);        /* stream will be complete */

											/* clean up and return */
		(void)deflateEnd(&strm);
		return nBytesWrittenToFile; //return Z_OK;
	}

	if (zResult != Z_OK)
	{
		zerr(zResult);
		dest = NULL;
	}

	
}

void* Compression::CompressionHandler_zlib::compress_fileToMemory(FILE *source, void *dest, unsigned int nBytes)
{
	// TODO Write the compress_fileToMemory function.

	void *compressedData = NULL;

	return compressedData;
}

void* Compression::CompressionHandler_zlib::deCompress_memoryToMemory(void *source, void *dest, unsigned int nBytes)
{
	// TODO Write the deCompress_memoryToMemory function.

	void *compressedData = NULL;

	return compressedData;
}

FILE* Compression::CompressionHandler_zlib::deCompress_memoryToFile(void *source, FILE *dest, unsigned int nBytes)
{
	// TODO Write the deCompress_memoryToFile function.

	FILE *compressedData = NULL;

	return compressedData;
}

FILE* Compression::CompressionHandler_zlib::deCompress_fileToFile(FILE *source, FILE *dest, unsigned int nBytes)
{
	int zResult = Z_OK;

	unsigned nBytesDecompToFile = 0;

	unsigned have;
	z_stream strm;
	unsigned char in[CHUNK];
	unsigned char out[CHUNK];

	/* allocate inflate state */
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	strm.avail_in = 0;
	strm.next_in = Z_NULL;
	zResult = inflateInit(&strm);
	if (zResult == Z_OK)
	{
		/* decompress until deflate stream ends or end of file */
		do {
			strm.avail_in = fread(in, 1, CHUNK, source);
			if (ferror(source)) {
				(void)inflateEnd(&strm);
				zResult = Z_ERRNO;
				break;
			}

			if (zResult == Z_ERRNO)
				break;

			if (strm.avail_in == 0)
				break;
			strm.next_in = in;

			/* run inflate() on input until output buffer not full */
			do {
				if (nBytes == -1)
					strm.avail_out = CHUNK;
				else if (nBytesDecompToFile + CHUNK > nBytes)
					strm.avail_out = CHUNK - ((nBytesDecompToFile + CHUNK) - nBytes);

				strm.next_out = out;
				zResult = inflate(&strm, Z_NO_FLUSH);
				assert(zResult != Z_STREAM_ERROR);  /* state not clobbered */
				switch (zResult) {
				case Z_NEED_DICT:
					zResult = Z_DATA_ERROR;     /* and fall through */
				case Z_DATA_ERROR:
				case Z_MEM_ERROR:
					(void)inflateEnd(&strm);
					break;
				}
				have = CHUNK - strm.avail_out;
				if (fwrite(out, 1, have, dest) != have || ferror(dest)) {
					(void)inflateEnd(&strm);
					zResult = Z_ERRNO;
					break;
				}

				nBytesDecompToFile += have;
			} while (strm.avail_out == 0 && (nBytes == -1 || nBytesDecompToFile < nBytes));

			/* done when inflate() says it's done */
		} while (zResult != Z_STREAM_END);
	}

	/* clean up and return */
	(void)inflateEnd(&strm);
	zResult = (zResult == Z_STREAM_END) ? Z_OK : Z_DATA_ERROR;

	if (zResult != Z_OK)
	{
		zerr(zResult);
		dest = NULL;
	}

	return dest;
}

int Compression::CompressionHandler_zlib::deCompress_fileToMemory(FILE *source, unsigned int sourceOffset, void *dest, unsigned int nBytes)
{
	unsigned int nBytesDecompressed = 0;

	int zResult = Z_OK;

	int ret;
	unsigned have;
	z_stream strm;
	unsigned char in[CHUNK];
	unsigned char out[CHUNK];

	/* allocate inflate state */
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	strm.avail_in = 0;
	strm.next_in = Z_NULL;
	zResult = inflateInit(&strm);
	if (zResult == Z_OK)
	{
		/* decompress until deflate stream ends or end of file */
		do {
			strm.avail_in = fread(in, 1, CHUNK, source);
			if (ferror(source)) {
				(void)inflateEnd(&strm);
				zResult = Z_ERRNO;
			}

			if (zResult != Z_ERRNO)
			{
				if (strm.avail_in == 0)
					break;
				strm.next_in = in;

				/* run inflate() on input until output buffer not full */
				do {
					strm.avail_out = CHUNK;
					strm.next_out = out;
					zResult = inflate(&strm, Z_NO_FLUSH);
					assert(zResult != Z_STREAM_ERROR);  /* state not clobbered */
					switch (zResult) {
					case Z_NEED_DICT:
						zResult = Z_DATA_ERROR;     /* and fall through */
					case Z_DATA_ERROR:
					case Z_MEM_ERROR:
						(void)inflateEnd(&strm);
						break; // return ret;
					}
					have = CHUNK - strm.avail_out;

					memcpy((char*)dest + nBytesDecompressed, out, have);
					nBytesDecompressed += have;

					/*	if (fwrite(out, 1, have, dest) != have || ferror(dest)) {
							(void)inflateEnd(&strm);
							return Z_ERRNO;
						}*/
				} while (strm.avail_out == 0);

				if (zResult == Z_ERRNO || zResult == Z_NEED_DICT || zResult == Z_DATA_ERROR || zResult == Z_MEM_ERROR)
					break;
			}

			if (zResult == Z_ERRNO || zResult == Z_NEED_DICT || zResult == Z_DATA_ERROR || zResult == Z_MEM_ERROR)
				break;

			/* done when inflate() says it's done */
		} while (zResult != Z_STREAM_END);

		/* clean up and return */
		(void)inflateEnd(&strm);
		zResult = zResult == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
		if (zResult != Z_OK)
		{
			int test = 5;

			Compression::CompressionHandler_zlib::zerr(zResult);
		}

		return nBytesDecompressed;
	}
}

//int Compression::CompressionHandler_zlib::deCompress_fileToMemory(FILE *source, void *dest, unsigned int nBytes)
//{
//	int zResult = Z_OK;
//
//	unsigned nBytesLoaded = 0;
//	unsigned nBytesDecompToMem = 0;
//
//	unsigned have;
//	z_stream strm;
//	unsigned char in[CHUNK];
//	unsigned char out[CHUNK];
//
//	/* allocate inflate state */
//	strm.zalloc = Z_NULL;
//	strm.zfree = Z_NULL;
//	strm.opaque = Z_NULL;
//	strm.avail_in = 0;
//	strm.next_in = Z_NULL;
//	zResult = inflateInit(&strm);
//	if (zResult == Z_OK)
//	{
//		/* decompress until deflate stream ends or end of file */
//		do {
//			strm.avail_in = fread(in, 1, CHUNK, source);
//			if (ferror(source)) {
//				(void)inflateEnd(&strm);
//				zResult = Z_ERRNO;
//				break;
//			}
//
//			if (zResult == Z_ERRNO)
//				break;
//
//			if (strm.avail_in == 0)
//				break;
//			strm.next_in = in;
//
//			/* run inflate() on input until output buffer not full */
//			do {
//				//if (nBytes == -1)
//					strm.avail_out = CHUNK;
//				//else if (nBytesDecompToMem + CHUNK > nBytes)
//				//	strm.avail_out = CHUNK - ((nBytesDecompToMem + CHUNK) - nBytes);
//
//				strm.next_out = out; // (Bytef*)dest + nBytesDecompToMem;
//				zResult = inflate(&strm, Z_NO_FLUSH);
//				assert(zResult != Z_STREAM_ERROR);  /* state not clobbered */
//				switch (zResult) {
//				case Z_NEED_DICT:
//					zResult = Z_DATA_ERROR;     /* and fall through */
//				case Z_DATA_ERROR:
//				case Z_MEM_ERROR:
//					(void)inflateEnd(&strm);
//					break;
//				}
//				have = CHUNK - strm.avail_out;
//				if (  fwrite(out, 1, have, dest) != have || ferror(dest)) {
//					(void)inflateEnd(&strm);
//					zResult = Z_ERRNO;
//					break;
//				}
//
//				nBytesDecompToMem += have;
//			} while (strm.avail_out == 0);
//
//			if (zResult == Z_STREAM_END)
//			{
//				int test = 3;
//			}
//
//			/* done when inflate() says it's done */
//		} while (zResult != Z_STREAM_END && (nBytes == -1 || nBytesDecompToMem < nBytes));
//	}
//
//	/* clean up and return */
//	(void)inflateEnd(&strm);
//	zResult = (zResult == Z_STREAM_END) ? Z_OK : Z_DATA_ERROR;
//
//	if (zResult != Z_OK)
//	{
//		zerr(zResult);
//		dest = NULL;
//	}
//
//	return nBytesDecompToMem;
//}

//
//int Compression::CompressionHandler_zlib::deCompress_fileToMemory(FILE *source, void *dest, unsigned int nBytes)
//{
//	int zResult = Z_OK;
//
//	unsigned nBytesLoaded = 0;
//	unsigned nBytesDecompToMem = 0;
//
//	unsigned have;
//	z_stream strm;
//	unsigned char in[CHUNK];
//	unsigned char out[CHUNK];
//
//	/* allocate inflate state */
//	strm.zalloc = Z_NULL;
//	strm.zfree = Z_NULL;
//	strm.opaque = Z_NULL;
//	strm.avail_in = 0;
//	strm.next_in = Z_NULL;
//	zResult = inflateInit(&strm);
//	if (zResult == Z_OK)
//	{
//		/* decompress until deflate stream ends or end of file */
//		do {
//			strm.avail_in = fread(in, 1, CHUNK, source);
//			if (ferror(source)) {
//				(void)inflateEnd(&strm);
//				zResult = Z_ERRNO;
//				break;
//			}
//
//			if (zResult == Z_ERRNO)
//				break;
//
//			if (strm.avail_in == 0)
//				break;
//			strm.next_in = in;
//
//			/* run inflate() on input until output buffer not full */
//			do {
//				//if (nBytes == -1)
//				strm.avail_out = CHUNK;
//				//else if (nBytesDecompToMem + CHUNK > nBytes)
//				//	strm.avail_out = CHUNK - ((nBytesDecompToMem + CHUNK) - nBytes);
//
//				strm.next_out = out; // (Bytef*)dest + nBytesDecompToMem;
//				zResult = inflate(&strm, Z_NO_FLUSH);
//				assert(zResult != Z_STREAM_ERROR);  /* state not clobbered */
//				switch (zResult) {
//				case Z_NEED_DICT:
//					zResult = Z_DATA_ERROR;     /* and fall through */
//				case Z_DATA_ERROR:
//				case Z_MEM_ERROR:
//					(void)inflateEnd(&strm);
//					break;
//				}
//				have = CHUNK - strm.avail_out;
//
//				nBytesDecompToMem += have;
//			} while (strm.avail_out == 0);
//
//			if (zResult == Z_STREAM_END)
//			{
//				int test = 3;
//			}
//
//			/* done when inflate() says it's done */
//		} while (zResult != Z_STREAM_END && (nBytes == -1 || nBytesDecompToMem < nBytes));
//	}
//
//	/* clean up and return */
//	(void)inflateEnd(&strm);
//	zResult = (zResult == Z_STREAM_END) ? Z_OK : Z_DATA_ERROR;
//
//	if (zResult != Z_OK)
//	{
//		zerr(zResult);
//		dest = NULL;
//	}
//
//	return nBytesDecompToMem;
//}


void Compression::CompressionHandler_zlib::zerr(int ret)
{
	fputs("zpipe: ", stderr);
	switch (ret) {
	case Z_ERRNO:
		if (ferror(stdin))
			fputs("error reading stdin\n", stderr);
		if (ferror(stdout))
			fputs("error writing stdout\n", stderr);
		break;
	case Z_STREAM_ERROR:
		fputs("invalid compression level\n", stderr);
		break;
	case Z_DATA_ERROR:
		fputs("invalid or incomplete deflate data\n", stderr);
		break;
	case Z_MEM_ERROR:
		fputs("out of memory\n", stderr);
		break;
	case Z_VERSION_ERROR:
		fputs("zlib version mismatch!\n", stderr);
	}
}