#include "HttpUtility.h"

#include <iostream>
#include <algorithm>
#include <optional>
#include <charconv>
#include <array>
#include <cassert>

#include "HttpParserException.h"
#include "HttpParser.h"

static std::optional<std::string_view> encodeSymbol(char symbol);

static std::optional<char> decodeSymbol(std::string_view symbol);

template<typename T>
struct Converter
{
	constexpr void convert(std::string_view data, T& result)
	{
		static_assert(false, "Wrong type");
	}
};

template<>
struct Converter<std::string>
{
	void convert(std::string_view data, std::string& result)
	{
		result = data;
	}
};

template<>
struct Converter<std::optional<std::string>>
{
	void convert(std::string_view data, std::optional<std::string>& result)
	{
		result = data;
	}
};

template<typename... Args>
class MultipartParser
{
private:
	std::array<size_t, sizeof...(Args)> offsets;
	std::array<char, sizeof...(Args)> nextCharacter;

private:
	template<size_t Index>
	constexpr auto& getValue(Args&... args) const
	{
		return std::get<Index>(std::forward_as_tuple(args...));
	}

	template<size_t Index = 0>
	constexpr void parseValue(std::string_view data, size_t offset, Args&... args) const
	{
		auto& value = this->getValue<Index>(args...);
		Converter<std::remove_reference_t<decltype(value)>> converter;
		size_t stringValueIndex = data.find(nextCharacter[Index], offset + offsets[Index]);
		std::string_view stringValue(data.begin() + offset + offsets[Index], (stringValueIndex == std::string_view::npos) ? data.end() : data.begin() + stringValueIndex);

		converter.convert(stringValue, value);

		if constexpr (Index + 1 != sizeof...(Args))
		{
			this->parseValue<Index + 1>(data, offset + stringValue.size(), args...);
		}
	}

public:
	constexpr MultipartParser(std::string_view format)
	{
		size_t offset = format.find("{}");
		size_t index = 0;

		while (offset != std::string_view::npos)
		{
#ifndef __LINUX__
#pragma warning(push)
#pragma warning(disable: 28020)
#endif
			offsets[index] = offset - 2 * index;
#ifndef __LINUX__
#pragma warning(pop)
#endif
			format.size() > offset + 2 ?
				nextCharacter[index] = format[offset + 2] :
				nextCharacter[index] = '\0';

			offset = format.find("{}", offset + 1);

			index++;
		}
	}

	void getValues(std::string_view data, Args&... args) const
	{
		this->parseValue(data, 0, args...);
	}
};

namespace web
{
	std::string getHTTPLibraryVersion()
	{
		std::string version = "1.14.0";

		return version;
	}

	std::string encodeUrl(std::string_view data)
	{
		std::string result;

		for (char symbol : data)
		{
			if (std::optional<std::string_view> encodedSymbol = encodeSymbol(symbol); encodedSymbol)
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

	std::string decodeUrl(std::string_view data)
	{
		static constexpr size_t encodedSymbolSize = 3;

		std::string result;

		for (size_t i = 0; i < data.size(); i++)
		{
			if (data[i] == '%')
			{
				std::string_view percentEncodedData(data.data() + i, encodedSymbolSize);

				if (std::optional<char> encodedSymbol = decodeSymbol(percentEncodedData); encodedSymbol)
				{
					result += *encodedSymbol;
				}
				else
				{
					std::cerr << "Unknown encoded symbol: " << percentEncodedData << std::endl;

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

	size_t InsensitiveStringHash::operator ()(const std::string& value) const
	{
		std::string tem;

		tem.reserve(value.size());

		for_each(value.begin(), value.end(), [&tem](char c) { tem += tolower(c); });

		return std::hash<std::string>()(tem);
	}

	bool InsensitiveStringEqual::operator ()(const std::string& left, const std::string& right) const
	{
		return std::equal
		(
			left.begin(), left.end(),
			right.begin(), right.end(),
			[](char first, char second) { return tolower(first) == tolower(second); }
		);
	}

	Multipart::Multipart(std::string_view data)
	{
		if (data.starts_with(constants::crlf))
		{
			data = std::string_view(data.begin() + constants::crlf.size(), data.end());
		}

		size_t firstStringEnd = data.find(constants::crlf);

		if (data.find("filename") != std::string_view::npos)
		{
			constexpr MultipartParser<std::string, std::optional<std::string>> parser(R"(Content-Disposition: form-data; name="{}"; filename="{}")");

			parser.getValues(data.substr(0, firstStringEnd), name, fileName);
		}
		else
		{
			constexpr MultipartParser<std::string> parser(R"(Content-Disposition: form-data; name="{}")");

			parser.getValues(data.substr(0, firstStringEnd), name);
		}

		if (data.find("Content-Type:") != std::string_view::npos)
		{
			constexpr MultipartParser<std::optional<std::string>> parser("Content-Type: {}");

			parser.getValues
			(
				std::string_view
				(
					data.begin() + firstStringEnd + constants::crlf.size(),
					data.begin() + data.find(constants::crlf, firstStringEnd + constants::crlf.size())
				),
				contentType
			);
		}

		this->data = std::string(data.begin() + data.find(HttpParser::crlfcrlf) + HttpParser::crlfcrlf.size(), data.end() - constants::crlf.size());
	}

	Multipart::Multipart(std::string_view name, const std::optional<std::string>& fileName, const std::optional<std::string>& contentType, std::string&& data) :
		name(name),
		fileName(fileName),
		contentType(contentType),
		data(move(data))
	{

	}

	const std::string& Multipart::getName() const
	{
		return name;
	}

	const std::optional<std::string>& Multipart::getFileName() const
	{
		return fileName;
	}

	const std::optional<std::string>& Multipart::getContentType() const
	{
		return contentType;
	}

	const std::string& Multipart::getData() const
	{
		return data;
	}

	std::string __getMessageFromCode(int code)
	{
		static const std::unordered_map<ResponseCodes, std::string> responseMessage =
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

std::optional<std::string_view> encodeSymbol(char symbol)
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
		return std::optional<std::string_view>();
	}
}

std::optional<char> decodeSymbol(std::string_view symbol)
{
	static const std::unordered_map<std::string_view, char> symbols =
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

	return it != symbols.end() ? it->second : std::optional<char>();
}
