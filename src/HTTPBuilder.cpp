#include "HTTPBuilder.h"

#include <array>
#include <unordered_map>

using namespace std;

constexpr std::array<char, 4> convert(web::ResponseCodes code);

namespace web
{
	static const unordered_map<ResponseCodes, string> responseMessage =
	{
		{ ResponseCodes::Continue, "Continue" },
		{ ResponseCodes::switchingProtocols, "Switching Protocols" },
		{ ResponseCodes::processing, "Processing" },
		{ ResponseCodes::ok, "OK" },
		{ ResponseCodes::created, "Created" },
		{ ResponseCodes::accepted, "Accepted" },
		{ ResponseCodes::nonAuthoritativeInformation, "Non-Authoritative Information" },
		{ ResponseCodes::noContent, "No Content" },
		{ ResponseCodes::resetContent, "Reset Content" },
		{ ResponseCodes::partialContent, "Partial Content" },
		{ ResponseCodes::multiStatus, "Multi-Status" },
		{ ResponseCodes::alreadyReported, "Already Reported" },
		{ ResponseCodes::IMUsed, "IM Used" },
		{ ResponseCodes::multipleChoices, "Multiple Choices" },
		{ ResponseCodes::movedPermanently, "Moved Permanently" },
		{ ResponseCodes::found, "Found" },
		{ ResponseCodes::seeOther, "See Other" },
		{ ResponseCodes::notModified, "Not Modified" },
		{ ResponseCodes::useProxy, "Use Proxy" },
		{ ResponseCodes::temporaryRedirect, "Temporary Redirect" },
		{ ResponseCodes::permanentRedirect, "Permanent Redirect" },
		{ ResponseCodes::badRequest, "Bad Request" },
		{ ResponseCodes::unauthorized, "Unauthorized" },
		{ ResponseCodes::paymentRequired, "Payment Required" },
		{ ResponseCodes::forbidden, "Forbidden" },
		{ ResponseCodes::notFound, "Not Found" },
		{ ResponseCodes::methodNotAllowed, "Method Not Allowed" },
		{ ResponseCodes::notAcceptable, "Not Acceptable" },
		{ ResponseCodes::proxyAuthenticationRequired, "Proxy Authentication Required" },
		{ ResponseCodes::requestTimeout, "Request Timeout" },
		{ ResponseCodes::conflict, "Conflict" },
		{ ResponseCodes::gone, "Gone" },
		{ ResponseCodes::lengthRequired, "Length Required" },
		{ ResponseCodes::preconditionFailed, "Precondition Failed" },
		{ ResponseCodes::payloadTooLarge, "Payload Too Large" },
		{ ResponseCodes::URITooLong, "URI Tool Long" },
		{ ResponseCodes::unsupportedMediaType, "Unsupported Media Type" },
		{ ResponseCodes::rangeNotSatisfiable, "Range Not Satisfiable" },
		{ ResponseCodes::expectationFailed, "Expectation Failed" },
		{ ResponseCodes::iamATeapot, "I am teapot" },
		{ ResponseCodes::authenticationTimeout, "Authentication Timeout" },
		{ ResponseCodes::misdirectedRequest, "Misdirected Request" },
		{ ResponseCodes::unprocessableEntity, "Unprocessable Entity" },
		{ ResponseCodes::locked, "Locked" },
		{ ResponseCodes::failedDependency, "Failed Dependency" },
		{ ResponseCodes::upgradeRequired, "Upgrade Required" },
		{ ResponseCodes::preconditionRequired, "Precondition Required" },
		{ ResponseCodes::tooManyRequests, "Too Many Requests" },
		{ ResponseCodes::requestHeaderFieldsTooLarge, "Request Header Fields Too Large" },
		{ ResponseCodes::retryWith, "Retry With" },
		{ ResponseCodes::unavailableForLegalReasons, "Unavailable For Legal Reasons" },
		{ ResponseCodes::clientClosedRequest, "Client Closed Request" },
		{ ResponseCodes::internalServerError, "Internal Server Error" },
		{ ResponseCodes::notImplemented, "Not Implemented" },
		{ ResponseCodes::badGateway, "Bad Gateway" },
		{ ResponseCodes::serviceUnavailable, "Service Unavailable" },
		{ ResponseCodes::gatewayTimeout, "Gateway Timeout" },
		{ ResponseCodes::HTTPVersionNotSupported, "HTTP Version Not Supported" },
		{ ResponseCodes::variantAlsoNegotiates, "Variant Also Negotiates" },
		{ ResponseCodes::insufficientStorage, "Insufficient Storage" },
		{ ResponseCodes::loopDetected, "Loop Detected" },
		{ ResponseCodes::bandwidthLimitExceeded, "Bandwidth Limit Exceeded" },
		{ ResponseCodes::notExtended, "Not Extended" },
		{ ResponseCodes::networkAuthenticationRequired, "Network Authentication Required" },
		{ ResponseCodes::unknownError, "Unknown Error" },
		{ ResponseCodes::webServerIsDown, "Web Server Is Down" },
		{ ResponseCodes::connectionTimedOut, "Connection Timed Out" },
		{ ResponseCodes::originIsUnreachable, "Origin Is Unreachable" },
		{ ResponseCodes::aTimeoutOccurred, "A Timeout Occurred" },
		{ ResponseCodes::SSLHandshakeFailed, "SSL Handshake Failed" },
		{ ResponseCodes::invalidSSLCertificate, "Invalid SSL Certificate" }
	};

	HTTPBuilder& HTTPBuilder::parameters()
	{
		_parameters.pop_back();

		return *this;
	}

	HTTPBuilder::HTTPBuilder() :
		_HTTPVersion("HTTP/1.1")
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

	HTTPBuilder& HTTPBuilder::responseCode(ResponseCodes code)
	{
		_responseCode = convert(code).data() + string(" ") + responseMessage.at(code);

		return *this;
	}

	HTTPBuilder& HTTPBuilder::HTTPVersion(const string& HTTPVersion)
	{
		_HTTPVersion = HTTPVersion;

		return *this;
	}

	string HTTPBuilder::build(const string* const data)
	{
		string result;

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

		if (data)
		{
			result += *data;
		}

		return result;
	}

	HTTPBuilder& HTTPBuilder::clear()
	{
		method = _parameters = _responseCode = _headers = "";

		return *this;
	}
}

constexpr std::array<char, 4> convert(web::ResponseCodes code)
{
	std::array<char, 4> res{};
	short tem = static_cast<short>(code);
	size_t i = 2;

	while (tem)
	{
		res[i--] = tem % 10 + '0';

		tem *= 0.1;
	}

	return res;
}