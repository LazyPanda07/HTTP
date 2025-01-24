#include "HTTPUtility.h"

#include <iostream>
#include <algorithm>
#include <optional>
#include <charconv>
#include <array>

#include "HTTPParseException.h"
#include "HTTPParser.h"

using namespace std;

static optional<string_view> encodeSymbol(char symbol);

static optional<char> decodeSymbol(string_view symbol);

template<typename T>
struct Converter
{
	constexpr void convert(string_view data, T& result)
	{
		static_assert(false, "Wrong type");
	}
};

template<>
struct Converter<string>
{
	void convert(string_view data, string& result)
	{
		result = data;
	}
};

template<>
struct Converter<optional<string>>
{
	void convert(string_view data, optional<string>& result)
	{
		result = data;
	}
};

template<typename... Args>
class MultipartParser
{
private:
	array<size_t, sizeof...(Args)> offsets;
	array<char, sizeof...(Args)> nextCharacter;

private:
	template<size_t Index>
	constexpr auto& getValue(Args&... args) const
	{
		return get<Index>(forward_as_tuple(args...));
	}

	template<size_t Index = 0>
	constexpr void parseValue(string_view data, size_t offset, Args&... args) const
	{
		auto& value = this->getValue<Index>(args...);
		string_view stringValue(data.begin() + offset + offsets[Index], data.begin() + data.find(nextCharacter[Index], offset + offsets[Index]));
		Converter<remove_reference_t<decltype(value)>> converter;

		converter.convert(stringValue, value);

		if constexpr (Index + 1 != sizeof...(Args))
		{
			this->parseValue<Index + 1>(data, offset + stringValue.size(), args...);
		}
	}

public:
	constexpr MultipartParser(string_view format)
	{
		size_t offset = format.find("{}");
		size_t index = 0;

		while (offset != string_view::npos)
		{
#ifndef __LINUX__
#pragma warning(push)
#pragma warning(disable: 28020)
#endif
			offsets[index] = offset - 2 * index;
#ifndef __LINUX__
#pragma warning(pop)
#endif
			nextCharacter[index] = format[offset + 2];

			offset = format.find("{}", offset + 1);

			index++;
		}
	}

	void getValues(string_view data, Args&... args) const
	{
		this->parseValue(data, 0, args...);
	}
};

namespace web
{
	string getHTTPLibraryVersion()
	{
		string version = "1.10.0";

		return version;
	}

	string encodeUrl(string_view data)
	{
		string result;

		for (char symbol : data)
		{
			if (optional<string_view> encodedSymbol = encodeSymbol(symbol); encodedSymbol)
			{
				result += *encodedSymbol;
			}
			else
			{
				result += symbol;
			}
		}

		return result;
	}

	string decodeUrl(string_view data)
	{
		static constexpr size_t encodedSymbolSize = 3;

		string result;

		for (size_t i = 0; i < data.size(); i++)
		{
			if (data[i] == '%')
			{
				string_view percentEncodedData(data.data() + i, encodedSymbolSize);

				if (optional<char> encodedSymbol = decodeSymbol(percentEncodedData); encodedSymbol)
				{
					result += *encodedSymbol;
				}
				else
				{
					cerr << "Unknown encoded symbol: " << percentEncodedData << endl;

					result += percentEncodedData;
				}
				
				i += encodedSymbolSize - 1;
			}
			else
			{
				result += data[i];
			}
		}

		return result;
	}

	size_t InsensitiveStringHash::operator () (const string& value) const
	{
		string tem;

		tem.reserve(value.size());

		for_each(value.begin(), value.end(), [&tem](char c) { tem += tolower(c); });

		return hash<string>()(tem);
	}

	bool InsensitiveStringEqual::operator () (const string& left, const string& right) const
	{
		return equal
		(
			left.begin(), left.end(),
			right.begin(), right.end(),
			[](char first, char second) { return tolower(first) == tolower(second); }
		);
	}

	Multipart::Multipart(string_view data)
	{
		if (data.find("filename") != string_view::npos)
		{
			constexpr MultipartParser<string> parser(R"(Content-Disposition: form-data; name="{}")");

			parser.getValues(data.substr(0, data.find(web::HTTPParser::crlf)), name);
		}
		else
		{
			constexpr MultipartParser<string, optional<string>> parser(R"(Content-Disposition: form-data; name="{}"; filename="{}")");

			parser.getValues(data, name, fileName);
		}

		if (data.find("Content-Type:") != string_view::npos)
		{
			constexpr MultipartParser<optional<string>> parser("Content-Type: {}\r");

			parser.getValues(data, contentType);
		}
	}

	string __getMessageFromCode(int code)
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

		if (auto it = responseMessage.find(static_cast<ResponseCodes>(code)); it != responseMessage.end())
		{
			return it->second;
		}

		return "Unknown response code";
	}
}

optional<string_view> encodeSymbol(char symbol)
{
	switch (symbol)
	{
	case ' ':
		return "%20";

	case '!':
		return "%21";

	case '\"':
		return "%22";

	case '#':
		return "%23";

	case '$':
		return "%24";

	case '%':
		return "%25";

	case '&':
		return "%26";

	case '\'':
		return "%27";

	case '(':
		return "%28";

	case ')':
		return "%29";

	case '*':
		return "%2A";

	case '+':
		return "%2B";

	case ',':
		return "%2C";

	case '/':
		return "%2F";

	case ':':
		return "%3A";

	case ';':
		return "%3B";

	case '=':
		return "%3D";

	case '?':
		return "%3F";

	case '@':
		return "%40";

	case '[':
		return "%5B";

	case ']':
		return "%5D";

	default:
		return optional<string_view>();
	}
}

optional<char> decodeSymbol(string_view symbol)
{
	static const unordered_map<string_view, char> symbols =
	{
		{ "%20", ' ' },
		{ "%21", '!' },
		{ "%22", '\"' },
		{ "%23", '#' },
		{ "%24", '$' },
		{ "%25", '%' },
		{ "%26", '&' },
		{ "%27", '\'' },
		{ "%28", '(' },
		{ "%29", ')' },
		{ "%2A", '*' },
		{ "%2B", '+' },
		{ "%2C", ',' },
		{ "%2F", '/' },
		{ "%3A", ':' },
		{ "%3B", ';' },
		{ "%3D", '=' },
		{ "%3F", '?' },
		{ "%40", '@' },
		{ "%5B", '[' },
		{ "%5D", ']' }
	};

	auto it = symbols.find(symbol);

	return it != symbols.end() ? it->second : optional<char>();
}
