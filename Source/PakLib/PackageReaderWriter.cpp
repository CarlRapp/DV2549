#include "PackageReaderWriter.h"

#include <unordered_map>
#include "Hash/MD5.h"

PackageReaderWriter::PackageReaderWriter(Compression::ICompressionHandler *_compressionHandler)
{
	compressionHandler = _compressionHandler;
}

PackageReaderWriter::PackageReaderWriter()
{
	compressionHandler = new Compression::CompressionHandler_lz4(); //new Compression::CompressionHandler_zlib();
}

PackageReaderWriter::~PackageReaderWriter()
{
	delete compressionHandler;
}

void PackageReaderWriter::createPackageFromFiles(std::string PAKFilePath, std::vector<std::string> filePaths, bool compressFiles)
{
	Package package;
	package.m_header.numFileTableEntries = 0;
	package.m_filePath = PAKFilePath;

	FILE *fileToAdd;

	int nBytesAdded = 0;

	std::vector<PackageFileTableEntry> fileTableEntries;

	unsigned int sizeOfHeaderAndFileTable = sizeof(PackageHeader) + filePaths.size() * sizeof(PackageFileTableEntry);

	/* Destroy old file if it exists and create new */
	fopen_s(&package.m_fileHandle, package.m_filePath.c_str(), "wb");
	//fclose(package.m_fileHandle);

	if (package.m_header.numFileTableEntries == 0)
	{
		package.m_nextFileOffset = sizeOfHeaderAndFileTable;

		// Write dummy data to move the file pointer.
		//fopen_s(&package.m_fileHandle, package.m_filePath.c_str(), "ab");
		char *dummyData = new char[sizeOfHeaderAndFileTable] {0};
		fwrite(dummyData, sizeOfHeaderAndFileTable, 1, package.m_fileHandle);
		//fclose(package.m_fileHandle);
	}

	// Append the file data to the end of the Package file.
	//fopen_s(&package.m_fileHandle, package.m_filePath.c_str(), "wb"); // Open the Package file for writing at the end of it. NOTE: Opening with "a" keeps the last EOF marker, which should work well with repeated use of the current version of decompress_fileToMemory.
	for (unsigned int i = 0; i < filePaths.size(); ++i)
	{
		
		fopen_s(&fileToAdd, filePaths[i].c_str(), "rb");

		fseek(fileToAdd, 0, SEEK_END);
		int fileSize_unCompressed = ftell(fileToAdd);
		rewind(fileToAdd);

		// Append the file contents to the contents of the Package.
		if (compressFiles == true)
		{
			// Compress and append the file contents.
			nBytesAdded = compressionHandler->compress_fileToFile(fileToAdd, package.m_fileHandle);
		}

		PackageFileTableEntry fileTableEntry;
		int startIndex = filePaths[i].find_last_of('/') + 1;
		strncpy_s(fileTableEntry.fileName, filePaths[i].c_str() + startIndex, filePaths[i].size() - startIndex);
		fileTableEntry.fileName[sizeof(fileTableEntry.fileName) - 1] = 0;
		fileTableEntry.fileOffset = package.m_nextFileOffset; // Add the offset to the start of the added file.
		fileTableEntry.fileSize_compressed = nBytesAdded;
		fileTableEntry.fileSize_uncompressed = fileSize_unCompressed;
		fileTableEntries.push_back(fileTableEntry);
		
		// Increment the file offset so the next eventual file to be added is added after the current one.
		package.m_nextFileOffset = fileTableEntry.fileOffset + nBytesAdded;

		// Do I have to add the size of EOF to the offset or is it only relevant to the OS / FILE class handling?
		++package.m_header.numFileTableEntries;

		fclose(fileToAdd); 
		//fclose(package.m_fileHandle); // NOTE: Adds EOF marker while closing.
	}
		
	//fopen_s(&package.m_fileHandle, package.m_filePath.c_str(), "r+"); // Opens for reading and writing without destroying the already existing file.
	fseek(package.m_fileHandle, 0, SEEK_SET); // Set the read/write marker to the start of the Package File.

	// Write the header the start of the Package file.
	fwrite(&package.m_header, sizeof(PackageHeader), 1, package.m_fileHandle);

	// Write the file table to the Package file, after the header.
	for (unsigned int i = 0; i < filePaths.size(); ++i)
	{
		fwrite(&fileTableEntries[i], sizeof(PackageFileTableEntry), 1, package.m_fileHandle);
	}

	fclose(package.m_fileHandle);

	// NOTE: In-game, first read sizeof(header) to see how many sizeof(PackageFileTableEntry) to read in to get all resource names and offsets.
}

void PackageReaderWriter::createPackageFromUniqueFiles(std::string PAKFilePath, std::vector<std::string> filePaths, bool compressFiles)
{
	Package package;
	package.m_header.numFileTableEntries = 0;
	package.m_filePath = PAKFilePath;

	FILE *fileToAdd;

	int nBytesAdded = 0;

	std::vector<PackageFileTableEntry> fileTableEntries;

	unsigned int sizeOfHeaderAndFileTable = sizeof(PackageHeader) + filePaths.size() * sizeof(PackageFileTableEntry);

	/* Destroy old file if it exists and create new */
	fopen_s(&package.m_fileHandle, package.m_filePath.c_str(), "wb");
	//fclose(package.m_fileHandle);

	if (package.m_header.numFileTableEntries == 0)
	{
		package.m_nextFileOffset = sizeOfHeaderAndFileTable;

		// Write dummy data to move the file pointer.
		//fopen_s(&package.m_fileHandle, package.m_filePath.c_str(), "ab");
		char *dummyData = new char[sizeOfHeaderAndFileTable] {0};
		fwrite(dummyData, sizeOfHeaderAndFileTable, 1, package.m_fileHandle);
		//fclose(package.m_fileHandle);
	}

	/* unordered_map used for GUID checks */
	std::unordered_map<std::string, std::string>* hashedFiles = new std::unordered_map<std::string, std::string>();

	// Append the file data to the end of the Package file.
	//fopen_s(&package.m_fileHandle, package.m_filePath.c_str(), "wb"); // Open the Package file for writing at the end of it. NOTE: Opening with "a" keeps the last EOF marker, which should work well with repeated use of the current version of decompress_fileToMemory.
	for (unsigned int i = 0; i < filePaths.size(); ++i)
	{

		fopen_s(&fileToAdd, filePaths[i].c_str(), "rb");

		fseek(fileToAdd, 0, SEEK_END);
		int fileSize_unCompressed = ftell(fileToAdd);
		rewind(fileToAdd);

		char* buffer = (char*)malloc(fileSize_unCompressed);
		fread(buffer, sizeof(char), fileSize_unCompressed, fileToAdd);

		MD5 hash;
		hash.update(buffer, fileSize_unCompressed);
		hash.finalize();
		std::string hashedFile = hash.hexdigest();

		auto it = hashedFiles->find(hashedFile);
		if (it != hashedFiles->end())
		{
			printf("The content of a unique file was found twice in %s and %s\n", filePaths[i].c_str(), it->second.c_str());
		}
		else
		{
			hashedFiles->insert(std::pair<std::string, std::string>(hashedFile, filePaths[i].c_str()));
		}

		delete[] buffer;

		// Append the file contents to the contents of the Package.
		if (compressFiles == true)
		{
			// Compress and append the file contents.
			nBytesAdded = compressionHandler->compress_fileToFile(fileToAdd, package.m_fileHandle);
		}

		PackageFileTableEntry fileTableEntry;
		int startIndex = filePaths[i].find_last_of('/') + 1;
		strncpy_s(fileTableEntry.fileName, filePaths[i].c_str() + startIndex, filePaths[i].size() - startIndex);
		fileTableEntry.fileName[sizeof(fileTableEntry.fileName) - 1] = 0;
		fileTableEntry.fileOffset = package.m_nextFileOffset; // Add the offset to the start of the added file.
		fileTableEntry.fileSize_compressed = nBytesAdded;
		fileTableEntry.fileSize_uncompressed = fileSize_unCompressed;
		fileTableEntries.push_back(fileTableEntry);

		// Increment the file offset so the next eventual file to be added is added after the current one.
		package.m_nextFileOffset = fileTableEntry.fileOffset + nBytesAdded;

		// Do I have to add the size of EOF to the offset or is it only relevant to the OS / FILE class handling?
		++package.m_header.numFileTableEntries;

		fclose(fileToAdd);
		//fclose(package.m_fileHandle); // NOTE: Adds EOF marker while closing.
	}

	delete hashedFiles;

	//fopen_s(&package.m_fileHandle, package.m_filePath.c_str(), "r+"); // Opens for reading and writing without destroying the already existing file.
	fseek(package.m_fileHandle, 0, SEEK_SET); // Set the read/write marker to the start of the Package File.

											  // Write the header the start of the Package file.
	fwrite(&package.m_header, sizeof(PackageHeader), 1, package.m_fileHandle);

	// Write the file table to the Package file, after the header.
	for (unsigned int i = 0; i < filePaths.size(); ++i)
	{
		fwrite(&fileTableEntries[i], sizeof(PackageFileTableEntry), 1, package.m_fileHandle);
	}

	fclose(package.m_fileHandle);

	// NOTE: In-game, first read sizeof(header) to see how many sizeof(PackageFileTableEntry) to read in to get all resource names and offsets.
}

//PackageHeader PackageReaderWriter::loadPackageHeader(std::string packageFileName)
//{
//	PackageHeader header;
//
//	return header;
//}

std::vector<PackageFileTableEntry> PackageReaderWriter::loadPackageFileTable(std::string packageFileName)
{
	PackageHeader header;
	//ZeroMemory(&header, sizeof(PackageHeader));

	FILE *packageFileHandle;
	fopen_s(&packageFileHandle, packageFileName.c_str(), "rb");

	// Load the package header.
	fread(&header, sizeof(PackageHeader), 1, packageFileHandle);

	// Load the package file table entries.
	std::vector<PackageFileTableEntry> fileTableEntries;
	for (unsigned int i = 0; i < header.numFileTableEntries; ++i)
	{
		long fp = ftell(packageFileHandle);
		PackageFileTableEntry fileTableEntry;
		fread(&fileTableEntry, sizeof(PackageFileTableEntry), 1, packageFileHandle);
		fileTableEntries.push_back(fileTableEntry);
	}

	fclose(packageFileHandle);
	
	return fileTableEntries;
}

void PackageReaderWriter::loadPackageData(std::string packageFileName, void *dest, int _loadStartIndex, int _loadEndIndex)
{
	void *loadedData = NULL;
	unsigned int loadStartIndex, loadEndIndex;
	
	// Load the package file table entries.
	std::vector<PackageFileTableEntry> fileTableEntries = loadPackageFileTable(packageFileName);
		
	unsigned int startOffset;
	unsigned int endOffset;
	if (_loadStartIndex == -1)
		loadStartIndex = 0;
	else
		loadStartIndex = _loadStartIndex;

	if (_loadEndIndex == -1)
		loadEndIndex = 0;
	else
		loadEndIndex = _loadStartIndex;

	//loadedData = new char[endOffset - startOffset];

	FILE *packageFileHandle;
	fopen_s(&packageFileHandle, packageFileName.c_str(), "rb");

	int nBytesLoaded = 0;
	unsigned int currentIndex = loadStartIndex;
	do 
	{
		unsigned int startOffset = fileTableEntries[currentIndex].fileOffset;
		fseek(packageFileHandle, startOffset, SEEK_SET);
		//unsigned int nFileSize = fileTableEntries[currentIndex + 1].fileOffset - startOffset;
		nBytesLoaded += compressionHandler->deCompress_fileToMemory(packageFileHandle, startOffset, (char*)dest + nBytesLoaded, fileTableEntries[currentIndex].fileSize_uncompressed, fileTableEntries[currentIndex].fileSize_compressed);  // NOTE: Continuously moves the file pointer and reads until various EOF markers in the Package, I think...
		++currentIndex;
	} while (currentIndex < loadEndIndex);

	fclose(packageFileHandle);
}

int PackageReaderWriter::getIndexOfResourceByName(std::string packageFileName, std::string resourceName)
{
	std::vector<PackageFileTableEntry> fileTableEntries = loadPackageFileTable(packageFileName);

	int indexOfResource = -1;

	for (unsigned int i = 0; i << fileTableEntries.size(); ++i)
	{
		if (strcmp(fileTableEntries[i].fileName, resourceName.c_str()) == 0)
		{
			indexOfResource = i;
		}
	}

	return indexOfResource;
}