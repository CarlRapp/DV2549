#include <iostream>

#include "FileDirectory.h"
#include "PakLib/PackageReaderWriter.h"
#include "PakLib/Compression/CompressionHandler_zlib.h"
#include "PakLib/Hash/MD5.h"

int main(int argc, char* argv[]) 
{
	/* Do nothing if there aren't input/output arguments specified */
	if (argc != 3)
	{
		printf("PakTool: Input Directory and Output File not specified\n");
		printf("PakTool: Shutting down\n");
		return 0;
	}

	std::string inputDir = argv[1];
	std::string outputFile = argv[2];

	printf("PakTool: Input Directory: %s\n", inputDir.c_str());
	printf("PakTool: Output File: %s\n", outputFile.c_str());

	/* Get all files in input directory */
	std::vector<std::string> filePaths;
	FileDirectory::GetAllFilesInDirectory(filePaths, inputDir + "/", false);

	printf("PakTool: Found %i files in input directory\n", filePaths.size());

	for (const std::string filePath : filePaths)
	{
		printf("PakTool: About to package %s\n", filePath.c_str());
	}

	printf("PakTool: Packaging has begun. This can take a while...\n");

	PackageReaderWriter packageReaderWriter = PackageReaderWriter(new Compression::CompressionHandler_zlib());
	packageReaderWriter.createPackageFromFiles(outputFile, filePaths);

	printf("PakTool: Work finished\n");
	printf("PakTool: Shutting down\n");

	return 0;
}