#include "HTTPBuilder.h"

#include <array>
#include <unordered_map>

using namespace std;

constexpr std::array<char, 4> convert(web::responseCodes code);

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

	HTTPBuilder::HTTPBuilder(const string& fullHTTPVersion) :
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
		_parameters = '/' + parameters;

		return *this;
	}

	HTTPBuilder& HTTPBuilder::responseCode(responseCodes code)
	{
		_responseCode = convert(code).data() + string(" ") + responseMessage.at(code);

		return *this;
	}

	HTTPBuilder& HTTPBuilder::HTTPVersion(const string& HTTPVersion)
	{
		_HTTPVersion = HTTPVersion;

		return *this;
	}

	string HTTPBuilder::build(const string& data)
	{
		string result;

		if (data.size())
		{
			this->headers
			(
				"Content-Length", data.size()
			);
		}

		if (method.empty())	//response 
		{
			result = string(_HTTPVersion) + " " + _responseCode + "\r\n" + _headers;
		}
		else	//request
		{
			if (_parameters.empty())
			{
				_parameters = "/";
			}

			result = method + " " + _parameters + " " + _HTTPVersion + "\r\n" + _headers;
		}

		result += "\r\n";

		if (data.size())
		{
			result += data;
		}

		return result;
	}

	string HTTPBuilder::build(const json::JSONBuilder& builder)
	{
		string json = builder.build();

		this->headers
		(
			"Content-Type", "application/json"
		);

		return this->build(json);
	}

	HTTPBuilder& HTTPBuilder::clear()
	{
		method = _parameters = _responseCode = _headers = "";

		return *this;
	}
}

constexpr std::array<char, 4> convert(web::responseCodes code)
{
	std::array<char, 4> res{};
	short tem = static_cast<short>(code);
	size_t i = 2;

	while (tem)
	{
		res[i--] = tem % 10 + '0';

		tem /= 10;
	}

	return res;
}