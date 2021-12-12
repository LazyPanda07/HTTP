#include "HTTPParser.h"

#include <algorithm>
#include <iterator>

#pragma warning(disable: 26800)

using namespace std;

constexpr int responseCodeSize = 3;

namespace web
{
	size_t HTTPParser::insensitiveStringHash::operator () (const string& value) const
	{
		string tem;

		tem.reserve(value.size());

		for_each(value.begin(), value.end(), [&tem](char c) { tem += tolower(c); });

		return hash<string>()(tem);
	}

	bool HTTPParser::insensitiveStringEqual::operator () (const string& left, const string& right) const
	{
		return equal
		(
			left.begin(), left.end(),
			right.begin(), right.end(),
			[](char first, char second) { return tolower(first) == tolower(second); }
		);
	}

	const string HTTPParser::contentLengthHeader = "Content-Length";
	const string HTTPParser::contentTypeHeader = "Content-Type";
	const string HTTPParser::utf8Encoded = "charset=utf-8";

	void HTTPParser::parseKeyValueParameter(string_view rawParameters)
	{
		size_t nextKeyValuePair = 0;
		string key;
		string value;
		bool equal = false;

		if (rawParameters.find("HTTP") != string_view::npos)
		{
			rawParameters.remove_suffix(httpVersion.size());
		}

		for (; nextKeyValuePair < rawParameters.size(); nextKeyValuePair++)
		{
			if (rawParameters[nextKeyValuePair] == '&')
			{
				equal = false;

				keyValueParameters[move(key)] = move(value);

				continue;
			}

			if (!equal)
			{
				equal = rawParameters[nextKeyValuePair] == '=';

				if (equal)
				{
					continue;
				}
			}

			equal ? value += rawParameters[nextKeyValuePair] : key += rawParameters[nextKeyValuePair];
		}

		keyValueParameters[move(key)] = move(value);
	}

	void HTTPParser::parse(string_view&& HTTPMessage)
	{
		size_t prevString = 0;
		size_t nextString = HTTPMessage.find('\r');
		string_view firstString(HTTPMessage.data(), nextString);

		switch (firstString[0])
		{
		case 'H':
			if (firstString[1] == 'E')
			{
				method = "HEAD";
			}

			break;

		case 'P':
			if (firstString[1] == 'U')
			{
				method = "PUT";
			}
			else if (firstString[1] == 'O')
			{
				method = "POST";
			}
			else
			{
				method = "PATCH";
			}

			break;

		case 'O':
			method = "OPTIONS";

			break;

		case 'D':
			method = "DELETE";

			break;

		case 'C':
			method = "CONNECT";

			break;

		case 'G':
			method = "GET";

			break;
		}

		if (method.empty())
		{
			size_t startHTTP = firstString.find("HTTP");
			size_t endHTTP = firstString.find(' ', startHTTP);

			httpVersion = string(firstString.begin() + startHTTP, firstString.begin() + endHTTP);

			for (size_t i = 0; i < firstString.size() - responseCodeSize; i++)
			{
				string_view tem(firstString.data() + i, responseCodeSize);
				if (atoi(tem.data()) >= 100 && all_of(begin(tem), end(tem), [](char c) { return isdigit(c); }))
				{
					string message;

					for (size_t j = i + responseCodeSize + 1; j < firstString.size(); j++)
					{
						message += firstString[j];
					}

					response = { static_cast<ResponseCodes>(stoi(string(tem))), message };

					break;
				}
			}
		}

		if (method.size())
		{
			size_t startParameters = firstString.find('/');
			size_t endParameters = firstString.find(' ', startParameters);
			size_t queryStart = firstString.find('?');

			parameters = string(firstString.begin() + startParameters, firstString.begin() + endParameters);

			httpVersion = string(firstString.begin() + firstString.find("HTTP"), firstString.end());

			if (queryStart != string::npos)
			{
				this->parseKeyValueParameter(firstString.substr(queryStart + 1));
			}
		}

		while (true)
		{
			prevString = nextString + crlf.size();
			nextString = HTTPMessage.find('\r', prevString);

			string_view next(HTTPMessage.data() + prevString, nextString - prevString);

			if (prevString == nextString || nextString == string::npos)
			{
				break;
			}

			string header(next.begin(), next.begin() + next.find(':'));
			string value(next.begin() + next.find(": ") + 2, next.end());

			headers[move(header)] = move(value);
		}

		if (headers.find(contentLengthHeader) != headers.end())
		{
			if (HTTPMessage.find(utf8Encoded) != string::npos)
			{
				body = json::utility::toUTF8JSON(string(HTTPMessage.begin() + HTTPMessage.find(crlfcrlf) + crlfcrlf.size(), HTTPMessage.end()), CP_UTF8);
			}
			else
			{
				body = string(HTTPMessage.begin() + HTTPMessage.find(crlfcrlf) + crlfcrlf.size(), HTTPMessage.end());
			}
			
			auto it = headers.find(contentTypeHeader);

			if (it != headers.end())
			{
				if (it->second == urlEncoded)
				{
					this->parseKeyValueParameter(body);
				}
				else if (it->second == jsonEncoded)
				{
					jsonParser.setJSONData(body);
				}
			}
		}
	}

	HTTPParser::HTTPParser(const string& HTTPMessage)
	{
		this->parse(string_view(HTTPMessage.data(), HTTPMessage.size()));
	}

	HTTPParser::HTTPParser(const vector<char>& HTTPMessage)
	{
		this->parse(string_view(HTTPMessage.data(), HTTPMessage.size()));
	}

	const string& HTTPParser::getMethod() const
	{
		return method;
	}

	const string& HTTPParser::getHTTPVersion() const
	{
		return httpVersion;
	}

	const string& HTTPParser::getParameters() const
	{
		return parameters;
	}

	const unordered_map<string, string>& HTTPParser::getKeyValueParameters() const
	{
		return keyValueParameters;
	}

	const pair<ResponseCodes, string>& HTTPParser::getFullResponse() const
	{
		return response;
	}

	ResponseCodes HTTPParser::getResponseCode() const
	{
		return response.first;
	}

	string HTTPParser::getResponseMessage() const
	{
		return response.second;
	}

	const unordered_map<string, string, HTTPParser::insensitiveStringHash, HTTPParser::insensitiveStringEqual>& HTTPParser::getHeaders() const
	{
		return headers;
	}

	const string& HTTPParser::getBody() const
	{
		return body;
	}

	const json::JSONParser& HTTPParser::getJSON() const
	{
		return jsonParser;
	}
}
