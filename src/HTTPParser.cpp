#include "HTTPParser.h"

#include <algorithm>
#include <iterator>

#pragma warning(disable: 26800)

using namespace std;

constexpr int responseCodeSize = 3;

namespace web
{
	HTTPParser::readOnlyBuffer::readOnlyBuffer(string_view view)
	{
		char* data = const_cast<char*>(view.data());

		setg(data, data, data + view.size());
	}

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

	void HTTPParser::parse(string_view HTTPMessage)
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
			readOnlyBuffer buffer(firstString);
			istringstream data;
			string responseCode;

			data.set_rdbuf(&buffer);

			data >> httpVersion >> responseCode >> response.second;

			response.first = static_cast<responseCodes>(stoi(responseCode));
		}

		if (method.size())
		{
			size_t startParameters = firstString.find('/') + 1;
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

		bool isUTF8 = HTTPMessage.find(utf8Encoded) != string::npos;

		if (headers.find(contentLengthHeader) != headers.end())
		{
			if (isUTF8)
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
				else if (it->second.find(jsonEncoded) != string::npos)
				{
					jsonParser.setJSONData(body);
				}
			}
		}
		else if (auto transferEncoding = headers.find(transferEncodingHeader); transferEncoding != headers.end())
		{
			if (transferEncoding->second == chunkEncoded)
			{
				size_t chunksStart = HTTPMessage.find(crlfcrlf) + crlfcrlf.size();
				size_t chunksEnd = HTTPMessage.rfind(crlfcrlf) + crlfcrlf.size();
				readOnlyBuffer buffer(string_view(HTTPMessage.data() + chunksStart, chunksEnd - chunksStart));
				istringstream chunksData;

				chunksData.set_rdbuf(&buffer);

				while (true)
				{
					string size;
					string value;

					getline(chunksData, size);

					size.pop_back(); // \r symbol from \r\n

					value.resize(stol(size, nullptr, 16));

					if (value.empty())
					{
						return;
					}

					chunksData.read(value.data(), value.size());

					chunksData.ignore(crlf.size());

					if (isUTF8)
					{
						chunks.push_back(json::utility::toUTF8JSON(value, CP_UTF8));
					}
					else
					{
						chunks.push_back(move(value));
					}
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

	HTTPParser::HTTPParser(const HTTPParser& other)
	{
		(*this) = other;
	}

	HTTPParser::HTTPParser(HTTPParser&& other) noexcept
	{
		(*this) = move(other);
	}

	HTTPParser& HTTPParser::operator = (const HTTPParser& other)
	{
		headers = other.headers;
		keyValueParameters = other.keyValueParameters;
		response = other.response;
		method = other.method;
		httpVersion = other.httpVersion;
		parameters = other.parameters;
		body = other.body;
		chunks = other.chunks;
		jsonParser = other.jsonParser;

		return *this;
	}

	HTTPParser& HTTPParser::operator = (HTTPParser&& other) noexcept
	{
		headers = move(other.headers);
		keyValueParameters = move(other.keyValueParameters);
		response = move(other.response);
		method = move(other.method);
		httpVersion = move(other.httpVersion);
		parameters = move(other.parameters);
		body = move(other.body);
		chunks = move(other.chunks);
		jsonParser = move(other.jsonParser);

		return *this;
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

	const pair<responseCodes, string>& HTTPParser::getFullResponse() const
	{
		return response;
	}

	responseCodes HTTPParser::getResponseCode() const
	{
		return response.first;
	}

	const string& HTTPParser::getResponseMessage() const
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

	const vector<string>& HTTPParser::getChunks() const
	{
		return chunks;
	}

	const json::JSONParser& HTTPParser::getJSON() const
	{
		return jsonParser;
	}

	HTTP_API ostream& operator << (ostream& outputStream, const HTTPParser& parser)
	{
		string result;

		if (parser.method.size())
		{
			result += parser.method + " /" + parser.parameters + ' ' + parser.httpVersion;
		}
		else
		{
			const auto& [code, message] = parser.getFullResponse();

			result += parser.httpVersion + ' ' + to_string(static_cast<int>(code)) + ' ' + message;
		}

		result += HTTPParser::crlf;

		for (const auto& [header, value] : parser.headers)
		{
			result.
				append(header + ": " + value).
				append(HTTPParser::crlf);
		}

		if (parser.body.size())
		{
			result.
				append(HTTPParser::crlf).
				append(parser.body);
		}
		else if (parser.chunks.size())
		{
			result += HTTPParser::crlf;

			for (const auto& chunk : parser.chunks)
			{
				result.
					append((ostringstream() << hex << chunk.size() << HTTPParser::crlf).str()).
					append(chunk).
					append(HTTPParser::crlf);
			}

			result.
				append("0").
				append(HTTPParser::crlfcrlf);
		}

		return outputStream << result;
	}
}
