#pragma once

#include <string>

class StringConversion {
public:
	static std::string  toStdString (const std::wstring & s);
	static std::wstring toStdWString(const std::string & s);
};
