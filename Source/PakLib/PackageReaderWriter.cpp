#include "PackageReaderWriter.h"

#include <unordered_map>
#include "Hash/MD5.h"


PackageReaderWriter::PackageReaderWriter()
{
	compressionHandler_lz4 = new Compression::CompressionHandler_lz4();
	compressionHandler_zlib = new Compression::CompressionHandler_zlib();
}

PackageReaderWriter::~PackageReaderWriter()
{
	delete compressionHandler_lz4;
	delete compressionHandler_zlib;
}

std::vector<std::string> PackageReaderWriter::interleaveInputFolderFilePaths(std::vector<std::vector<std::string>> filePathLists)
{
	std::vector<std::string> interleavedFilePaths;

	for (unsigned int listItemIndex = 0; listItemIndex < filePathLists[0].size(); ++listItemIndex)
	{
		for (unsigned int listIndex = 0; listIndex < filePathLists.size(); ++listIndex)
		{
			interleavedFilePaths.push_back(filePathLists[listIndex][listItemIndex]);
		}
	}

	return interleavedFilePaths;
}

void PackageReaderWriter::createPackageByInterleavingInputFolderFiles(std::string PAKFilePath, std::vector<std::vector<std::string>> filePathLists)
{
	std::vector<std::string> interleavedFilePaths = interleaveInputFolderFilePaths(filePathLists);

	// What about compressing X files as 1 file and decompressing them together?

	createPackageFromFiles(PAKFilePath, interleavedFilePaths);
}

void PackageReaderWriter::storeFileTableForPackage(std::string PAKname)
{
	map_PAKnameToFileTable.insert(std::pair<std::string, std::vector<PackageFileTableEntry>>(PAKname.c_str(), loadPackageFileTable(PAKname)));
}

bool PackageReaderWriter::getFileTableOfPackage(std::string PAKname, std::vector<PackageFileTableEntry> &out_fileTableEntries)
{
	bool tableFound = false;

	std::unordered_map<std::string, std::vector<PackageFileTableEntry>>::iterator iter;
	iter = map_PAKnameToFileTable.find(PAKname);
	if (iter != map_PAKnameToFileTable.end())
	{
		out_fileTableEntries = iter->second;
		tableFound = true;
	}

	return tableFound;
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

	// Destroy old file if it exists and create new.
	fopen_s(&package.m_fileHandle, package.m_filePath.c_str(), "wb");

	if (package.m_header.numFileTableEntries == 0)
	{
		package.m_nextFileOffset = sizeOfHeaderAndFileTable;

		// Write dummy data to move the file pointer.
		char *dummyData = new char[sizeOfHeaderAndFileTable] {0};
		fwrite(dummyData, sizeOfHeaderAndFileTable, 1, package.m_fileHandle);
	}

	// Append the file data to the end of the Package file.
	PackageFileTableEntry fileTableEntry;
	for (unsigned int i = 0; i < filePaths.size(); ++i)
	{
		ZeroMemory(&fileTableEntry, sizeof(PackageFileTableEntry));
		fileTableEntry.redirectIndex = -1;

		fopen_s(&fileToAdd, filePaths[i].c_str(), "rb");

		fseek(fileToAdd, 0, SEEK_END);
		int fileSize_unCompressed = ftell(fileToAdd);

		rewind(fileToAdd);

		// Get the file extension of the file to add.
		size_t indexOfLastPeriod = filePaths[i].find_last_of('.');
		std::string fileExtension = filePaths[i].substr(indexOfLastPeriod + 1, 258); // Not that important, but: Windows 7's character limit for filepaths is supposedly 260, so a file extension for a file with a name that's 1 char long can be at most 255, given the usage of 2 characters for the name and the period before the extension, and 3 for the drive letter, colon, and the initial slash.

		// Check if the file format (extension) is one that is already compressed/should not be compressed.
		fileTableEntry.compressionSetting = 1; // Hard-coded setting due to the inferiority of ZLib.
		if (fileExtension.compare("png") == 0)
		{
			fileTableEntry.compressionSetting = 0;
			fileTableEntry.fileFormat = PNG;
		}
		else if (fileExtension.compare("jpg") == 0)
		{
			fileTableEntry.compressionSetting = 0;
			fileTableEntry.fileFormat = JPG;
		}
		else if (fileExtension.compare("flac") == 0)
		{
			fileTableEntry.compressionSetting = 0;
			fileTableEntry.fileFormat = FLAC;
		}
		else if (fileExtension.compare("ogg") == 0)
		{
			fileTableEntry.compressionSetting = 0;
			fileTableEntry.fileFormat = OGG;
		}
		else if (fileExtension.compare("mp3") == 0)
		{
			fileTableEntry.compressionSetting = 0;
			fileTableEntry.fileFormat = MP3;
		}
		else
		{
			fileTableEntry.compressionSetting = 1;
			fileTableEntry.fileFormat = RAW;
		}
		
		// Append the file contents to the contents of the Package.
		if (fileTableEntry.compressionSetting != 0)
		{
			// Compress the file content with the chosen/appropriate compressor, and append them to the PAK file.
			rewind(fileToAdd);
			if (fileTableEntry.compressionSetting == 1)
				nBytesAdded = compressionHandler_lz4->compress_fileToFile(fileToAdd, package.m_fileHandle);
			else if (fileTableEntry.compressionSetting == 2)
				nBytesAdded = compressionHandler_zlib->compress_fileToFile(fileToAdd, package.m_fileHandle);
		}
		else // Don't compress, due to file format already having compression.
		{
			rewind(fileToAdd);
			char *buffer = new char[fileSize_unCompressed]; // Create a buffer to hold the file data to add to the PAK.
			fread(buffer, 1, fileSize_unCompressed, fileToAdd); // Read in the file data, which is to be added to the PAK, to the buffer.
			nBytesAdded = fwrite(buffer, 1, fileSize_unCompressed, package.m_fileHandle); // Append the buffer's file data to the PAK, without any compression.
			delete[] buffer;
		}

		int startIndex = filePaths[i].find_last_of('/') + 1;
		strncpy_s(fileTableEntry.fileName, filePaths[i].c_str() + startIndex, filePaths[i].size() - startIndex);
		fileTableEntry.fileName[sizeof(fileTableEntry.fileName) - 1] = 0;
		fileTableEntry.fileOffset = package.m_nextFileOffset; // Add the offset to the start of the added file.
		fileTableEntry.fileSize_compressed = nBytesAdded;
		fileTableEntry.fileSize_uncompressed = fileSize_unCompressed;

		// Increment the file offset so the next eventual file to be added is added after the current one.
		package.m_nextFileOffset = fileTableEntry.fileOffset + nBytesAdded;

		fileTableEntries.push_back(fileTableEntry);

		++package.m_header.numFileTableEntries;

		fclose(fileToAdd);
	}

	fseek(package.m_fileHandle, 0, SEEK_SET); // Set the read/write marker to the start of the Package File.

	// Write the header the start of the Package file.
	fwrite(&package.m_header, sizeof(PackageHeader), 1, package.m_fileHandle);

	// Write the file table to the Package file, after the header.
	for (unsigned int i = 0; i < filePaths.size(); ++i)
	{
		fwrite(&fileTableEntries[i], sizeof(PackageFileTableEntry), 1, package.m_fileHandle);
	}

	fclose(package.m_fileHandle);
}

void PackageReaderWriter::createPackageFromUniqueFiles(std::string PAKFilePath, std::vector<std::string> filePaths, bool compressFiles)
{
	Package package;
	package.m_header.numFileTableEntries = 0;
	package.m_filePath = PAKFilePath;

	FILE *fileToAdd;

	// Remove copies of the same file.
	std::unordered_map<std::string, std::string>* hashedFiles = new std::unordered_map<std::string, std::string>();
	std::vector<std::string> filteredFiles;
	for (unsigned int i = 0; i < filePaths.size(); ++i)
	{
		fopen_s(&fileToAdd, filePaths[i].c_str(), "rb");
		fseek(fileToAdd, 0, SEEK_END);
		int fileSize_unCompressed = ftell(fileToAdd);
		rewind(fileToAdd);

		char* buffer = (char*)malloc(fileSize_unCompressed);
		fread(buffer, sizeof(char), fileSize_unCompressed, fileToAdd);
		rewind(fileToAdd);

		MD5 hash;
		hash.update(buffer, fileSize_unCompressed);
		hash.finalize();
		std::string hashedFile = hash.hexdigest();

		auto it = hashedFiles->find(hashedFile);
		if (it != hashedFiles->end())
		{
			printf("The content of a unique file was found twice in %s and %s. The file will not be added to the PAK file.\n", filePaths[i].c_str(), it->second.c_str());
		}
		else
		{
			hashedFiles->insert(std::pair<std::string, std::string>(hashedFile, filePaths[i].c_str()));
			filteredFiles.push_back(filePaths[i]);
		}

		delete[] buffer;
		fclose(fileToAdd);
	}
	filePaths = filteredFiles;

	int nBytesAdded = 0;

	std::vector<PackageFileTableEntry> fileTableEntries;

	unsigned int sizeOfHeaderAndFileTable = sizeof(PackageHeader) + filePaths.size() * sizeof(PackageFileTableEntry);

	// Destroy old file if it exists and create new.
	fopen_s(&package.m_fileHandle, package.m_filePath.c_str(), "wb");

	if (package.m_header.numFileTableEntries == 0)
	{
		package.m_nextFileOffset = sizeOfHeaderAndFileTable;

		// Write dummy data to move the file pointer.
		char *dummyData = new char[sizeOfHeaderAndFileTable] {0};
		fwrite(dummyData, sizeOfHeaderAndFileTable, 1, package.m_fileHandle);
	}

	// Append the file data to the end of the Package file, unless the same chunk of data has already been added to the PAK.
	PackageFileTableEntry fileTableEntry;
	for (unsigned int i = 0; i < filePaths.size(); ++i)
	{
		ZeroMemory(&fileTableEntry, sizeof(PackageFileTableEntry));
		fileTableEntry.redirectIndex = -1;

		fopen_s(&fileToAdd, filePaths[i].c_str(), "rb");

		fseek(fileToAdd, 0, SEEK_END);
		int fileSize_unCompressed = ftell(fileToAdd);

		rewind(fileToAdd);

		// Get the file extension of the file to add.
		size_t indexOfLastPeriod = filePaths[i].find_last_of('.');
		std::string fileExtension = filePaths[i].substr(indexOfLastPeriod, 258); // Not that important, but: Windows 7's character limit for filepaths is supposedly 260, so a file extension for a file with a name that's 1 char long can be at most 255, given the usage of 2 characters for the name and the period before the extension, and 3 for the drive letter, colon, and the initial slash.

		// Check if the file format (extension) is one that is already compressed/should not be compressed.
		fileTableEntry.compressionSetting = 1;
		if (fileExtension.compare("png") == 0)
		{
			fileTableEntry.compressionSetting = 0;
			fileTableEntry.fileFormat = PNG;
		}
		else if (fileExtension.compare("jpg") == 0)
		{
			fileTableEntry.compressionSetting = 0;
			fileTableEntry.fileFormat = JPG;
		}
		else if (fileExtension.compare("flac") == 0)
		{
			fileTableEntry.compressionSetting = 0;
			fileTableEntry.fileFormat = FLAC;
		}
		else if (fileExtension.compare("ogg") == 0)
		{
			fileTableEntry.compressionSetting = 0;
			fileTableEntry.fileFormat = OGG;
		}
		else if (fileExtension.compare("mp3") == 0)
		{
			fileTableEntry.compressionSetting = 0;
			fileTableEntry.fileFormat = MP3;
		}
		else
		{
			fileTableEntry.compressionSetting = 1;
			fileTableEntry.fileFormat = RAW;
		}

		// Append the file contents to the contents of the Package.
		if (fileTableEntry.compressionSetting != 0)
		{
			// Compress the file content with the chosen/appropriate compressor, and append them to the PAK file.
			rewind(fileToAdd);
			if (fileTableEntry.compressionSetting == 1)
				nBytesAdded = compressionHandler_lz4->compress_fileToFile(fileToAdd, package.m_fileHandle);
			else if (fileTableEntry.compressionSetting == 2)
				nBytesAdded = compressionHandler_zlib->compress_fileToFile(fileToAdd, package.m_fileHandle);
		}
		else // Don't compress due to file format already having compression.
		{
			rewind(fileToAdd);
			char *buffer = new char[fileSize_unCompressed]; // Create a buffer to hold the file data to add to the PAK.
			fread(buffer, 1, fileSize_unCompressed, fileToAdd); // Read in the file data, which is to be added to the PAK, to the buffer.
			nBytesAdded = fwrite(buffer, 1, fileSize_unCompressed, package.m_fileHandle); // Append the buffer's file data to the PAK, without any compression.
			delete[] buffer;
		}

		int startIndex = filePaths[i].find_last_of('/') + 1;
		strncpy_s(fileTableEntry.fileName, filePaths[i].c_str() + startIndex, filePaths[i].size() - startIndex);
		fileTableEntry.fileName[sizeof(fileTableEntry.fileName) - 1] = 0;
		fileTableEntry.fileOffset = package.m_nextFileOffset; // Add the offset to the start of the added file.
		fileTableEntry.fileSize_compressed = nBytesAdded;
		fileTableEntry.fileSize_uncompressed = fileSize_unCompressed;

		// Increment the file offset so the next eventual file to be added is added after the current one.
		package.m_nextFileOffset = fileTableEntry.fileOffset + nBytesAdded;

		fileTableEntries.push_back(fileTableEntry);

		++package.m_header.numFileTableEntries;

		fclose(fileToAdd);
	}

	delete hashedFiles;

	fseek(package.m_fileHandle, 0, SEEK_SET); // Set the read/write marker to the start of the Package File.

	// Write the header the start of the Package file.
	fwrite(&package.m_header, sizeof(PackageHeader), 1, package.m_fileHandle);

	// Write the file table to the Package file, after the header.
	for (unsigned int i = 0; i < filePaths.size(); ++i)
	{
		fwrite(&fileTableEntries[i], sizeof(PackageFileTableEntry), 1, package.m_fileHandle);
	}

	fclose(package.m_fileHandle);
}

void PackageReaderWriter::createPackageFromUniqueFiles2(std::string PAKFilePath, std::vector<std::string> filePaths, bool compressFiles)
{
	Package package;
	package.m_header.numFileTableEntries = 0;
	package.m_filePath = PAKFilePath;

	std::vector<PackageFileTableEntry> fileTableEntries;

	FILE *fileToAdd;

	// Remove copies of the same file
	std::unordered_map<std::string, HashFileInfo>* hashedFiles = new std::unordered_map<std::string, HashFileInfo>();
	std::vector<DuplicateDataInfo> duplicateDataInfos; //std::vector<std::string> filteredFiles;
	
	int counter = 0;
	for (unsigned int i = 0; i < filePaths.size(); ++i)
	{
		DuplicateDataInfo duplicateDataInfo;
		duplicateDataInfo.duplicateData = false;

		if (counter == 507)
		{
			int test = 4;
		}
		++counter;

		fopen_s(&fileToAdd, filePaths[i].c_str(), "rb");
		fseek(fileToAdd, 0, SEEK_END);
		int fileSize_unCompressed = ftell(fileToAdd);
		rewind(fileToAdd);

		char* buffer = (char*)malloc(fileSize_unCompressed);
		fread(buffer, sizeof(char), fileSize_unCompressed, fileToAdd);
		rewind(fileToAdd);

		MD5 hash;
		hash.update(buffer, fileSize_unCompressed);
		hash.finalize();
		std::string hashedFile = hash.hexdigest();

		auto it = hashedFiles->find(hashedFile);
		if (it != hashedFiles->end())
		{
			//printf("The content of a unique file was found twice in %s and %s. The file will not be added to the PAK file.\n", filePaths[i].c_str(), it->second.fileName);
		
			duplicateDataInfo.duplicateData = true;
			duplicateDataInfo.redirectIndex = it->second.index;
		}
		else
		{
			HashFileInfo hashFileInfo;
			hashFileInfo.fileName = std::string(filePaths[i]);
			hashFileInfo.index = i;
			hashedFiles->insert(std::pair<std::string, HashFileInfo>(hashedFile, hashFileInfo));
		}

		delete[] buffer;

		fclose(fileToAdd);

		duplicateDataInfos.push_back(duplicateDataInfo);
	}
	//filePaths = filteredFiles;

	int t = duplicateDataInfos.size();

	int nBytesAdded = 0;
	
	unsigned int sizeOfHeaderAndFileTable = sizeof(PackageHeader) + filePaths.size() * sizeof(PackageFileTableEntry);

	/* Destroy old file if it exists and create new */
	fopen_s(&package.m_fileHandle, package.m_filePath.c_str(), "wb");

	if (package.m_header.numFileTableEntries == 0)
	{
		package.m_nextFileOffset = sizeOfHeaderAndFileTable;
		char *dummyData = new char[sizeOfHeaderAndFileTable] {0};
		fwrite(dummyData, sizeOfHeaderAndFileTable, 1, package.m_fileHandle);
	}

	// Append the file data to the end of the Package file, unless the same chunk of data has already been added to the PAK.
	PackageFileTableEntry fileTableEntry;
	for (unsigned int i = 0; i < filePaths.size(); ++i)
	{
		ZeroMemory(&fileTableEntry, sizeof(PackageFileTableEntry));
		fileTableEntry.redirectIndex = -1;

		fopen_s(&fileToAdd, filePaths[i].c_str(), "rb");

		fseek(fileToAdd, 0, SEEK_END);
		int fileSize_unCompressed = ftell(fileToAdd);
		
		rewind(fileToAdd);
		
		//duplicateDataInfos[i].duplicateData = false;
		if (duplicateDataInfos[i].duplicateData != true)
		{
			// Get the file extension of the file to add.
			size_t indexOfLastPeriod = filePaths[i].find_last_of('.');
			std::string fileExtension = filePaths[i].substr(indexOfLastPeriod, 258); // Not that important, but: Windows 7's character limit for filepaths is supposedly 260, so a file extension for a file with a name that's 1 char long can be at most 255, given the usage of 2 characters for the name and the period before the extension, and 3 for the drive letter, colon, and the initial slash.

			// Check if the file format (extension) is one that is already compressed/should not be compressed.
			fileTableEntry.compressionSetting = 1;
			if (fileExtension.compare("png") == 0)
			{
				fileTableEntry.compressionSetting = 0;
				fileTableEntry.fileFormat = PNG;
			}
			else if (fileExtension.compare("jpg") == 0)
			{
				fileTableEntry.compressionSetting = 0;
				fileTableEntry.fileFormat = JPG;
			}
			else if (fileExtension.compare("flac") == 0)
			{
				fileTableEntry.compressionSetting = 0;
				fileTableEntry.fileFormat = FLAC;
			}
			else if (fileExtension.compare("ogg") == 0)
			{
				fileTableEntry.compressionSetting = 0;
				fileTableEntry.fileFormat = OGG;
			}
			else if (fileExtension.compare("mp3") == 0)
			{
				fileTableEntry.compressionSetting = 0;
				fileTableEntry.fileFormat = MP3;
			}
			else
			{
				fileTableEntry.compressionSetting = 1;
				fileTableEntry.fileFormat = RAW;
			}

			// Append the file contents to the contents of the Package.
			if (fileTableEntry.compressionSetting != 0)
			{
				// Compress the file content with the chosen/appropriate compressor, and append them to the PAK file.
				rewind(fileToAdd);
				if (fileTableEntry.compressionSetting == 1) // >> 1) & 1 == 1) //fileTableEntry.compressionSetting == 1)
					nBytesAdded = compressionHandler_lz4->compress_fileToFile(fileToAdd, package.m_fileHandle);
				else if (fileTableEntry.compressionSetting == 2) //>> 2) & 1 == 1) //fileTableEntry.compressionSetting == 2)
					nBytesAdded = compressionHandler_zlib->compress_fileToFile(fileToAdd, package.m_fileHandle);
			}
			else
			{
				// Without compressing the file content, append them to the PAK file.
				//fwrite(buffer, 1, fileSize_unCompressed, package.m_fileHandle);
			}

			// Delete the buffer after the hash function is done with it and after, possibly, its contents have been written to the PAK (in the case of the contents not being compressed).
			//delete[] buffer;

			int startIndex = filePaths[i].find_last_of('/') + 1;
			strncpy_s(fileTableEntry.fileName, filePaths[i].c_str() + startIndex, filePaths[i].size() - startIndex);
			fileTableEntry.fileName[sizeof(fileTableEntry.fileName) - 1] = 0;
			fileTableEntry.fileOffset = package.m_nextFileOffset; // Add the offset to the start of the added file.
			fileTableEntry.fileSize_compressed = nBytesAdded;
			fileTableEntry.fileSize_uncompressed = fileSize_unCompressed;

			// Increment the file offset so the next eventual file to be added is added after the current one.
			package.m_nextFileOffset = fileTableEntry.fileOffset + nBytesAdded;
		}
		else
		{
			// The data is a duplicate of already packed data, so store the index of the original occurrence of the data.
			ZeroMemory(&fileTableEntry, sizeof(PackageFileTableEntry));
			fileTableEntry.redirectIndex = duplicateDataInfos[i].redirectIndex;
			
			package.m_nextFileOffset = fileTableEntry.fileOffset; // No bytes were added since the data is to be unique in the PAK file, so keep the current offset.

			// The rest of the fileTableEntry doesn't need to be set, since the redirectIndex redirects to a fully set entry.
		}

		fileTableEntries.push_back(fileTableEntry);

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

// Set _loadStartIndex and _loadEndIndex.
std::vector<LoadedFileInfo> PackageReaderWriter::loadPackageData(std::string packageFileName, void *dest, int _loadStartIndex, int _loadEndIndex, bool _loadEntirePackage)
{
	std::vector<LoadedFileInfo> loadedFileInfos;

	void *loadedData = NULL;
	unsigned int loadStartIndex, loadEndIndex;
	
	// Get the package file table entries.
	std::vector<PackageFileTableEntry> fileTableEntries;
	bool entryFound = getFileTableOfPackage(packageFileName, fileTableEntries);

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

	FILE *packageFileHandle;
	fopen_s(&packageFileHandle, packageFileName.c_str(), "rb");

	int nBytesLoaded = 0;
	unsigned int currentIndex = loadStartIndex;
	unsigned int backupOfCurrentIndex;
	do 
	{
		backupOfCurrentIndex = currentIndex;

		// If the redirectIndex isn't zero then data for this entry occurs only once in the package, at another index.
		if (fileTableEntries[currentIndex].redirectIndex != -1)
		{
			// Change the currentIndex to the index that contains the unique data.
			currentIndex = fileTableEntries[currentIndex].redirectIndex;
		}

		LoadedFileInfo loadedFileInfo;

		// Get the file extension of the file to load.
		loadedFileInfo.fileFormat = fileTableEntries[currentIndex].fileFormat;

		unsigned int startOffset = fileTableEntries[currentIndex].fileOffset;

		fseek(packageFileHandle, startOffset, SEEK_SET);
		
		int compressionSetting = fileTableEntries[currentIndex].compressionSetting;
		if (compressionSetting != 0)
		{
			if (compressionSetting == 1)
				nBytesLoaded += compressionHandler_lz4->deCompress_fileToMemory(packageFileHandle, startOffset, (char*)dest + nBytesLoaded, fileTableEntries[currentIndex].fileSize_uncompressed, fileTableEntries[currentIndex].fileSize_compressed);
			else if (compressionSetting == 2)
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

		// Just in case a re-direct index was used, restore currentIndex with the backup of it, before it was set to the redirectIndex.
		currentIndex = backupOfCurrentIndex;

		++currentIndex;
	} while (currentIndex < loadEndIndex);

	fclose(packageFileHandle);

	return loadedFileInfos;
}

std::vector<LoadedFileInfo> PackageReaderWriter::loadPackageData2(std::string packageFileName, void *dest, int _loadStartIndex, int _loadEndIndex, bool _loadEntirePackage)
{
	std::vector<LoadedFileInfo> loadedFileInfos;

	void *loadedData = NULL;
	unsigned int loadStartIndex, loadEndIndex;

	// Get the package file table entries.
	std::vector<PackageFileTableEntry> fileTableEntries;
	bool entryFound = getFileTableOfPackage(packageFileName, fileTableEntries);

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

	FILE *packageFileHandle;
	fopen_s(&packageFileHandle, packageFileName.c_str(), "rb");

	// Calculate memory needed to store uncompressed file data.
	unsigned long nBytesUnCompressedData = 0;
	unsigned long nBytesCompressedData = 0;
	for (unsigned int i = loadStartIndex; i < loadEndIndex; ++i)
	{
		if (fileTableEntries[i].redirectIndex != -1)
		{
			// Change the currentIndex to the index that contains the unique data.
			nBytesUnCompressedData += fileTableEntries[fileTableEntries[i].redirectIndex].fileSize_uncompressed;
			nBytesCompressedData += fileTableEntries[fileTableEntries[i].redirectIndex].fileSize_compressed;
		}
		else
		{
			nBytesUnCompressedData += fileTableEntries[i].fileSize_uncompressed;
			nBytesCompressedData += fileTableEntries[i].fileSize_compressed;
		}
	}


	//packageFileHandle

	int nBytesLoaded = 0;
	unsigned int currentIndex = loadStartIndex;
	unsigned int backupOfCurrentIndex;
	do
	{
		backupOfCurrentIndex = currentIndex;

		// If the redirectIndex isn't zero then data for this entry occurs only once in the package, at another index.
		if (fileTableEntries[currentIndex].redirectIndex != -1)
		{
			// Change the currentIndex to the index that contains the unique data.
			currentIndex = fileTableEntries[currentIndex].redirectIndex;
		}

		LoadedFileInfo loadedFileInfo;

		// Get the file extension of the file to load.
		loadedFileInfo.fileFormat = fileTableEntries[currentIndex].fileFormat;

		unsigned int startOffset = fileTableEntries[currentIndex].fileOffset;

		fseek(packageFileHandle, startOffset, SEEK_SET);

		int compressionSetting = fileTableEntries[currentIndex].compressionSetting;
		if (compressionSetting != 0)
		{
			if (compressionSetting == 1)
				nBytesLoaded += compressionHandler_lz4->deCompress_fileToMemory(packageFileHandle, startOffset, (char*)dest + nBytesLoaded, fileTableEntries[currentIndex].fileSize_uncompressed, fileTableEntries[currentIndex].fileSize_compressed);
			else if (compressionSetting == 2)
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

		// Just in case a re-direct index was used, restore currentIndex with the backup of it, before it was set to the redirectIndex.
		currentIndex = backupOfCurrentIndex;

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