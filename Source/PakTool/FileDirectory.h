#ifndef FILEDIRECTORY_H
#define FILEDIRECTORY_H

#include <SDL/SDL.h>
#include <fstream>
#include <vector>

namespace FileDirectory
{
	/// <summary>
	/// Finds all files in directory.
	/// </summary>
	/// <param name="_out">Will be filled with paths to all files.</param>
	/// <param name="_directory">Directory to be searched. Don't forget the forward slash ('/') at the end of the string.</param>
	/// <param name="_subDirectories">Should subdirectories be searched as well?</param>
	/// <param name="_fileExtension">Optional. Will only return files that matches the extension.</param>
	DECLSPEC void GetAllFilesInDirectory(
		std::vector<std::string>& _out, 
		const std::string& _directory,  
		bool _subdirectories,
		const char* _fileExtension = "*");
}

#endif