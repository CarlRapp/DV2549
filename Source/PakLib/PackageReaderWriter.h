#ifndef PACKAGE_READER_WRITER_H
#define PACKAGE_READER_WRITER_H

#include <vector>
#include <string>

//#define NOMINMAX
#include <Windows.h>
#include <unordered_map>

#include "Compression\CompressionHandler_zlib.h"
#include "Compression\CompressionHandler_lz4.h"

enum FileFormat {RAW = 0, PNG, JPG, FLAC, OGG, MP3};

struct LoadedFileInfo
{
	FileFormat fileFormat;
	long offset_bytes;
	long size_bytes;
};

struct HashFileInfo
{
	std::string fileName;
	int index;
};

struct DuplicateDataInfo
{
	bool duplicateData;
	unsigned int redirectIndex;
};

struct PackageHeader
{
	unsigned int numFileTableEntries;
};

struct PackageFileTableEntry
{
	char fileName[30];
	int compressionSetting;
	unsigned int fileSize_uncompressed;
	unsigned int fileSize_compressed;
	unsigned int fileOffset; // The offset, in bytes, to the first byte of the file, in its PAK file.
	int redirectIndex;
	FileFormat fileFormat;
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
	std::unordered_map<std::string, std::vector<PackageFileTableEntry>> map_PAKnameToFileTable;

	// Compression/decompression handlers:
	Compression::ICompressionHandler *compressionHandler_zlib;
	Compression::ICompressionHandler *compressionHandler_lz4;

public:
	PackageReaderWriter(Compression::ICompressionHandler *compressionHandler);
	PackageReaderWriter();
	~PackageReaderWriter();

	std::vector<std::string> interleaveInputFolderFilePaths(std::vector<std::vector<std::string>> filePathLists);
	void createPackageByInterleavingInputFolderFiles(std::string PAKFilePath, std::vector<std::vector<std::string>> filePathLists); // Not quite finished.

	void storeFileTableForPackage(std::string PAKname);
	bool getFileTableOfPackage(std::string PAKname, std::vector<PackageFileTableEntry> &out_fileTableEntries);

	/* Adds one or more file to the PAK, compressing them if so desired. */
	void createPackageFromFiles(std::string PAKFilePath, std::vector<std::string> filePaths, bool compressFiles = true);
	void createPackageFromUniqueFiles(std::string PAKFilePath, std::vector<std::string> filePaths, bool compressFiles = true);
	void createPackageFromUniqueFiles2(std::string PAKFilePath, std::vector<std::string> filePaths, bool compressFiles = true); // Unfinished.

	//PackageHeader loadPackageHeader(std::string packageFileName);
	std::vector<PackageFileTableEntry> loadPackageFileTable(std::string packageFileName);

	// Load data from package within a given a specific index in the PAK file to load data from.
	std::vector<LoadedFileInfo> loadPackageData(std::string packageFileName, void *&dest, int _loadStartIndex, int _loadEndIndex, bool _loadEntirePackage = false);
	std::vector<LoadedFileInfo> loadPackageData2(std::string packageFileName, void *dest, int _loadStartIndex, int _loadEndIndex, bool _loadEntirePackage = false); // Unfinished.
	int getIndexOfResourceByName(std::string packageFileName, std::string resourceName);
};
#endif

