#include "PackageReaderWriter.h"

#include <unordered_map>
#include "Hash/MD5.h"


PackageReaderWriter::PackageReaderWriter()
{
	compressionHandler_lz4 = new Compression::CompressionHandler_lz4(); //new Compression::CompressionHandler_zlib();
	compressionHandler_zlib = new Compression::CompressionHandler_zlib();
}

PackageReaderWriter::~PackageReaderWriter()
{
	delete compressionHandler_lz4;
	delete compressionHandler_zlib;
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
		PackageFileTableEntry fileTableEntry;

		fopen_s(&fileToAdd, filePaths[i].c_str(), "rb");

		fseek(fileToAdd, 0, SEEK_END);
		int fileSize_unCompressed = ftell(fileToAdd);
		rewind(fileToAdd);

		// Append the file contents to the contents of the Package.
		if (compressFiles == true)
		{
			// Compress and append the file contents.
			nBytesAdded = compressionHandler_lz4->compress_fileToFile(fileToAdd, package.m_fileHandle);
		}

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

	if (package.m_header.numFileTableEntries == 0)
	{
		package.m_nextFileOffset = sizeOfHeaderAndFileTable;
		char *dummyData = new char[sizeOfHeaderAndFileTable] {0};
		fwrite(dummyData, sizeOfHeaderAndFileTable, 1, package.m_fileHandle);
	}

	/* unordered_map used for GUID checks */
	std::unordered_map<std::string, std::string>* hashedFiles = new std::unordered_map<std::string, std::string>();

	// Append the file data to the end of the Package file.
	for (unsigned int i = 0; i < filePaths.size(); ++i)
	{
		PackageFileTableEntry fileTableEntry;

		fopen_s(&fileToAdd, filePaths[i].c_str(), "rb");

		fseek(fileToAdd, 0, SEEK_END);
		int fileSize_unCompressed = ftell(fileToAdd);
		
		char* buffer = (char*)malloc(fileSize_unCompressed);
		rewind(fileToAdd);
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
		
		// Get the file extension of the file to add.
		size_t indexOfLastPeriod = filePaths[i].find_last_of('.');
		std::string fileExtension = filePaths[i].substr(indexOfLastPeriod, 258); // Not that important, but: Windows 7's character limit for filepaths is supposedly 260, so a file extension for a file with a name that's 1 char long can be at most 255, given the usage of 2 characters for the name and the period before the extension, and 3 for the drive letter, colon, and the initial slash.

		// Check if the file format (extension) is one that is already compressed/should not be compressed.
		fileTableEntry.compressionSetting = 1;
		if (fileExtension.compare("jpg") == 0)
			fileTableEntry.compressionSetting = 0;
		else if (fileExtension.compare("png") == 0)
			fileTableEntry.compressionSetting = 0;
		else if (fileExtension.compare("flac") == 0)
			fileTableEntry.compressionSetting = 0;
		else if (fileExtension.compare("ogg") == 0)
			fileTableEntry.compressionSetting = 0;
		else if (fileExtension.compare("mp3") == 0)
			fileTableEntry.compressionSetting = 0;

		// Append the file contents to the contents of the Package.
		if (fileTableEntry.compressionSetting != 0)
		{
			// Compress the file content with the chosen/appropriate compressor, and append them to the PAK file.
			rewind(fileToAdd);
			if(fileTableEntry.compressionSetting == 1)
				nBytesAdded = compressionHandler_lz4->compress_fileToFile(fileToAdd, package.m_fileHandle);
			if(fileTableEntry.compressionSetting == 2)
				nBytesAdded = compressionHandler_zlib->compress_fileToFile(fileToAdd, package.m_fileHandle);
		}
		else
		{
			// Without compressing the file content, append them to the PAK file.
			fwrite(buffer, 1, fileSize_unCompressed, package.m_fileHandle);
		}

		// Delete the buffer after the hash function is done with it and after, possibly, its contents have been written to the PAK (in the case of the contents not being compressed).
		delete[] buffer;

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
	// TODO Optimize this so it's only read once per frame, at the most. PRW could store "current package file header" or something.

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

// Set _loadStartIndex and _loadEndIndex
std::vector<LoadedFileInfo> PackageReaderWriter::loadPackageData(std::string packageFileName, void *dest, int _loadStartIndex, int _loadEndIndex, bool _loadEntirePackage)
{
	std::vector<LoadedFileInfo> loadedFileInfos;

	void *loadedData = NULL;
	unsigned int loadStartIndex, loadEndIndex;
	
	// Load the package file table entries.
	std::vector<PackageFileTableEntry> fileTableEntries = loadPackageFileTable(packageFileName);
		
	unsigned int startOffset;
	unsigned int endOffset;

	if (_loadEntirePackage == true)
	{
		loadStartIndex = 0;
		loadEndIndex = fileTableEntries.size() - 1;
	}
	else
	{
		loadStartIndex = _loadStartIndex;
		loadEndIndex = _loadEndIndex;
	}

	//if (_loadStartIndex == -1)
	//	loadStartIndex = 0;
	//else
	//	loadStartIndex = _loadStartIndex;

	//if (_loadEndIndex == -1)
	//	loadEndIndex = fileTableEntries.size() - 1;
	//else
	//	loadEndIndex = _loadStartIndex;

	//loadedData = new char[endOffset - startOffset];

	FILE *packageFileHandle;
	fopen_s(&packageFileHandle, packageFileName.c_str(), "rb");

	int nBytesLoaded = 0;
	unsigned int currentIndex = loadStartIndex;
	do 
	{
		LoadedFileInfo loadedFileInfo;

		// Get the file extension of the file to load.
		std::string fileName = std::string(fileTableEntries[currentIndex].fileName);
		size_t indexOfLastPeriod = fileName.find_last_of('.');
		loadedFileInfo.fileExtension = fileName.substr(indexOfLastPeriod, 258); // Not that important, but: Windows 7's character limit for filepaths is supposedly 260, so a file extension for a file with a name that's 1 char long can be at most 255, given the usage of 2 characters for the name and the period before the extension, and 3 for the drive letter, colon, and the initial slash.

		unsigned int startOffset = fileTableEntries[currentIndex].fileOffset;

		fseek(packageFileHandle, startOffset, SEEK_SET);
		//unsigned int nFileSize = fileTableEntries[currentIndex + 1].fileOffset - startOffset;
		
		if (fileTableEntries[currentIndex].compressionSetting != 0)
		{
			if (fileTableEntries[currentIndex].compressionSetting == 1)
				nBytesLoaded += compressionHandler_lz4->deCompress_fileToMemory(packageFileHandle, startOffset, (char*)dest + nBytesLoaded, fileTableEntries[currentIndex].fileSize_uncompressed, fileTableEntries[currentIndex].fileSize_compressed);
			else if (fileTableEntries[currentIndex].compressionSetting == 2)
				nBytesLoaded += compressionHandler_zlib->deCompress_fileToMemory(packageFileHandle, startOffset, (char*)dest + nBytesLoaded, fileTableEntries[currentIndex].fileSize_uncompressed, fileTableEntries[currentIndex].fileSize_compressed);
		
			loadedFileInfo.size_bytes = fileTableEntries[currentIndex].fileSize_uncompressed;
			loadedFileInfo.offset_bytes = nBytesLoaded;
		}
		else
		{
			nBytesLoaded += fread((char*)dest + nBytesLoaded, 1, fileTableEntries[currentIndex].fileSize_uncompressed, packageFileHandle);
			loadedFileInfo.size_bytes = fileTableEntries[currentIndex].fileSize_uncompressed;
			loadedFileInfo.offset_bytes = nBytesLoaded;
		}
				
		loadedFileInfos.push_back(loadedFileInfo);

		++currentIndex;
	} while (currentIndex < loadEndIndex);

	fclose(packageFileHandle);

	return loadedFileInfos;
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