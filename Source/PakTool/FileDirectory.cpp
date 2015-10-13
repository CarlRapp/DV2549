#include "FileDirectory.h"

#include <Windows.h>
#include <sstream>

#include "StringConverter.h"

void FileDirectory::GetAllFilesInDirectory(
	std::vector<std::string>& _out, 
	const std::string& _directory, 
	bool _subdirectories,
	const char* _fileExtension)
{
	HANDLE dir;
	WIN32_FIND_DATA file_data;
	//char* buf = SDL_GetBasePath();
	//std::string currentDirectory(buf);
	//std::wstring directory = StringConverter::StringToWString(currentDirectory + "/" + _directory + "*");
	std::wstring directory = StringConverter::StringToWString(_directory + "/*");
	if ((dir = FindFirstFile(directory.c_str(), &file_data)) == INVALID_HANDLE_VALUE)
		return; /* No files found */

	while (FindNextFile(dir, &file_data)) 
	{
		const std::wstring file_name = file_data.cFileName;
		const std::wstring full_file_name = StringConverter::StringToWString(_directory + StringConverter::WStringToString(file_name));
		const bool is_directory = (file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;

		if (file_name[0] == '.')
			continue;

		size_t extensionStart = file_name.find_last_of('.');
		std::wstring extension = StringConverter::StringToWString(_fileExtension);
		extension.pop_back();
		
		if (!is_directory && _fileExtension[0] != '*' && file_name.substr(extensionStart + 1, file_name.length() - extensionStart - 1) != extension)
			continue;

		if (is_directory)
		{
			if (_subdirectories)
			{
				std::string directory = _directory;
				directory.append(StringConverter::WStringToString(file_name));
				directory.append("/");
				GetAllFilesInDirectory(_out, directory, _subdirectories, _fileExtension);
			}

			continue;
		}

		_out.push_back(StringConverter::WStringToString(full_file_name));
	} 

	FindClose(dir);
}