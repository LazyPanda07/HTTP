#include "HttpBuilder.h"

#include <array>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <format>

#include "HttpParser.h"

static const std::unordered_set<std::string_view> availableHTTPVersions =
{
	"HTTP/0.9",
	"HTTP/1.0",
	"HTTP/1.1"
};

namespace web
{
	std::string HttpBuilder::getChunks(const std::vector<std::string>& chunks, bool partialChunks, bool preCalculateSize)
	{
		std::string result;

		if (preCalculateSize)
		{
			size_t resultSize = 0;

			for (const std::string& chunk : chunks)
			{
				resultSize += std::format("{:x}", chunk.size()).size() + constants::crlf.size() + chunk.size() + constants::crlf.size();
			}

			resultSize += 1 + constants::crlf.size();

			result.reserve(resultSize);
		}

		for (const std::string& chunk : chunks)
		{
			result += HttpBuilder::getChunk(chunk);
		}

		if (!partialChunks)
		{
			result += HttpBuilder::getChunk({});
		}

		return result;
	}

	std::string HttpBuilder::getChunk(std::string_view chunk)
	{
		return std::format("{:x}{}{}{}", chunk.size(), constants::crlf, chunk, constants::crlf);
	}

	HttpBuilder::HttpBuilder(std::string_view fullHTTPVersion) :
		_HTTPVersion(fullHTTPVersion),
		_partialChunks(false)
	{
		if (availableHTTPVersions.find(_HTTPVersion) == availableHTTPVersions.end())
		{
			throw std::runtime_error(std::format("HTTP version: {} not supported", _HTTPVersion));
		}
	}

	HttpBuilder& HttpBuilder::getRequest()
	{
		method = "GET";

		return *this;
	}

	HttpBuilder& HttpBuilder::postRequest()
	{
		method = "POST";

		return *this;
	}

	HttpBuilder& HttpBuilder::putRequest()
	{
		method = "PUT";

		return *this;
	}

	HttpBuilder& HttpBuilder::headRequest()
	{
		method = "HEAD";

		return *this;
	}

	HttpBuilder& HttpBuilder::optionsRequest()
	{
		method = "OPTIONS";

		return *this;
	}

	HttpBuilder& HttpBuilder::deleteRequest()
	{
		method = "DELETE";

		return *this;
	}

	HttpBuilder& HttpBuilder::connectRequest()
	{
		method = "CONNECT";

		return *this;
	}

	HttpBuilder& HttpBuilder::traceRequest()
	{
		method = "TRACE";

		return *this;
	}

	HttpBuilder& HttpBuilder::patchRequest()
	{
		method = "PATCH";

		return *this;
	}

	HttpBuilder& HttpBuilder::parameters(std::string_view parameters)
	{
		if (method != "CONNECT")
		{
			if (size_t queryStart = parameters.find('?'); queryStart != std::string::npos)
			{
				_parameters = std::format
				(
					"{}{}",
					std::string_view(parameters.begin(), parameters.begin() + queryStart + 1),
					std::string_view(parameters.begin() + queryStart + 1, parameters.end())
				);
			}
			else
			{
				_parameters = parameters;
			}

			if (!_parameters.starts_with('/'))
			{
				_parameters.insert(_parameters.begin(), '/');
			}
		}
		else
		{
			_parameters = parameters;
		}

		return *this;
	}

	HttpBuilder& HttpBuilder::responseCode(ResponseCodes code)
	{
		_responseCode = format("{} {}", static_cast<int>(code), getMessageFromCode(code));

		return *this;
	}

	HttpBuilder& HttpBuilder::responseCode(int code, std::string_view responseMessage)
	{
		_responseCode = format("{} {}", code, responseMessage);

		return *this;
	}

	HttpBuilder& HttpBuilder::HTTPVersion(std::string_view HTTPVersion)
	{
		if (HTTPVersion.find("HTTP") == std::string::npos)
		{
			_HTTPVersion = HTTPVersion;
		}
		else
		{
			_HTTPVersion = std::format("HTTP/{}", HTTPVersion);
		}

		if (availableHTTPVersions.find(_HTTPVersion) == availableHTTPVersions.end())
		{
			throw std::runtime_error(std::format("HTTP version: {} not supported", _HTTPVersion));
		}

		return *this;
	}

	HttpBuilder& HttpBuilder::chunks(const std::vector<std::string>& chunks)
	{
		_chunks.reserve(chunks.size());

		copy(chunks.begin(), chunks.end(), back_inserter(_chunks));

		return *this;
	}

	HttpBuilder& HttpBuilder::chunks(std::vector<std::string>&& chunks)
	{
		_chunks.reserve(chunks.size());

		move(chunks.begin(), chunks.end(), back_inserter(_chunks));

		return *this;
	}

	HttpBuilder& HttpBuilder::chunk(std::string_view chunk)
	{
		_chunks.emplace_back(chunk);

		return *this;
	}

	HttpBuilder& HttpBuilder::clear()
	{
		method.clear();
		_parameters.clear();
		_responseCode.clear();
		_headers.clear();

		return *this;
	}

	HttpBuilder& HttpBuilder::partialChunks()
	{
		_partialChunks = true;

		return *this;
	}

	std::ostream& operator << (std::ostream& outputStream, const HttpBuilder& builder)
	{
		return outputStream << builder.build();
	}
}
