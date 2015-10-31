#include <iostream>

#include "FileDirectory.h"
#include "PakLib/PackageReaderWriter.h"
#include "PakLib/Compression/CompressionHandler_zlib.h"
#include "PakLib/Compression/CompressionHandler_lz4.h"
#include "PakLib/Hash/MD5.h"


int main(int argc, char* argv[]) 
{
	/* Fetch output file from user */
	//int choice;
	//std::cout << "PakTool: Choice: ";
	//std::cin >> choice;

	//if (choice == 1)
	//{
	//	int nInputFolders;
	//	std::cout << "PakTool: Number of input folders to input: ";
	//	std::cin >> nInputFolders;
	//	std::cin.ignore();
	//	std::cout << nInputFolders;
	//			
	//	std::vector<std::string> inputFolders;
	//	for (unsigned int i = 0; i < nInputFolders; ++i)
	//	{
	//		/* Fetch input directory from user */
	//		std::string inputDir;
	//		std::cout << "PakTool: Input Directory #" + std::to_string(i) + ": ";
	//		getline(std::cin, inputDir);
	//		inputFolders.push_back(inputDir);
	//	}

	//	/* Fetch output file from user */
	//	std::string outputFile;
	//	std::cout << "PakTool: Output File: ";
	//	getline(std::cin, outputFile);
	//	printf("PakTool: Output File: %s\n", outputFile.c_str());

	//	std::vector<std::vector<std::string>> filePathLists;
	//	for (unsigned int i = 0; i < nInputFolders; ++i)
	//	{
	//		/* Get all files in input directory */
	//		std::vector<std::string> filePaths;
	//		FileDirectory::GetAllFilesInDirectory(filePaths, inputFolders[i] + "/", false);
	//		printf("PakTool: Found %i files in input directory\n", filePaths.size());
	//		filePathLists.push_back(filePaths);
	//	}

	//	printf("PakTool: Packaging has begun. This can take a while...\n");
	//	
	//	PackageReaderWriter packageReaderWriter = PackageReaderWriter();
	//	packageReaderWriter.createPackageByInterleavingInputFolderFiles(outputFile, filePathLists);

	//	printf("PakTool: Work finished\n");
	//	printf("PakTool: Shutting down\n");
	//	system("pause");
	//}
	//else
	//{
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

		printf("PakTool: Packaging has begun. This can take a while...\n");

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
		
		if(choice == 'y' || choice == 'Y')
			packageReaderWriter.createPackageFromUniqueFiles(outputFile, filePaths);
		else
			packageReaderWriter.createPackageFromFiles(outputFile, filePaths);

		printf("PakTool: Work finished\n");
		printf("PakTool: Shutting down\n");
		system("pause");
	//}

	return 0;
}