#include <iostream>

#include "FileDirectory.h"
#include "PakLib/PackageReaderWriter.h"
#include "PakLib/Compression/CompressionHandler_zlib.h"
#include "PakLib/Compression/CompressionHandler_lz4.h"
#include "PakLib/Hash/MD5.h"


int main(int argc, char* argv[]) 
{
	/* Fetch input directory from user */
	std::string inputDir;
	std::cout << "PakTool: Input Directory: ";
	getline(std::cin, inputDir);

	/* Fetch output file from user */
	std::string outputFile;
	std::cout << "PakTool: Output File: ";
	getline(std::cin, outputFile);

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
	
	PackageReaderWriter packageReaderWriter = PackageReaderWriter();

	/* Fetch output file from user */
	char choice;
	std::cout << "PakTool: Remove duplicate files? (y/n): ";
	std::cin >> choice;
	std::cin.ignore();

	while (choice != 'y' && choice != 'n' && choice != 'Y' && choice != 'N')
	{
		std::cout << "PakTool: Invalid choice - must input y/Y (yes) or n/N (no). " << std::endl;
		std::cout << "PakTool: Remove duplicate files? (y/n): ";
		std::cin >> choice;
		std::cin.ignore();
	}
	
	if (choice == 'y' || choice == 'Y')
	{
		printf("PakTool: Packaging has begun. This can take a while...\n");
		packageReaderWriter.createPackageFromUniqueFiles(outputFile, filePaths);
	}
	else
	{
		printf("PakTool: Packaging has begun. This can take a while...\n");
		packageReaderWriter.createPackageFromFiles(outputFile, filePaths);
	}

	printf("PakTool: Work finished\n");
	printf("PakTool: Shutting down\n");
	system("pause");

	return 0;
}