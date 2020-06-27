#pragma once

#include <string>
#include <stdexcept>

#include "CheckAtCompileTime.h"

namespace web
{
	enum class ResponseCodes : short
	{
		Continue = 100,
		switchingProtocols,
		processing,
		ok = 200,
		created,
		accepted,
		nonAuthoritativeInformation,
		noContent,
		resetContent,
		partialContent,
		multiStatus,
		alreadyReported,
		IMUsed = 226,
		multipleChoices = 300,
		movedPermanently,
		found,
		seeOther,
		notModified,
		useProxy,
		temporaryRedirect = 307,
		permanentRedirect,
		badRequest = 400,
		unauthorized,
		paymentRequired,
		forbidden,
		notFound,
		methodNotAllowed,
		notAcceptable,
		proxyAuthenticationRequired,
		requestTimeout,
		conflict,
		gone,
		lengthRequired,
		preconditionFailed,
		payloadTooLarge,
		URITooLong,
		unsupportedMediaType,
		rangeNotSatisfiable,
		expectationFailed,
		iamATeapot,
		authenticationTimeout,
		misdirectedRequest = 421,
		unprocessableEntity,
		locked,
		failedDependency,
		upgradeRequired = 426,
		preconditionRequired = 428,
		tooManyRequests,
		requestHeaderFieldsTooLarge = 431,
		retryWith = 449,
		unavailableForLegalReasons = 451,
		clientClosedRequest = 499,
		internalServerError = 500,
		notImplemented,
		badGateway,
		serviceUnavailable,
		gatewayTimeout,
		HTTPVersionNotSupported,
		variantAlsoNegotiates,
		insufficientStorage,
		loopDetected,
		bandwidthLimitExceeded,
		notExtended,
		networkAuthenticationRequired,
		unknownError = 520,
		webServerIsDown,
		connectionTimedOut,
		originIsUnreachable,
		aTimeoutOccurred,
		SSLHandshakeFailed,
		invalidSSLCertificate
	};

	class HTTPBuilder
	{
	private:
		HTTPBuilder& parameters();

		HTTPBuilder& headers();

	private:
		static constexpr std::string_view httpVersion = "HTTP/1.1";

		std::string method;
		std::string _parameters;
		std::string _responseCode;
		std::string _headers;

	public:
		HTTPBuilder() = default;

		HTTPBuilder& getRequest();

		HTTPBuilder& postRequest();

		HTTPBuilder& putRequest();

		HTTPBuilder& headRequest();

		HTTPBuilder& optionsRequest();

		HTTPBuilder& deleteRequest();

		HTTPBuilder& connectRequest();

		HTTPBuilder& traceRequest();

		HTTPBuilder& patchRequest();

		template<typename StringT, typename T, typename... Args>
		HTTPBuilder& parameters(StringT&& name, T&& value, Args&&... args);

		HTTPBuilder& parameters(const std::string& parameters);

		HTTPBuilder& responseCode(ResponseCodes code);

		template<typename StringT, typename T, typename... Args>
		HTTPBuilder& headers(StringT&& name, T&& value, Args&&... args);

		std::string build(const std::string* const data = nullptr);

		HTTPBuilder& clear();

		~HTTPBuilder() = default;
	};

	template<typename StringT, typename T, typename... Args>
	HTTPBuilder& HTTPBuilder::parameters(StringT&& name, T&& value, Args&&... args)
	{
		if (_parameters.empty())
		{
			_parameters = "/?";
		}

		if constexpr (utility::StringConversion<StringT>::value)
		{
			if constexpr (std::is_arithmetic_v<T>)
			{
				_parameters += static_cast<std::string>(name) + std::string("=") + std::to_string(value) + std::string("&");
			}
			else if constexpr (utility::StringConversion<T>::value)
			{
				_parameters += static_cast<std::string>(name) + std::string("=") + static_cast<std::string>(value) + std::string("&");
			}
			else
			{
				throw std::logic_error("Bad type of T, it must be converted to string or arithmetic type");
			}
		}
		else
		{
			throw std::logic_error("Bad type of StringT, it must be converted to string");
		}

		return parameters(std::move(args)...);
	}

	template<typename StringT, typename T, typename... Args>
	HTTPBuilder& HTTPBuilder::headers(StringT&& name, T&& value, Args&&... args)
	{
		if constexpr (utility::StringConversion<StringT>::value)
		{
			if constexpr (std::is_arithmetic_v<T>)
			{
				_headers += static_cast<std::string>(name) + std::string(": ") + std::to_string(value) + std::string("\r\n");
			}
			else if constexpr (utility::StringConversion<T>::value)
			{
				_headers += static_cast<std::string>(name) + std::string(": ") + static_cast<std::string>(value) + std::string("\r\n");
			}
			else if constexpr (utility::checkBegin<T>::value && utility::checkEnd<T>::value)
			{
				_headers += static_cast<std::string>(name) + std::string(": ");

				auto checkValueType = *std::begin(value);

				if constexpr (std::is_arithmetic_v<decltype(checkValueType)>)
				{
					for (auto&& i : value)
					{
						_headers += std::to_string(i) + std::string(", ");
					}

					_headers.pop_back();	// delete space
					_headers.pop_back();	// delete ;

					_headers.insert(_headers.size(), "\r\n");
				}
				else if constexpr (utility::StringConversion<decltype(checkValueType)>::value)
				{
					for (auto&& i : value)
					{
						_headers += static_cast<std::string>(i) + std::string(", ");
					}

					_headers.pop_back();	// delete space
					_headers.pop_back();	// delete ;

					_headers.insert(_headers.size(), "\r\n");
				}
				else
				{
					throw std::locale("Bad type of values in class T, it must be converted to string or arithmetictype");
				}
			}
			else
			{
				throw std::logic_error("Bad type of T, it must be converted to string or arithmetic type");
			}
		}
		else
		{
			throw std::logic_error("Bad type of StringT, it must be converted to string");
		}

		return headers(std::move(args)...);
	}
}
