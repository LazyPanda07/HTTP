#include "HTTPParser.h"

#include <iterator>
#include <format>
#include <algorithm>
#include <unordered_set>
#include <functional>

#include "HTTPParseException.h"

#ifndef __LINUX__
#pragma warning(disable: 26800)
#endif

using namespace std;

static const unordered_set<string> methods =
{
	"HEAD",
	"PUT",
	"POST",
	"PATCH",
	"OPTIONS",
	"DELETE",
	"CONNECT",
	"GET",
	"TRACE"
};

namespace web
{
	const unordered_map<string_view, function<void(HTTPParser&, string_view)>> HTTPParser::contentTypeParsers =
	{
		{ HTTPParser::urlEncoded, [](HTTPParser& parser, string_view data) { parser.parseKeyValueParameter(data); }},
		{ HTTPParser::jsonEncoded, [](HTTPParser& parser, string_view data) { parser.jsonParser.setJSONData(data); }},
		{ HTTPParser::multipartEncoded, [](HTTPParser& parser, string_view data) { parser.parseMultipart(data); }},
	};

	HTTPParser::readOnlyBuffer::readOnlyBuffer(string_view view)
	{
		char* data = const_cast<char*>(view.data());

		setg(data, data, data + view.size());
	}

	string HTTPParser::mergeChunks() const
	{
		string result;

		result.reserve(chunksSize);

		ranges::for_each(chunks, [&result](const string& value) { result += value; });

		return result;
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

		string decodedParameters = web::decodeUrl(rawParameters);

		for (size_t nextKeyValuePair = 0; nextKeyValuePair < decodedParameters.size(); nextKeyValuePair++)
		{
			if (decodedParameters[nextKeyValuePair] == '&')
			{
				equal = false;

				keyValueParameters[move(key)] = move(value);

				continue;
			}

			if (!equal)
			{
				equal = decodedParameters[nextKeyValuePair] == '=';

				if (equal)
				{
					continue;
				}
			}

			equal ? value += decodedParameters[nextKeyValuePair] : key += decodedParameters[nextKeyValuePair];
		}

		keyValueParameters[move(key)] = move(value);
	}
	
	void HTTPParser::parseMultipart(string_view data)
	{
		constexpr string_view boundaryText = "boundary=";

		const string& contentType = headers["Content-Type"];
		size_t index = contentType.find("boundaryText");
		string boundary = format("--{}", string_view(contentType.begin() + index + boundaryText.size(), contentType.end()));

		index = 0;
		boyer_moore_horspool_searcher searcher(boundary.begin(), boundary.end());

		string_view::const_iterator it = search(data.begin(), data.end(), searcher);

		do
		{
			string_view::const_iterator next = search(it + 1, data.end(), searcher);

			multiparts.emplace_back(string_view(it + boundary.size(), next));

			it = search(next + 1, data.end(), searcher);
		} while (it != data.end());
	}

	void HTTPParser::parseContentType()
	{
		if (auto it = headers.find(contentTypeHeader); it != headers.end())
		{
			for (const auto& [encodeType, parser] : contentTypeParsers)
			{
				if (it->second.find(encodeType) != string::npos)
				{
					parser(*this, chunksSize ? this->mergeChunks() : body);

					break;
				}
			}
		}
	}

	void HTTPParser::parseChunkEncoded(string_view HTTPMessage, bool isUTF8)
	{
		size_t chunksStart = HTTPMessage.find(crlfcrlf) + crlfcrlf.size();
		size_t chunksEnd = HTTPMessage.rfind(crlfcrlf) + crlfcrlf.size();
		readOnlyBuffer buffer(string_view(HTTPMessage.data() + chunksStart, chunksEnd - chunksStart));
		istringstream chunksData;

		static_cast<ios&>(chunksData).rdbuf(&buffer);

		chunksSize = 0;

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

			string& chunk = isUTF8 ?
				chunks.emplace_back(json::utility::toUTF8JSON(value, CP_UTF8)) :
				chunks.emplace_back(move(value));

			chunksSize += chunk.size();
		}
	}

	HTTPParser::HTTPParser(const string& HTTPMessage)
	{
		this->parse(HTTPMessage);
	}

	HTTPParser::HTTPParser(const vector<char>& HTTPMessage)
	{
		this->parse(string_view(HTTPMessage.data(), HTTPMessage.size()));
	}

	void HTTPParser::parse(string_view HTTPMessage)
	{
		if (HTTPMessage.empty())
		{
			parsed = false;

			return;
		}

		parsed = true;

		size_t prevString = 0;
		size_t nextString = HTTPMessage.find('\r');
		string_view firstString(HTTPMessage.data(), nextString);
		
		if (string_view temp = firstString.substr(0, firstString.find(' ')); temp.find("HTTP") == string_view::npos)
		{
			method = temp;

			if (methods.find(method) == methods.end())
			{
				throw exceptions::HTTPParseException(format("Wrong method: {}", method));
			}
		}

		chunksSize = 0;

		rawData = HTTPMessage;

		if (method.empty())
		{
			readOnlyBuffer buffer(firstString);
			istringstream data;
			string responseCode;

			static_cast<ios&>(data).rdbuf(&buffer);

			data >> httpVersion >> responseCode >> response.second;

			response.first = stoi(responseCode);
		}
		else if (method != "CONNECT")
		{
			size_t startParameters = firstString.find('/');

			if (startParameters == string::npos)
			{
				throw exceptions::HTTPParseException("Can't find /");
			}

			startParameters++;

			size_t endParameters = firstString.rfind(' ');
			size_t queryStart = firstString.find('?');

			parameters = web::decodeUrl(string_view(firstString.begin() + startParameters, firstString.begin() + endParameters));

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

			size_t colonIndex = next.find(':');
			string header(next.begin(), next.begin() + colonIndex);
			string value(next.begin() + colonIndex + 2, next.end());

			headers.try_emplace(move(header), move(value));
		}

		bool isUTF8 = HTTPMessage.find(utf8Encoded) != string::npos;

		if (auto it = headers.find(transferEncodingHeader); it != headers.end())
		{
			static const unordered_map<string, void (HTTPParser::*)(string_view HTTPMessage, bool isUTF8)> transferTypeParsers =
			{
				{ chunkEncoded, &HTTPParser::parseChunkEncoded }
			};

			if (!transferTypeParsers.contains(it->second))
			{
				throw exceptions::HTTPParseException("Not supported transfer encoding: " + it->second);
			}

			invoke(transferTypeParsers.at(it->second), *this, HTTPMessage, isUTF8);
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
		}

		this->parseContentType();
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

	const pair<int, string>& HTTPParser::getFullResponse() const
	{
		return response;
	}

	int HTTPParser::getResponseCode() const
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

	const string& HTTPParser::getRawData() const
	{
		return rawData;
	}

	const vector<Multipart>& HTTPParser::getMultiparts() const
	{
		return multiparts;
	}

	HTTPParser::operator bool() const
	{
		return parsed;
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

		if (httpMessage.size())
		{
			parser.parse(httpMessage);
		}

		return inputStream;
	}
}
