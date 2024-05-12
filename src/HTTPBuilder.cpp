#include "HTTPBuilder.h"

#include <array>
#include <unordered_map>
#include <algorithm>
#include <format>

#include "HTTPParser.h"

using namespace std;

namespace web
{
	static const unordered_map<responseCodes, string> responseMessage =
	{
		{ responseCodes::Continue, "Continue" },
		{ responseCodes::switchingProtocols, "Switching Protocols" },
		{ responseCodes::processing, "Processing" },
		{ responseCodes::ok, "OK" },
		{ responseCodes::created, "Created" },
		{ responseCodes::accepted, "Accepted" },
		{ responseCodes::nonAuthoritativeInformation, "Non-Authoritative Information" },
		{ responseCodes::noContent, "No Content" },
		{ responseCodes::resetContent, "Reset Content" },
		{ responseCodes::partialContent, "Partial Content" },
		{ responseCodes::multiStatus, "Multi-Status" },
		{ responseCodes::alreadyReported, "Already Reported" },
		{ responseCodes::IMUsed, "IM Used" },
		{ responseCodes::multipleChoices, "Multiple Choices" },
		{ responseCodes::movedPermanently, "Moved Permanently" },
		{ responseCodes::found, "Found" },
		{ responseCodes::seeOther, "See Other" },
		{ responseCodes::notModified, "Not Modified" },
		{ responseCodes::useProxy, "Use Proxy" },
		{ responseCodes::temporaryRedirect, "Temporary Redirect" },
		{ responseCodes::permanentRedirect, "Permanent Redirect" },
		{ responseCodes::badRequest, "Bad Request" },
		{ responseCodes::unauthorized, "Unauthorized" },
		{ responseCodes::paymentRequired, "Payment Required" },
		{ responseCodes::forbidden, "Forbidden" },
		{ responseCodes::notFound, "Not Found" },
		{ responseCodes::methodNotAllowed, "Method Not Allowed" },
		{ responseCodes::notAcceptable, "Not Acceptable" },
		{ responseCodes::proxyAuthenticationRequired, "Proxy Authentication Required" },
		{ responseCodes::requestTimeout, "Request Timeout" },
		{ responseCodes::conflict, "Conflict" },
		{ responseCodes::gone, "Gone" },
		{ responseCodes::lengthRequired, "Length Required" },
		{ responseCodes::preconditionFailed, "Precondition Failed" },
		{ responseCodes::payloadTooLarge, "Payload Too Large" },
		{ responseCodes::URITooLong, "URI Tool Long" },
		{ responseCodes::unsupportedMediaType, "Unsupported Media Type" },
		{ responseCodes::rangeNotSatisfiable, "Range Not Satisfiable" },
		{ responseCodes::expectationFailed, "Expectation Failed" },
		{ responseCodes::iamATeapot, "I am teapot" },
		{ responseCodes::authenticationTimeout, "Authentication Timeout" },
		{ responseCodes::misdirectedRequest, "Misdirected Request" },
		{ responseCodes::unprocessableEntity, "Unprocessable Entity" },
		{ responseCodes::locked, "Locked" },
		{ responseCodes::failedDependency, "Failed Dependency" },
		{ responseCodes::upgradeRequired, "Upgrade Required" },
		{ responseCodes::preconditionRequired, "Precondition Required" },
		{ responseCodes::tooManyRequests, "Too Many Requests" },
		{ responseCodes::requestHeaderFieldsTooLarge, "Request Header Fields Too Large" },
		{ responseCodes::retryWith, "Retry With" },
		{ responseCodes::unavailableForLegalReasons, "Unavailable For Legal Reasons" },
		{ responseCodes::clientClosedRequest, "Client Closed Request" },
		{ responseCodes::internalServerError, "Internal Server Error" },
		{ responseCodes::notImplemented, "Not Implemented" },
		{ responseCodes::badGateway, "Bad Gateway" },
		{ responseCodes::serviceUnavailable, "Service Unavailable" },
		{ responseCodes::gatewayTimeout, "Gateway Timeout" },
		{ responseCodes::HTTPVersionNotSupported, "HTTP Version Not Supported" },
		{ responseCodes::variantAlsoNegotiates, "Variant Also Negotiates" },
		{ responseCodes::insufficientStorage, "Insufficient Storage" },
		{ responseCodes::loopDetected, "Loop Detected" },
		{ responseCodes::bandwidthLimitExceeded, "Bandwidth Limit Exceeded" },
		{ responseCodes::notExtended, "Not Extended" },
		{ responseCodes::networkAuthenticationRequired, "Network Authentication Required" },
		{ responseCodes::unknownError, "Unknown Error" },
		{ responseCodes::webServerIsDown, "Web Server Is Down" },
		{ responseCodes::connectionTimedOut, "Connection Timed Out" },
		{ responseCodes::originIsUnreachable, "Origin Is Unreachable" },
		{ responseCodes::aTimeoutOccurred, "A Timeout Occurred" },
		{ responseCodes::SSLHandshakeFailed, "SSL Handshake Failed" },
		{ responseCodes::invalidSSLCertificate, "Invalid SSL Certificate" }
	};

	HTTPBuilder& HTTPBuilder::parameters()
	{
		_parameters.pop_back();

		return *this;
	}

	string HTTPBuilder::getChunks(const vector<string>& chunks)
	{
		string result;

		for (const auto& i : chunks)
		{
			result += HTTPBuilder::getChunk(i);
		}

		result += "0\r\n\r\n";

		return result;
	}

	string HTTPBuilder::getChunk(const string& chunk)
	{
		ostringstream result;

		result << hex << chunk.size() << "\r\n";

		result << chunk << "\r\n";

		return result.str();
	}

	HTTPBuilder::HTTPBuilder(string_view fullHTTPVersion) :
		_HTTPVersion(fullHTTPVersion)
	{

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

	HTTPBuilder& HTTPBuilder::parameters(const string& parameters)
	{
		if (_parameters.empty() && method != "CONNECT")
		{
			_parameters = '/';
		}

		if (parameters.empty())
		{
			return *this;
		}

		_parameters += parameters;

		return *this;
	}

	HTTPBuilder& HTTPBuilder::responseCode(responseCodes code)
	{
		_responseCode = to_string(static_cast<int>(code)) + ' ' + responseMessage.at(code);

		return *this;
	}

	HTTPBuilder& HTTPBuilder::responseCode(int code, const string& responseMessage)
	{
		_responseCode = to_string(code) + ' ' + responseMessage;

		return *this;
	}

	HTTPBuilder& HTTPBuilder::HTTPVersion(const string& HTTPVersion)
	{
		if (HTTPVersion.find("HTTP") == string::npos)
		{
			_HTTPVersion = HTTPVersion;
		}
		else
		{
			_HTTPVersion = "HTTP/" + HTTPVersion;
		}

		return *this;
	}

	HTTPBuilder& HTTPBuilder::chunks(const vector<string>& chunks)
	{
		copy(chunks.begin(), chunks.end(), back_inserter(_chunks));

		return *this;
	}

	HTTPBuilder& HTTPBuilder::chunk(const string& chunk)
	{
		_chunks.push_back(chunk);

		return *this;
	}

	string HTTPBuilder::build(const string& data, const unordered_map<string, string>& additionalHeaders) const
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
			result = string(_HTTPVersion) + " " + _responseCode + "\r\n" + _headers;
		}
		else
		{
			result = method + ' ';

			if (_parameters.empty() && method != "CONNECT")
			{
				result += "/";
			}

			result += format("{} {}\r\n{}", _parameters, _HTTPVersion, _headers);
		}

		for (const auto& [header, value] : buildHeaders)
		{
			result += format("{}: {}{}", header, value, HTTPParser::crlf);
		}

		result += "\r\n";

		if (data.size())
		{
			result += data;
		}
		else if (_chunks.size())
		{
			result += HTTPBuilder::getChunks(_chunks);
		}

		return result;
	}

	string HTTPBuilder::build(const json::JSONBuilder& builder, const unordered_map<string, string>& additionalHeaders) const
	{
		string json = builder.build();
		unordered_map<string, string> contentTypeHeader(additionalHeaders);

		contentTypeHeader["Content-Type"] = "application/json";

		return this->build(json, contentTypeHeader);
	}

	HTTPBuilder& HTTPBuilder::clear()
	{
		method = _parameters = _responseCode = _headers = "";

		return *this;
	}

	ostream& operator << (ostream& outputStream, const HTTPBuilder& builder)
	{
		return outputStream << builder.build();
	}
}
