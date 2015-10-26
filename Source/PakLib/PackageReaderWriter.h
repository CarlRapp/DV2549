#ifndef PACKAGE_READER_WRITER_H
#define PACKAGE_READER_WRITER_H

#include <vector>
#include <string>

//#define NOMINMAX
#include <Windows.h>

#include "Compression\CompressionHandler_zlib.h"
#include "Compression\CompressionHandler_lz4.h"


struct LoadedFileInfo
{
	std::string fileExtension;
	long offset_bytes;
	long size_bytes;
};

struct PackageHeader
{
	unsigned int numFileTableEntries;
};

struct PackageFileTableEntry
{
	char fileName[30];
	BYTE compressionSetting;
	unsigned int fileSize_uncompressed;
	unsigned int fileSize_compressed;
	unsigned int fileOffset; // The offset, in bytes, to the first byte of the file, in its PAK file.
	//PAKFileTableEntry *nextEntry;

	//PAKFileTableEntry()
	//{
	//	ZeroMemory(&fileName, sizeof(fileName));
	//	fileSize = 0;
	//	fileOffset = 0;
	//	nextEntry = NULL;
	//}

	//~PAKFileTableEntry()
	//{
	//	ZeroMemory(&fileName, sizeof(fileName));
	//	fileSize = 0;
	//	fileOffset = 0;
	//	delete nextEntry;
	//}
};

struct Package
{
	FILE *m_fileHandle;
	std::string m_filePath;
	PackageHeader m_header;
	std::vector<PackageFileTableEntry> m_fileTableEntries;
	unsigned int m_nextFileOffset;
};

class DECLSPEC PackageReaderWriter
{
private:
	Compression::ICompressionHandler *compressionHandler_zlib;
	Compression::ICompressionHandler *compressionHandler_lz4;

public:
	PackageReaderWriter(Compression::ICompressionHandler *compressionHandler);
	PackageReaderWriter();
	~PackageReaderWriter();

	/* Adds one or more file to the PAK, compressing them if so desired. */
	void createPackageFromFiles(std::string PAKFilePath, std::vector<std::string> filePaths, bool compressFiles = true);
	void createPackageFromUniqueFiles(std::string PAKFilePath, std::vector<std::string> filePaths, bool compressFiles = true);

	//PackageHeader loadPackageHeader(std::string packageFileName);
	std::vector<PackageFileTableEntry> loadPackageFileTable(std::string packageFileName);

	// Load data from package within a given range set by _loadStartIndex and _loadEndIndex, or load the entire package by setting _loadEntirePackage to true.
	std::vector<LoadedFileInfo> loadPackageData(std::string packageFileName, void *dest, int _loadStartIndex, int _loadEndIndex, bool _loadEntirePackage = false);
	int getIndexOfResourceByName(std::string packageFileName, std::string resourceName);
};
#endif

