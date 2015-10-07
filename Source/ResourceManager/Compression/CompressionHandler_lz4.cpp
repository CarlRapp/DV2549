#include "CompressionHandler_lz4.h"

Compression::CompressionHandler_lz4::CompressionHandler_lz4()
{
}

Compression::CompressionHandler_lz4::~CompressionHandler_lz4()
{
}

void * Compression::CompressionHandler_lz4::compress_memoryToMemory(void * source, void * dest, unsigned int nBytes)
{
	return nullptr;
}

FILE * Compression::CompressionHandler_lz4::compress_memoryToFile(void * source, unsigned int sizeBytes, FILE * dest, unsigned int nBytes)
{
	return nullptr;
}

typedef int(*CompressionFunc)
(const char* src, char* dst, int size, int maxOut, int maxOutputSize);

int bridge_LZ4_compress_limitedOutput(const char* src, char* dst, int size, int maxOut, int) {
	return LZ4_compress_limitedOutput(src, dst, size, maxOut);
}

int Compression::CompressionHandler_lz4::compress_fileToFile(FILE * source, FILE * dest, unsigned int nBytes)
{
	const char *sourceData = NULL;
	char *compressedData = NULL;

	fseek(source, 0, SEEK_END);
	int inputSize = ftell(source);
	sourceData = new char[inputSize];

	int outputBufferSizeNeeded_worstCaseScenario = LZ4_compressBound(inputSize);
	compressedData = new char[outputBufferSizeNeeded_worstCaseScenario];

	rewind(source);
	fread((void*)sourceData, 1, inputSize, source);
	fclose(source);
	
	int nBytes_compressedData = LZ4_compressHC(sourceData, compressedData, inputSize);

	fwrite(compressedData, 1, nBytes_compressedData, dest);
	fclose(dest);

	return nBytes_compressedData;

	//using namespace Lz4Mt::Cstdio;

	//Lz4MtStreamDescriptor sd = lz4mtInitStreamDescriptor();
	//Lz4MtContext ctx = lz4mtInitContext();

	//ctx.mode = static_cast<Lz4MtMode>(LZ4MT_MODE_DEFAULT); //static_cast<Lz4MtMode>(opt.mode);
	//ctx.read = read;
	//ctx.readSeek = readSeek;
	//ctx.readEof = readEof;
	//ctx.write = write;
	//ctx.compressBound = LZ4_compressBound;
	//ctx.decompress = LZ4_decompress_safe;
	//ctx.compressionLevel = 3; //opt.compressionMode.getCompressionLevel();
	//ctx.compress =  [&ctx]() -> CompressionFunc {
	//	// NOTE for "-> CompressionFunc" :
	//	//		It's a workaround for g++-4.6's strange warning.

	//	if (ctx.compressionLevel >= 3) {
	//		return LZ4_compressHC2_limitedOutput;
	//	}
	//	else {
	//		return bridge_LZ4_compress_limitedOutput;
	//	}
	//}();

	////const auto e = [&]() -> Lz4MtResult {
	////	if (opt.compressionMode.isCompress()) {
	////		return lz4mtCompress(&ctx, &opt.sd);
	////	}
	////	else if (opt.compressionMode.isDecompress()) {
	////		return lz4mtDecompress(&ctx, &opt.sd);
	////	}
	////	else {
	////		return LZ4MT_RESULT_ERROR;
	////	}

	//Lz4MtResult result = lz4mtCompress(&ctx, &sd);
}

void * Compression::CompressionHandler_lz4::compress_fileToMemory(FILE * source, void * dest, unsigned int nBytes)
{
	return nullptr;
}

void * Compression::CompressionHandler_lz4::deCompress_memoryToMemory(void * source, void * dest, unsigned int nBytes)
{
	return nullptr;
}

FILE * Compression::CompressionHandler_lz4::deCompress_memoryToFile(void * source, FILE * dest, unsigned int nBytes)
{
	return nullptr;
}

FILE * Compression::CompressionHandler_lz4::deCompress_fileToFile(FILE * source, FILE * dest, unsigned int nBytes)
{
	return nullptr;
}

int Compression::CompressionHandler_lz4::deCompress_fileToMemory(FILE * source, unsigned int sourceOffset, void * dest, unsigned int originalByteSize)
{
	// Read the file table.

	const char *sourceData = NULL;

	int byteOffsetToStartOfCurFile = ftell(source);
	fseek(source, 0, SEEK_END);
	int bytesBetweenStartAndEndOfFile = ftell(source) - byteOffsetToStartOfCurFile;
	sourceData = new char[bytesBetweenStartAndEndOfFile];

	fseek(source, byteOffsetToStartOfCurFile, SEEK_SET);
	byteOffsetToStartOfCurFile = ftell(source);
	fread((void*)sourceData, 1, bytesBetweenStartAndEndOfFile, source);
	fclose(source);

	int nBytes_compressedData = LZ4_decompress_fast(sourceData, (char*)dest, originalByteSize);

	return nBytes_compressedData;
}
