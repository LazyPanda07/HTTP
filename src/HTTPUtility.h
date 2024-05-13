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
	enum class responseCodes
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

	inline std::ostream& operator << (std::ostream& stream, responseCodes responseCode)
	{
		return stream << static_cast<int>(responseCode);
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
