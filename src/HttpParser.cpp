#include "HttpParser.h"

#include <iterator>
#include <format>
#include <algorithm>
#include <unordered_set>
#include <functional>

#include "HttpParserException.h"

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
	const std::unordered_map<std::string_view, std::function<void(HttpParser&, std::string_view)>> HttpParser::contentTypeParsers =
	{
		{ HttpParser::urlEncoded, [](HttpParser& parser, std::string_view data) { parser.parseQueryParameter(data); }},
		{ HttpParser::jsonEncoded, [](HttpParser& parser, std::string_view data) { parser.jsonParser.setJSONData(data); }},
		{ HttpParser::multipartEncoded, [](HttpParser& parser, std::string_view data) { parser.parseMultipart(data); }},
	};

	HttpParser::ReadOnlyBuffer::ReadOnlyBuffer(std::string_view view)
	{
		char* data = const_cast<char*>(view.data());

		setg(data, data, data + view.size());
	}

	std::string HttpParser::mergeChunks() const
	{
		std::string result;

		result.reserve(chunksSize);

		std::ranges::for_each(chunks, [&result](const std::string& value) { result += value; });

		return result;
	}

	void HttpParser::parseQueryParameter(std::string_view rawParameters)
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
	
	void HttpParser::parseMultipart(std::string_view data)
	{
		constexpr std::string_view boundaryText = "boundary=";

		const std::string& contentType = headers["Content-Type"];
		size_t index = contentType.find(boundaryText);

		if (index == std::string_view::npos)
		{
			throw std::runtime_error(std::format("Can't find {}", boundaryText));
		}

		std::string boundary = std::format("--{}", std::string_view(contentType.begin() + index + boundaryText.size(), contentType.end()));
		std::boyer_moore_horspool_searcher searcher(boundary.begin(), boundary.end());
		std::string_view::const_iterator current = std::search(data.begin(), data.end(), searcher);

		while (true)
		{
			std::string_view::const_iterator next = std::search(current + 1, data.end(), searcher);

			if (next == data.end())
			{
				break;
			}

			multiparts.emplace_back(std::string_view(current + boundary.size(), next));

			current = std::search(next, data.end(), searcher);
		}
	}

	void HttpParser::parseContentType()
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

	void HttpParser::parseChunkEncoded(std::string_view httpMessage, bool isUTF8)
	{
		size_t chunksStart = httpMessage.find(crlfcrlf);

		if (chunksStart == std::string_view::npos)
		{
			throw std::runtime_error(std::format("Can't find chunks start in {}", httpMessage));
		}

		chunksStart += crlfcrlf.size();

		if (chunksStart >= httpMessage.size())
		{
			throw std::runtime_error(std::format("Wrong stat chunks format in {}", httpMessage));
		}

		size_t chunksEnd = httpMessage.rfind(crlfcrlf);

		if (chunksEnd == std::string_view::npos)
		{
			throw std::runtime_error(std::format("Can't find chunks start in {}", httpMessage));
		}

		chunksEnd += crlfcrlf.size();

		if (chunksEnd > httpMessage.size())
		{
			throw std::runtime_error(std::format("Wrong end chunks format in {}", httpMessage));
		}

		ReadOnlyBuffer buffer(std::string_view(httpMessage.data() + chunksStart, chunksEnd - chunksStart));
		std::istringstream chunksData;

		static_cast<std::ios&>(chunksData).rdbuf(&buffer);

		chunksSize = 0;

		while (true)
		{
			std::string size;
			std::string value;

			std::getline(chunksData, size);

			size.pop_back(); // \r symbol from \r\n

			value.resize(std::stol(size, nullptr, 16));

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

	HttpParser::HttpParser() :
		chunksSize(0),
		parsed(false)
	{

	}

	HttpParser::HttpParser(const std::string& httpMessage)
	{
		this->parse(httpMessage);
	}

	HttpParser::HttpParser(const std::vector<char>& httpMessage)
	{
		this->parse(std::string_view(httpMessage.data(), httpMessage.size()));
	}

	void HttpParser::parse(std::string_view httpMessage)
	{
		if (httpMessage.empty())
		{
			parsed = false;

			return;
		}

		parsed = true;

		size_t prevString = 0;
		size_t nextString = httpMessage.find('\r');

		if (nextString == std::string_view::npos)
		{
			throw std::runtime_error(std::format("Can't find next string: {}", httpMessage));
		}

		std::string_view firstString(httpMessage.data(), nextString);
		
		if (std::string_view temp = firstString.substr(0, firstString.find(' ')); temp.find("HTTP") == std::string_view::npos)
		{
			method = temp;

			if (methods.find(method) == methods.end())
			{
				throw exceptions::HttpParserException(std::format("Wrong method: {}", method));
			}
		}

		chunksSize = 0;

		rawData = httpMessage;

		if (method.empty())
		{
			ReadOnlyBuffer buffer(firstString);
			std::istringstream data;
			std::string responseCode;

			static_cast<std::ios&>(data).rdbuf(&buffer);

			data >> httpVersion >> responseCode >> response.second;

			response.first = std::stoi(responseCode);
		}
		else if (method != "CONNECT")
		{
			size_t startParameters = firstString.find('/');

			if (startParameters == std::string::npos)
			{
				throw exceptions::HttpParserException("Can't find /");
			}

			startParameters++;

			size_t endParameters = firstString.rfind(' ');

			if (endParameters == std::string_view::npos)
			{
				throw std::runtime_error(std::format("Can't find end parameters in: {}", firstString));
			}

			size_t httpStartIndex = firstString.find("HTTP");

			if (httpStartIndex == std::string_view::npos)
			{
				throw std::runtime_error(std::format("Can't find HTTP in: {}", firstString));
			}

			parameters = web::decodeUrl(std::string_view(firstString.begin() + startParameters, firstString.begin() + endParameters));
			httpVersion = std::string(firstString.begin() + httpStartIndex, firstString.end());

			if (size_t queryStart = firstString.find('?'); queryStart != std::string::npos)
			{
				this->parseQueryParameter(std::string_view(firstString.data() + queryStart + 1, firstString.data() + endParameters));
			}
		}
		else
		{
			size_t space = firstString.find(' ');

			if (space == std::string_view::npos)
			{
				throw std::runtime_error(std::format("Can't find first space in {}", firstString));
			}

			size_t lastSpace = firstString.rfind(' ');

			if (lastSpace == std::string_view::npos)
			{
				throw std::runtime_error(std::format("Can't find first last space in {}", firstString));
			}

			size_t httpStartIndex = firstString.find("HTTP");

			if (httpStartIndex == std::string_view::npos)
			{
				throw std::runtime_error(std::format("Can't find HTTP in: {}", firstString));
			}

			parameters = std::string(firstString.begin() + space + 1, firstString.begin() + lastSpace);
			httpVersion = std::string(firstString.begin() + httpStartIndex, firstString.end());
		}

		while (true)
		{
			prevString = nextString + constants::crlf.size();
			nextString = httpMessage.find('\r', prevString);

			if (prevString == nextString || nextString == std::string::npos)
			{
				break;
			}

			std::string_view next(httpMessage.data() + prevString, nextString - prevString);

			size_t colonIndex = next.find(':');

			if (colonIndex == std::string_view::npos)
			{
				throw std::runtime_error(std::format("Can't find ':' while parsing headers in {}", next));
			}

			size_t nonSpace = colonIndex + 1;
			std::string header(next.begin(), next.begin() + colonIndex);

			while (next.size() > nonSpace && isspace(next[nonSpace]))
			{
				nonSpace++;
			}

			std::string value(next.begin() + nonSpace, next.end());

			headers.try_emplace(std::move(header), std::move(value));
		}

		bool isUTF8 = httpMessage.find(utf8Encoded) != std::string::npos;

		if (auto it = headers.find(transferEncodingHeader); it != headers.end())
		{
			static const std::unordered_map<std::string, void (HttpParser::*)(std::string_view httpMessage, bool isUTF8)> transferTypeParsers =
			{
				{ chunkEncoded, &HttpParser::parseChunkEncoded }
			};

			if (!transferTypeParsers.contains(it->second))
			{
				throw exceptions::HttpParserException("Not supported transfer encoding: " + it->second);
			}

			std::invoke(transferTypeParsers.at(it->second), *this, httpMessage, isUTF8);
		}
		else if (headers.find(contentLengthHeader) != headers.end())
		{
			if (isUTF8)
			{
				body = json::utility::toUTF8JSON(std::string_view(httpMessage.begin() + httpMessage.find(crlfcrlf) + crlfcrlf.size(), httpMessage.end()), CP_UTF8);
			}
			else
			{
				body = std::string(httpMessage.begin() + httpMessage.find(crlfcrlf) + crlfcrlf.size(), httpMessage.end());
			}
		}

		this->parseContentType();
	}

	const std::string& HttpParser::getMethod() const
	{
		return method;
	}

	double HttpParser::getHTTPVersion() const
	{
		return std::stod(httpVersion.substr(5));
	}

	const std::string& HttpParser::getParameters() const
	{
		return parameters;
	}

	const std::unordered_map<std::string, std::string>& HttpParser::getQueryParameters() const
	{
		return queryParameters;
	}

	const std::pair<int, std::string>& HttpParser::getFullResponse() const
	{
		return response;
	}

	int HttpParser::getResponseCode() const
	{
		return response.first;
	}

	const std::string& HttpParser::getResponseMessage() const
	{
		return response.second;
	}

	const HeadersMap& HttpParser::getHeaders() const
	{
		return headers;
	}

	const std::string& HttpParser::getBody() const
	{
		return body;
	}

	const std::vector<std::string>& HttpParser::getChunks() const
	{
		return chunks;
	}

	const json::JsonParser& HttpParser::getJson() const
	{
		return jsonParser;
	}

	const std::string& HttpParser::getRawData() const
	{
		return rawData;
	}

	const std::vector<Multipart>& HttpParser::getMultiparts() const
	{
		return multiparts;
	}

	HttpParser::operator bool() const
	{
		return parsed;
	}

	std::ostream& operator << (std::ostream& outputStream, const HttpParser& parser)
	{
		std::string result;

		if (parser.method.size())
		{
			if (parser.method != "CONNECT")
			{
				result += std::format("{} /{} {}", parser.method, parser.parameters, parser.httpVersion);
			}
			else
			{
				result += std::format("{} {} {}", parser.method, parser.parameters, parser.httpVersion);
			}
		}
		else
		{
			const auto& [code, message] = parser.getFullResponse();

			result += std::format("{} {} {}", parser.httpVersion, static_cast<int>(code), message);
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

			result += std::format("0{}", HttpParser::crlfcrlf);
		}

		if (!result.ends_with(HttpParser::crlfcrlf))
		{
			if (result.ends_with(constants::crlf))
			{
				result += constants::crlf;
			}
			else
			{
				result += HttpParser::crlfcrlf;
			}
		}

		return outputStream << result;
	}

	std::istream& operator >> (std::istream& inputStream, HttpParser& parser)
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
