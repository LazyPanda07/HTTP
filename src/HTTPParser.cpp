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

static const std::unordered_set<std::string> methods =
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
	const std::unordered_map<std::string_view, std::function<void(HTTPParser&, std::string_view)>> HTTPParser::contentTypeParsers =
	{
		{ HTTPParser::urlEncoded, [](HTTPParser& parser, std::string_view data) { parser.parseQueryParameter(data); }},
		{ HTTPParser::jsonEncoded, [](HTTPParser& parser, std::string_view data) { parser.jsonParser.setJSONData(data); }},
		{ HTTPParser::multipartEncoded, [](HTTPParser& parser, std::string_view data) { parser.parseMultipart(data); }},
	};

	HTTPParser::ReadOnlyBuffer::ReadOnlyBuffer(std::string_view view)
	{
		char* data = const_cast<char*>(view.data());

		setg(data, data, data + view.size());
	}

	std::string HTTPParser::mergeChunks() const
	{
		std::string result;

		result.reserve(chunksSize);

		std::ranges::for_each(chunks, [&result](const std::string& value) { result += value; });

		return result;
	}

	void HTTPParser::parseQueryParameter(std::string_view rawParameters)
	{
		std::string key;
		std::string value;
		bool equal = false;

		if (rawParameters.find("HTTP") != std::string_view::npos)
		{
			rawParameters.remove_suffix(httpVersion.size());
		}

		std::string decodedParameters = web::decodeUrl(rawParameters);

		for (size_t nextKeyValuePair = 0; nextKeyValuePair < decodedParameters.size(); nextKeyValuePair++)
		{
			if (decodedParameters[nextKeyValuePair] == '&')
			{
				equal = false;

				queryParameters.try_emplace(move(key), move(value));

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

		queryParameters.try_emplace(move(key), move(value));
	}
	
	void HTTPParser::parseMultipart(std::string_view data)
	{
		constexpr std::string_view boundaryText = "boundary=";

		const std::string& contentType = headers["Content-Type"];
		size_t index = contentType.find(boundaryText);
		std::string boundary = std::format("--{}", std::string_view(contentType.begin() + index + boundaryText.size(), contentType.end()));
		std::boyer_moore_horspool_searcher searcher(boundary.begin(), boundary.end());
		std::string_view::const_iterator current = search(data.begin(), data.end(), searcher);

		while (true)
		{
			std::string_view::const_iterator next = search(current + 1, data.end(), searcher);

			if (next == data.end())
			{
				break;
			}

			multiparts.emplace_back(std::string_view(current + boundary.size(), next));

			current = search(next, data.end(), searcher);
		}
	}

	void HTTPParser::parseContentType()
	{
		if (auto it = headers.find(contentTypeHeader); it != headers.end())
		{
			for (const auto& [encodeType, parser] : contentTypeParsers)
			{
				if (it->second.find(encodeType) != std::string::npos)
				{
					parser(*this, chunksSize ? this->mergeChunks() : body);

					break;
				}
			}
		}
	}

	void HTTPParser::parseChunkEncoded(std::string_view HTTPMessage, bool isUTF8)
	{
		size_t chunksStart = HTTPMessage.find(crlfcrlf) + crlfcrlf.size();
		size_t chunksEnd = HTTPMessage.rfind(crlfcrlf) + crlfcrlf.size();
		ReadOnlyBuffer buffer(std::string_view(HTTPMessage.data() + chunksStart, chunksEnd - chunksStart));
		std::istringstream chunksData;

		static_cast<std::ios&>(chunksData).rdbuf(&buffer);

		chunksSize = 0;

		while (true)
		{
			std::string size;
			std::string value;

			std::getline(chunksData, size);

			size.pop_back(); // \r symbol from \r\n

			value.resize(stol(size, nullptr, 16));

			if (value.empty())
			{
				return;
			}

			chunksData.read(value.data(), value.size());

			chunksData.ignore(constants::crlf.size());

			std::string& chunk = isUTF8 ?
				chunks.emplace_back(json::utility::toUTF8JSON(value, CP_UTF8)) :
				chunks.emplace_back(move(value));

			chunksSize += chunk.size();
		}
	}

	HTTPParser::HTTPParser() :
		chunksSize(0),
		parsed(false)
	{

	}

	HTTPParser::HTTPParser(const std::string& HTTPMessage)
	{
		this->parse(HTTPMessage);
	}

	HTTPParser::HTTPParser(const std::vector<char>& HTTPMessage)
	{
		this->parse(std::string_view(HTTPMessage.data(), HTTPMessage.size()));
	}

	void HTTPParser::parse(std::string_view HTTPMessage)
	{
		if (HTTPMessage.empty())
		{
			parsed = false;

			return;
		}

		parsed = true;

		size_t prevString = 0;
		size_t nextString = HTTPMessage.find('\r');
		std::string_view firstString(HTTPMessage.data(), nextString);
		
		if (std::string_view temp = firstString.substr(0, firstString.find(' ')); temp.find("HTTP") == std::string_view::npos)
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
			ReadOnlyBuffer buffer(firstString);
			std::istringstream data;
			std::string responseCode;

			static_cast<std::ios&>(data).rdbuf(&buffer);

			data >> httpVersion >> responseCode >> response.second;

			response.first = stoi(responseCode);
		}
		else if (method != "CONNECT")
		{
			size_t startParameters = firstString.find('/');

			if (startParameters == std::string::npos)
			{
				throw exceptions::HTTPParseException("Can't find /");
			}

			startParameters++;

			size_t endParameters = firstString.rfind(' ');
			size_t queryStart = firstString.find('?');

			parameters = web::decodeUrl(std::string_view(firstString.begin() + startParameters, firstString.begin() + endParameters));
			httpVersion = std::string(firstString.begin() + firstString.find("HTTP"), firstString.end());

			if (queryStart != std::string::npos)
			{
				this->parseQueryParameter(std::string_view(firstString.data() + queryStart + 1, firstString.data() + endParameters));
			}
		}
		else
		{
			parameters = std::string(firstString.begin() + firstString.find(' ') + 1, firstString.begin() + firstString.rfind(' '));
			httpVersion = std::string(firstString.begin() + firstString.find("HTTP"), firstString.end());
		}

		while (true)
		{
			prevString = nextString + constants::crlf.size();
			nextString = HTTPMessage.find('\r', prevString);

			std::string_view next(HTTPMessage.data() + prevString, nextString - prevString);

			if (prevString == nextString || nextString == std::string::npos)
			{
				break;
			}

			size_t colonIndex = next.find(':');
			size_t nonSpace = colonIndex + 1;
			std::string header(next.begin(), next.begin() + colonIndex);

			while (next.size() > nonSpace && isspace(next[nonSpace]))
			{
				nonSpace++;
			}

			std::string value(next.begin() + nonSpace, next.end());

			headers.try_emplace(std::move(header), std::move(value));
		}

		bool isUTF8 = HTTPMessage.find(utf8Encoded) != std::string::npos;

		if (auto it = headers.find(transferEncodingHeader); it != headers.end())
		{
			static const std::unordered_map<std::string, void (HTTPParser::*)(std::string_view HTTPMessage, bool isUTF8)> transferTypeParsers =
			{
				{ chunkEncoded, &HTTPParser::parseChunkEncoded }
			};

			if (!transferTypeParsers.contains(it->second))
			{
				throw exceptions::HTTPParseException("Not supported transfer encoding: " + it->second);
			}

			std::invoke(transferTypeParsers.at(it->second), *this, HTTPMessage, isUTF8);
		}
		else if (headers.find(contentLengthHeader) != headers.end())
		{
			if (isUTF8)
			{
				body = json::utility::toUTF8JSON(std::string(HTTPMessage.begin() + HTTPMessage.find(crlfcrlf) + crlfcrlf.size(), HTTPMessage.end()), CP_UTF8);
			}
			else
			{
				body = std::string(HTTPMessage.begin() + HTTPMessage.find(crlfcrlf) + crlfcrlf.size(), HTTPMessage.end());
			}
		}

		this->parseContentType();
	}

	const std::string& HTTPParser::getMethod() const
	{
		return method;
	}

	double HTTPParser::getHTTPVersion() const
	{
		return stod(httpVersion.substr(5));
	}

	const std::string& HTTPParser::getParameters() const
	{
		return parameters;
	}

	const std::unordered_map<std::string, std::string>& HTTPParser::getQueryParameters() const
	{
		return queryParameters;
	}

	const std::pair<int, std::string>& HTTPParser::getFullResponse() const
	{
		return response;
	}

	int HTTPParser::getResponseCode() const
	{
		return response.first;
	}

	const std::string& HTTPParser::getResponseMessage() const
	{
		return response.second;
	}

	const HeadersMap& HTTPParser::getHeaders() const
	{
		return headers;
	}

	const std::string& HTTPParser::getBody() const
	{
		return body;
	}

	const std::vector<std::string>& HTTPParser::getChunks() const
	{
		return chunks;
	}

	const json::JsonParser& HTTPParser::getJson() const
	{
		return jsonParser;
	}

	const std::string& HTTPParser::getRawData() const
	{
		return rawData;
	}

	const std::vector<Multipart>& HTTPParser::getMultiparts() const
	{
		return multiparts;
	}

	HTTPParser::operator bool() const
	{
		return parsed;
	}

	std::ostream& operator << (std::ostream& outputStream, const HTTPParser& parser)
	{
		std::string result;

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

		result += constants::crlf;

		for (const auto& [header, value] : parser.headers)
		{
			result += std::format("{}: {}{}", header, value, constants::crlf);
		}

		if (parser.body.size())
		{
			result += std::format("{}{}", constants::crlf, parser.body);
		}
		else if (parser.chunks.size())
		{
			result += constants::crlf;

			for (const auto& chunk : parser.chunks)
			{
				result += std::format("{}{}{}", (std::ostringstream() << std::hex << chunk.size() << constants::crlf).str(), chunk, constants::crlf);
			}

			result += format("0{}", HTTPParser::crlfcrlf);
		}

		if (!result.ends_with(HTTPParser::crlfcrlf))
		{
			if (result.ends_with(constants::crlf))
			{
				result += constants::crlf;
			}
			else
			{
				result += HTTPParser::crlfcrlf;
			}
		}

		return outputStream << result;
	}

	std::istream& operator >> (std::istream& inputStream, HTTPParser& parser)
	{
		std::istreambuf_iterator<char> it(inputStream);
		std::string httpMessage(it, {});

		if (httpMessage.size())
		{
			parser.parse(httpMessage);
		}

		return inputStream;
	}
}
