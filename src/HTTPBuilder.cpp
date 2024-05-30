#include "HTTPBuilder.h"

#include <array>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <format>

#include "HTTPParser.h"

using namespace std;

static const unordered_map<web::responseCodes, string> responseMessage =
{
	{ web::responseCodes::Continue, "Continue" },
	{ web::responseCodes::switchingProtocols, "Switching Protocols" },
	{ web::responseCodes::processing, "Processing" },
	{ web::responseCodes::ok, "OK" },
	{ web::responseCodes::created, "Created" },
	{ web::responseCodes::accepted, "Accepted" },
	{ web::responseCodes::nonAuthoritativeInformation, "Non-Authoritative Information" },
	{ web::responseCodes::noContent, "No Content" },
	{ web::responseCodes::resetContent, "Reset Content" },
	{ web::responseCodes::partialContent, "Partial Content" },
	{ web::responseCodes::multiStatus, "Multi-Status" },
	{ web::responseCodes::alreadyReported, "Already Reported" },
	{ web::responseCodes::IMUsed, "IM Used" },
	{ web::responseCodes::multipleChoices, "Multiple Choices" },
	{ web::responseCodes::movedPermanently, "Moved Permanently" },
	{ web::responseCodes::found, "Found" },
	{ web::responseCodes::seeOther, "See Other" },
	{ web::responseCodes::notModified, "Not Modified" },
	{ web::responseCodes::useProxy, "Use Proxy" },
	{ web::responseCodes::temporaryRedirect, "Temporary Redirect" },
	{ web::responseCodes::permanentRedirect, "Permanent Redirect" },
	{ web::responseCodes::badRequest, "Bad Request" },
	{ web::responseCodes::unauthorized, "Unauthorized" },
	{ web::responseCodes::paymentRequired, "Payment Required" },
	{ web::responseCodes::forbidden, "Forbidden" },
	{ web::responseCodes::notFound, "Not Found" },
	{ web::responseCodes::methodNotAllowed, "Method Not Allowed" },
	{ web::responseCodes::notAcceptable, "Not Acceptable" },
	{ web::responseCodes::proxyAuthenticationRequired, "Proxy Authentication Required" },
	{ web::responseCodes::requestTimeout, "Request Timeout" },
	{ web::responseCodes::conflict, "Conflict" },
	{ web::responseCodes::gone, "Gone" },
	{ web::responseCodes::lengthRequired, "Length Required" },
	{ web::responseCodes::preconditionFailed, "Precondition Failed" },
	{ web::responseCodes::payloadTooLarge, "Payload Too Large" },
	{ web::responseCodes::URITooLong, "URI Tool Long" },
	{ web::responseCodes::unsupportedMediaType, "Unsupported Media Type" },
	{ web::responseCodes::rangeNotSatisfiable, "Range Not Satisfiable" },
	{ web::responseCodes::expectationFailed, "Expectation Failed" },
	{ web::responseCodes::iamATeapot, "I am teapot" },
	{ web::responseCodes::authenticationTimeout, "Authentication Timeout" },
	{ web::responseCodes::misdirectedRequest, "Misdirected Request" },
	{ web::responseCodes::unprocessableEntity, "Unprocessable Entity" },
	{ web::responseCodes::locked, "Locked" },
	{ web::responseCodes::failedDependency, "Failed Dependency" },
	{ web::responseCodes::upgradeRequired, "Upgrade Required" },
	{ web::responseCodes::preconditionRequired, "Precondition Required" },
	{ web::responseCodes::tooManyRequests, "Too Many Requests" },
	{ web::responseCodes::requestHeaderFieldsTooLarge, "Request Header Fields Too Large" },
	{ web::responseCodes::retryWith, "Retry With" },
	{ web::responseCodes::unavailableForLegalReasons, "Unavailable For Legal Reasons" },
	{ web::responseCodes::clientClosedRequest, "Client Closed Request" },
	{ web::responseCodes::internalServerError, "Internal Server Error" },
	{ web::responseCodes::notImplemented, "Not Implemented" },
	{ web::responseCodes::badGateway, "Bad Gateway" },
	{ web::responseCodes::serviceUnavailable, "Service Unavailable" },
	{ web::responseCodes::gatewayTimeout, "Gateway Timeout" },
	{ web::responseCodes::HTTPVersionNotSupported, "HTTP Version Not Supported" },
	{ web::responseCodes::variantAlsoNegotiates, "Variant Also Negotiates" },
	{ web::responseCodes::insufficientStorage, "Insufficient Storage" },
	{ web::responseCodes::loopDetected, "Loop Detected" },
	{ web::responseCodes::bandwidthLimitExceeded, "Bandwidth Limit Exceeded" },
	{ web::responseCodes::notExtended, "Not Extended" },
	{ web::responseCodes::networkAuthenticationRequired, "Network Authentication Required" },
	{ web::responseCodes::unknownError, "Unknown Error" },
	{ web::responseCodes::webServerIsDown, "Web Server Is Down" },
	{ web::responseCodes::connectionTimedOut, "Connection Timed Out" },
	{ web::responseCodes::originIsUnreachable, "Origin Is Unreachable" },
	{ web::responseCodes::aTimeoutOccurred, "A Timeout Occurred" },
	{ web::responseCodes::SSLHandshakeFailed, "SSL Handshake Failed" },
	{ web::responseCodes::invalidSSLCertificate, "Invalid SSL Certificate" }
};

static const unordered_set<string_view> availableHTTPVersions =
{
	"HTTP/0.9",
	"HTTP/1.0",
	"HTTP/1.1",
	"HTTP/2",
	"HTTP/3"
};

namespace web
{
	HTTPBuilder& HTTPBuilder::parameters()
	{
		_parameters.pop_back();

		return *this;
	}

	string HTTPBuilder::getChunks(const vector<string>& chunks, bool partialChunks, bool preCalculateSize)
	{
		string result;

		if (preCalculateSize)
		{
			size_t resultSize = 0;

			for (const string& chunk : chunks)
			{
				resultSize += format("{:x}", chunk.size()).size() + HTTPParser::crlf.size() + chunk.size() + HTTPParser::crlf.size();
			}

			resultSize += 1 + HTTPParser::crlf.size();

			result.reserve(resultSize);
		}

		for (const string& chunk : chunks)
		{
			result += HTTPBuilder::getChunk(chunk);
		}

		if (!partialChunks)
		{
			result += HTTPBuilder::getChunk({});
		}

		return result;
	}

	string HTTPBuilder::getChunk(string_view chunk)
	{
		return format("{:x}{}{}{}", chunk.size(), HTTPParser::crlf, chunk, HTTPParser::crlf);
	}

	HTTPBuilder::HTTPBuilder(string_view fullHTTPVersion) :
		_HTTPVersion(fullHTTPVersion),
		_partialChunks(false)
	{
		if (availableHTTPVersions.find(fullHTTPVersion) == availableHTTPVersions.end())
		{
			throw runtime_error("Wrong HTTP version");
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

	HTTPBuilder& HTTPBuilder::parameters(string_view parameters)
	{
		if (method != "CONNECT")
		{
			if (size_t queryStart = parameters.find('?'); queryStart != string::npos)
			{
				_parameters = format
				(
					"{}{}",
					string_view(parameters.begin(), parameters.begin() + queryStart + 1),
					string_view(parameters.begin() + queryStart + 1, parameters.end())
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

	HTTPBuilder& HTTPBuilder::responseCode(responseCodes code)
	{
		_responseCode = to_string(static_cast<int>(code)) + ' ' + responseMessage.at(code);

		return *this;
	}

	HTTPBuilder& HTTPBuilder::responseCode(int code, string_view responseMessage)
	{
		_responseCode = format("{} {}", code, responseMessage);

		return *this;
	}

	HTTPBuilder& HTTPBuilder::HTTPVersion(string_view HTTPVersion)
	{
		if (HTTPVersion.find("HTTP") == string::npos)
		{
			_HTTPVersion = HTTPVersion;
		}
		else
		{
			_HTTPVersion = format("HTTP/{}", HTTPVersion);
		}

		if (availableHTTPVersions.find(_HTTPVersion) == availableHTTPVersions.end())
		{
			throw runtime_error("Wrong HTTP version");
		}

		return *this;
	}

	HTTPBuilder& HTTPBuilder::chunks(const vector<string>& chunks)
	{
		copy(chunks.begin(), chunks.end(), back_inserter(_chunks));

		return *this;
	}

	HTTPBuilder& HTTPBuilder::chunk(string_view chunk)
	{
		_chunks.emplace_back(chunk);

		return *this;
	}

	string HTTPBuilder::build(string_view data, const unordered_map<string, string>& additionalHeaders) const
	{
		string result;
		unordered_map<string, string> buildHeaders(additionalHeaders);

		if (data.size())
		{
			buildHeaders["Content-Length"] = to_string(data.size());
		}
		else if (_chunks.size())
		{
			buildHeaders["Transfer-Encoding"] = "chunked";
		}

		if (method.empty())
		{
			result = format("{} {}{}{}", _HTTPVersion, _responseCode, HTTPParser::crlf, _headers);
		}
		else
		{
			result = method + ' ';

			if (_parameters.empty() && method != "CONNECT")
			{
				result += "/";
			}

			result += format("{} {}{}{}", _parameters, _HTTPVersion, HTTPParser::crlf, _headers);
		}

		for (const auto& [header, value] : buildHeaders)
		{
			result += format("{}: {}{}", header, value, HTTPParser::crlf);
		}

		result += HTTPParser::crlf;

		if (data.size())
		{
			result += data;
		}
		else if (_chunks.size())
		{
			result += HTTPBuilder::getChunks(_chunks, _partialChunks);
		}

		return result;
	}

	string HTTPBuilder::build(const json::JSONBuilder& builder, unordered_map<string, string> additionalHeaders) const
	{
		additionalHeaders["Content-Type"] = "application/json";

		return this->build(builder.build(), additionalHeaders);
	}

	string HTTPBuilder::build(const unordered_map<string, string>& urlEncoded, unordered_map<string, string> additionalHeaders) const
	{
		string body;

		for (const auto& [key, value] : urlEncoded)
		{
			body += format("{}={}&", web::encodeUrl(key), web::encodeUrl(value));
		}

		body.pop_back(); // remove last &

		additionalHeaders["Content-Type"] = "application/x-www-form-urlencoded";

		return this->build(body, additionalHeaders);
	}

	HTTPBuilder& HTTPBuilder::clear()
	{
		method = _parameters = _responseCode = _headers = "";

		return *this;
	}

	HTTPBuilder& HTTPBuilder::partialChunks()
	{
		_partialChunks = true;
	}

	ostream& operator << (ostream& outputStream, const HTTPBuilder& builder)
	{
		return outputStream << builder.build();
	}
}
