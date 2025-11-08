#include "HTTPBuilder.h"

#include <array>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <format>

#include "HTTPParser.h"

static const std::unordered_set<std::string_view> availableHTTPVersions =
{
	"HTTP/0.9",
	"HTTP/1.0",
	"HTTP/1.1"
};

namespace web
{
	std::string HTTPBuilder::getChunks(const std::vector<std::string>& chunks, bool partialChunks, bool preCalculateSize)
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
			result += HTTPBuilder::getChunk(chunk);
		}

		if (!partialChunks)
		{
			result += HTTPBuilder::getChunk({});
		}

		return result;
	}

	std::string HTTPBuilder::getChunk(std::string_view chunk)
	{
		return std::format("{:x}{}{}{}", chunk.size(), constants::crlf, chunk, constants::crlf);
	}

	HTTPBuilder::HTTPBuilder(std::string_view fullHTTPVersion) :
		_HTTPVersion(fullHTTPVersion),
		_partialChunks(false)
	{
		if (availableHTTPVersions.find(_HTTPVersion) == availableHTTPVersions.end())
		{
			throw std::runtime_error(std::format("HTTP version: {} not supported", _HTTPVersion));
		}
	}

	HTTPBuilder& HTTPBuilder::getRequest()
	{
		method = "GET";

		return *this;
	}

	HTTPBuilder& HTTPBuilder::postRequest()
	{
		method = "POST";

		return *this;
	}

	HTTPBuilder& HTTPBuilder::putRequest()
	{
		method = "PUT";

		return *this;
	}

	HTTPBuilder& HTTPBuilder::headRequest()
	{
		method = "HEAD";

		return *this;
	}

	HTTPBuilder& HTTPBuilder::optionsRequest()
	{
		method = "OPTIONS";

		return *this;
	}

	HTTPBuilder& HTTPBuilder::deleteRequest()
	{
		method = "DELETE";

		return *this;
	}

	HTTPBuilder& HTTPBuilder::connectRequest()
	{
		method = "CONNECT";

		return *this;
	}

	HTTPBuilder& HTTPBuilder::traceRequest()
	{
		method = "TRACE";

		return *this;
	}

	HTTPBuilder& HTTPBuilder::patchRequest()
	{
		method = "PATCH";

		return *this;
	}

	HTTPBuilder& HTTPBuilder::parameters(std::string_view parameters)
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

	HTTPBuilder& HTTPBuilder::responseCode(ResponseCodes code)
	{
		_responseCode = format("{} {}", static_cast<int>(code), getMessageFromCode(code));

		return *this;
	}

	HTTPBuilder& HTTPBuilder::responseCode(int code, std::string_view responseMessage)
	{
		_responseCode = format("{} {}", code, responseMessage);

		return *this;
	}

	HTTPBuilder& HTTPBuilder::HTTPVersion(std::string_view HTTPVersion)
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

	HTTPBuilder& HTTPBuilder::chunks(const std::vector<std::string>& chunks)
	{
		_chunks.reserve(chunks.size());

		copy(chunks.begin(), chunks.end(), back_inserter(_chunks));

		return *this;
	}

	HTTPBuilder& HTTPBuilder::chunks(std::vector<std::string>&& chunks)
	{
		_chunks.reserve(chunks.size());

		move(chunks.begin(), chunks.end(), back_inserter(_chunks));

		return *this;
	}

	HTTPBuilder& HTTPBuilder::chunk(std::string_view chunk)
	{
		_chunks.emplace_back(chunk);

		return *this;
	}

	HTTPBuilder& HTTPBuilder::clear()
	{
		method.clear();
		_parameters.clear();
		_responseCode.clear();
		_headers.clear();

		return *this;
	}

	HTTPBuilder& HTTPBuilder::partialChunks()
	{
		_partialChunks = true;

		return *this;
	}

	std::ostream& operator << (std::ostream& outputStream, const HTTPBuilder& builder)
	{
		return outputStream << builder.build();
	}
}
