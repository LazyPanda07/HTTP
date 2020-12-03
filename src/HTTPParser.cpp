#include "HTTPParser.h"

#include <algorithm>
#include <iterator>

using namespace std;

constexpr int responseCodeSize = 3;

namespace web
{
	void HTTPParser::parsing(string_view&& HTTPMessage)
	{
		size_t prevString = 0;
		size_t nextString = HTTPMessage.find('\r');
		const string_view firstString(HTTPMessage.data(), nextString);

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
				if (atoi(tem.data()) >= 100 && all_of(begin(tem), end(tem), [](auto ch) {return isdigit(ch); }))
				{
					string message;

					for (size_t j = i + responseCodeSize + 1; j < firstString.size(); j++)
					{
						message += firstString[j];
					}

					response = make_pair(stoi(string(tem)), message);
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
				string_view queryValues = firstString.substr(queryStart + 1);
				size_t nextKeyValuePair = 0;
				string key;
				string value;
				bool equal = false;

				queryValues.remove_suffix(httpVersion.size() + 1);

				for (; nextKeyValuePair != queryValues.size(); nextKeyValuePair++)
				{
					if (queryValues[nextKeyValuePair] == '&' || nextKeyValuePair == queryValues.size())
					{
						equal = false;

						keyValueParameters.insert(make_pair(move(key), move(value)));

						continue;
					}

					if (!equal)
					{
						equal = queryValues[nextKeyValuePair] == '=';

						if (equal)
						{
							continue;
						}
					}

					equal ? value += queryValues[nextKeyValuePair] : key += queryValues[nextKeyValuePair];
				}
			}
		}

		while (true)
		{
			prevString = nextString + 2;
			nextString = HTTPMessage.find('\r', prevString);

			const string_view next(HTTPMessage.data() + prevString, nextString - prevString);

			if (prevString == nextString || nextString == string::npos)
			{
				break;
			}

			string header(next.begin(), next.begin() + next.find(':'));
			string value(next.begin() + next.find(": ") + 2, next.end());

			headers[header] = value;
		}

		if (headers.find("Content-Length") != headers.end())
		{
			body = string(HTTPMessage.begin() + HTTPMessage.find("\r\n\r\n") + 4, HTTPMessage.end());
		}
	}

	HTTPParser::HTTPParser(const string& HTTPMessage)
	{
		this->parsing(string_view(HTTPMessage.data(), HTTPMessage.size()));
	}

	HTTPParser::HTTPParser(const vector<char>& HTTPMessage)
	{
		this->parsing(string_view(HTTPMessage.data(), HTTPMessage.size()));
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

	const pair<short, string>& HTTPParser::getResponse() const
	{
		return response;
	}

	const unordered_map<string, string>& HTTPParser::getHeaders() const
	{
		return headers;
	}

	const string& HTTPParser::getBody() const
	{
		return body;
	}
}
