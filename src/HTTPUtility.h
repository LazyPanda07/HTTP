#pragma once

#include <ostream>
#include <unordered_map>
#include <string>

#ifdef HTTP_DLL
#ifdef __LINUX__
#define HTTP_API __attribute__((visibility("default")))
#else
#define HTTP_API __declspec(dllexport)
#endif
#define HTTP_API_FUNCTION extern "C" HTTP_API
#define JSON_DLL
#else
#define HTTP_API
#define HTTP_API_FUNCTION
#endif // HTTP_DLL

namespace web
{
	/// @brief Response codes
	enum class ResponseCodes
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

	/**
	 * @brief Output stream operator for ResponseCodes
	 * @param stream 
	 * @param responseCode 
	 * @return 
	 */
	inline std::ostream& operator << (std::ostream& stream, ResponseCodes responseCode)
	{
		return stream << static_cast<int>(responseCode);
	}

	/**
	 * @brief Compare operator for ResponseCodes
	 * @param code 
	 * @param otherCode 
	 * @return 
	 */
	inline bool operator == (int code, ResponseCodes otherCode)
	{
		return code == static_cast<int>(otherCode);
	}

	/**
	 * @brief Get version of HTTP library
	 * @return 
	*/
	HTTP_API_FUNCTION std::string getHTTPLibraryVersion();

	/**
	 * @brief Encode data to application/x-www-form-urlencoded format
	 * @param data 
	 * @return 
	 */
	HTTP_API_FUNCTION std::string encodeUrl(std::string_view data);

	/**
	 * @brief Decode data from application/x-www-form-urlencoded format
	 * @param data
	 * @return
	 */
	HTTP_API_FUNCTION std::string decodeUrl(std::string_view data);

	/**
	 * @brief Get response message from response code
	 * @tparam T 
	 * @param code 
	 * @return 
	 */
	template<typename T> requires (std::same_as<T, ResponseCodes> || std::convertible_to<T, int>)
	std::string getMessageFromCode(const T& code);

	HTTP_API_FUNCTION std::string __getMessageFromCode(int code);

	/// @brief Custom hashing for headers with case insensitive
	struct HTTP_API insensitiveStringHash
	{
		size_t operator () (const std::string& value) const;
	};

	/// @brief Custom equal for headers
	struct HTTP_API insensitiveStringEqual
	{
		bool operator () (const std::string& left, const std::string& right) const;
	};

	/**
	 * @brief Case insensitive unordered_map
	*/
	using HeadersMap = std::unordered_map<std::string, std::string, insensitiveStringHash, insensitiveStringEqual>;
}

namespace web
{
	template<typename T> requires (std::same_as<T, ResponseCodes> || std::convertible_to<T, int>)
	std::string getMessageFromCode(const T& code)
	{
		return __getMessageFromCode(static_cast<int>(code));
	}
}
