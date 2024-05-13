#include "HTTPParser.h"

#include <iterator>
#include <format>

#ifndef __LINUX__
#pragma warning(disable: 26800)
#endif

using namespace std;

constexpr int responseCodeSize = 3;

namespace web
{
	HTTPParser::readOnlyBuffer::readOnlyBuffer(string_view view)
	{
		char* data = const_cast<char*>(view.data());

		setg(data, data, data + view.size());
	}

	void HTTPParser::parseKeyValueParameter(string_view rawParameters)
	{
		string key;
		string value;
		bool equal = false;

		if (rawParameters.find("HTTP") != string_view::npos)
		{
			rawParameters.remove_suffix(httpVersion.size());
		}

		for (size_t nextKeyValuePair = 0; nextKeyValuePair < rawParameters.size(); nextKeyValuePair++)
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

	HTTPParser::HTTPParser(const string& HTTPMessage)
	{
		this->parse(HTTPMessage);
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

	void HTTPParser::parse(string_view HTTPMessage)
	{
		size_t prevString = 0;
		size_t nextString = HTTPMessage.find('\r');
		string_view firstString(HTTPMessage.data(), nextString);

		rawData = HTTPMessage;

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

		case 'T':
			method = "TRACE";

			break;
		}

		if (method.empty())
		{
			readOnlyBuffer buffer(firstString);
			istringstream data;
			string responseCode;

			static_cast<ios&>(data).rdbuf(&buffer);

			data >> httpVersion >> responseCode >> response.second;

			response.first = static_cast<responseCodes>(stoi(responseCode));
		}
		else if (method != "CONNECT")
		{
			size_t startParameters = firstString.find('/');

			if (startParameters == string::npos)
			{
				throw runtime_error("Can't find /");
			}

			startParameters++;

			size_t endParameters = firstString.find(' ', startParameters);

			if (endParameters == string::npos)
			{
				throw runtime_error("Can't find end of paramters");
			}

			size_t queryStart = firstString.find('?');

			parameters = string(firstString.begin() + startParameters, firstString.begin() + endParameters);

			httpVersion = string(firstString.begin() + firstString.find("HTTP"), firstString.end());

			if (queryStart != string::npos)
			{
				this->parseKeyValueParameter(string_view(firstString.data() + queryStart + 1, firstString.data() + endParameters));
			}
		}
		else
		{
			parameters = string(firstString.begin() + firstString.find(' ') + 1, firstString.begin() + firstString.rfind(' '));

			httpVersion = string(firstString.begin() + firstString.find("HTTP"), firstString.end());
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

		if (auto transferEncoding = headers.find(transferEncodingHeader); transferEncoding != headers.end())
		{
			if (transferEncoding->second == chunkEncoded)
			{
				size_t chunksStart = HTTPMessage.find(crlfcrlf) + crlfcrlf.size();
				size_t chunksEnd = HTTPMessage.rfind(crlfcrlf) + crlfcrlf.size();
				readOnlyBuffer buffer(string_view(HTTPMessage.data() + chunksStart, chunksEnd - chunksStart));
				istringstream chunksData;

				static_cast<ios&>(chunksData).rdbuf(&buffer);

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
		else if (headers.find(contentLengthHeader) != headers.end())
		{
			if (isUTF8)
			{
				body = json::utility::toUTF8JSON(string(HTTPMessage.begin() + HTTPMessage.find(crlfcrlf) + crlfcrlf.size(), HTTPMessage.end()), CP_UTF8);
			}
			else
			{
				body = string(HTTPMessage.begin() + HTTPMessage.find(crlfcrlf) + crlfcrlf.size(), HTTPMessage.end());
			}

			if (auto it = headers.find(contentTypeHeader); it != headers.end())
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
	}

	const string& HTTPParser::getMethod() const
	{
		return method;
	}

	double HTTPParser::getHTTPVersion() const
	{
		return stod(httpVersion.substr(5));
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

	const HeadersMap& HTTPParser::getHeaders() const
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

	const std::string& HTTPParser::getRawData() const
	{
		return rawData;
	}

	ostream& operator << (ostream& outputStream, const HTTPParser& parser)
	{
		string result;

		if (parser.method.size())
		{
			if (parser.method != "CONNECT")
			{
				result += format("{} /{} {}", parser.method, parser.parameters, parser.httpVersion);
			}
			else
			{
				result += format("{} {} {}", parser.method, parser.parameters, parser.httpVersion);
			}
		}
		else
		{
			const auto& [code, message] = parser.getFullResponse();

			result += format("{} {} {}", parser.httpVersion, static_cast<int>(code), message);
		}

		result += HTTPParser::crlf;

		for (const auto& [header, value] : parser.headers)
		{
			result += format("{}: {}{}", header, value, HTTPParser::crlf);
		}

		if (parser.body.size())
		{
			result += format("{}{}", HTTPParser::crlf, parser.body);
		}
		else if (parser.chunks.size())
		{
			result += HTTPParser::crlf;

			for (const auto& chunk : parser.chunks)
			{
				result += format("{}{}{}", (ostringstream() << hex << chunk.size() << HTTPParser::crlf).str(), chunk, HTTPParser::crlf);
			}

			result += format("0{}", HTTPParser::crlfcrlf);
		}

		if (!result.ends_with(HTTPParser::crlfcrlf))
		{
			if (result.ends_with(HTTPParser::crlf))
			{
				result += HTTPParser::crlf;
			}
			else
			{
				result += HTTPParser::crlfcrlf;
			}
		}

		return outputStream << result;
	}

	istream& operator >> (istream& inputStream, HTTPParser& parser)
	{
		istreambuf_iterator<char> it(inputStream);
		string httpMessage(it, {});

		parser.parse(httpMessage);

		return inputStream;
	}
}
