#include "StringConverter.h"
#ifdef _WIN32

#include <codecvt>
#include <locale>
#include <Windows.h>

std::wstring StringConverter::StringToWString(const std::string& _s)
{
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	std::wstring ty = converter.from_bytes(_s);
	return converter.from_bytes(_s);
}

std::string StringConverter::WStringToString(const std::wstring& _s)
{
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	std::string ty = converter.to_bytes(_s);
	return converter.to_bytes(_s);
}

#endif