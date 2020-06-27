#include "HTTPParser.h"

#include <algorithm>

using namespace std;

constexpr int responseCodeSize = 3;

namespace web
{
	void HTTPParser::parsing(string_view&& HTTPMessage)
	{
		size_t prevString = 0;
		size_t nextString = HTTPMessage.find('\r');
		const string_view firstString(HTTPMessage.data(), nextString);
		const size_t httpStart = firstString.find("HTTP");

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

		for (size_t i = httpStart; i < firstString.size(); i++)
		{
			if (firstString[i] == ' ')
			{
				break;
			}

			httpVersion += firstString[i];
		}

		if (method.empty())
		{
			for (size_t i = 0; i < firstString.size() - responseCodeSize; i++)
			{
				string_view tem(firstString.data() + i, responseCodeSize);
				if (atoi(tem.data()) >= 100 && all_of(begin(tem), end(tem), [](auto ch) {return isdigit(ch); }))
				{
					string message;

					for (size_t j = i; j < firstString.size(); j++)
					{
						message += firstString[j];
					}

					response = make_pair(tem, message);
					break;
				}
			}
		}

		if (method.size())
		{
			size_t startParameters = firstString.find('/');
			size_t httpStart = firstString.find("HTTP");

			if (firstString[startParameters + 1] != ' ')
			{
				if (firstString[startParameters + 1] == '?')
				{
					startParameters += 2;
				}
				else
				{
					startParameters++;
				}

				string name;
				string value;
				bool equal = false;

				for (size_t i = startParameters; i < httpStart; i++)
				{
					if (!equal)
					{
						equal = firstString[i] == '=';
						if (equal)
						{
							continue;
						}
					}

					else if (firstString[i] == '&')
					{
						equal = false;
						parameters[name] = value;
						name = value = "";
						continue;
					}

					equal ? value += firstString[i] : name += firstString[i];
				}

				parameters[name] = value;
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

			string header;
			string value;
			bool colon = false;

			for (size_t i = 0; i < next.size(); i++)
			{
				if (!colon)
				{
					colon = next[i] == ':';
					if (colon)
					{
						i++;	//skip :
						continue;	//skip space
					}
				}

				colon ? value += next[i] : header += next[i];
			}

			headers[header] = value;
		}

		map<string, string>::const_iterator length = headers.find("Content-Length");

		if (length != end(headers))
		{
			body.reserve(atoi(length->second.data()));

			size_t dataSegment = HTTPMessage.find("\r\n\r\n") + 4;

			for (size_t i = dataSegment; i < HTTPMessage.size(); i++)
			{
				body.push_back(HTTPMessage[i]);
			}
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

	const map<string, string>& HTTPParser::getParameters() const
	{
		return parameters;
	}

	const pair<string, string>& HTTPParser::getResponse() const
	{
		return response;
	}

	const map<string, string>& HTTPParser::getHeaders() const
	{
		return headers;
	}

	const string& HTTPParser::getBody() const
	{
		return body;
	}
}
