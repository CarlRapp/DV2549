#ifndef PACKAGE_READER_WRITER_H
#define PACKAGE_READER_WRITER_H

#include <vector>
#include <string>

//#define NOMINMAX
#include <Windows.h>

#include "Compression\CompressionHandler_zlib.h"

struct PackageHeader
{
	unsigned int numFileTableEntries;
};

struct PackageFileTableEntry
{
	char fileName[30];
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
	Compression::ICompressionHandler *compressionHandler;

public:
	PackageReaderWriter(Compression::ICompressionHandler *compressionHandler);
	PackageReaderWriter();
	~PackageReaderWriter();

	/* Adds one or more file to the PAK, compressing them if so desired. */
	void createPackageFromFiles(std::string PAKFilePath, std::vector<std::string> filePaths, bool compressFiles = true);

	//PackageHeader loadPackageHeader(std::string packageFileName);
	std::vector<PackageFileTableEntry> loadPackageFileTable(std::string packageFileName);
	void loadPackageData(std::string packageFileName, void *dest, int _loadStartIndex = -1, int _loadEndIndex = -1);
	int getIndexOfResourceByName(std::string packageFileName, std::string resourceName);
};
#endif

