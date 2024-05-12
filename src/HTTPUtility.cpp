#include "HTTPUtility.h"

#include <algorithm>

using namespace std;

namespace web
{
	string getHTTPLibraryVersion()
	{
		string version = "1.7.0";

		return version;
	}

	size_t insensitiveStringHash::operator () (const string& value) const
	{
		string tem;

		tem.reserve(value.size());

		for_each(value.begin(), value.end(), [&tem](char c) { tem += tolower(c); });

		return hash<string>()(tem);
	}

	bool insensitiveStringEqual::operator () (const string& left, const string& right) const
	{
		return equal
		(
			left.begin(), left.end(),
			right.begin(), right.end(),
			[](char first, char second) { return tolower(first) == tolower(second); }
		);
	}
}
