#pragma once

#ifdef HTTP_DLL
#define HTTP_API __declspec(dllexport)
#define JSON_DLL
#else
#define HTTP_API
#endif // HTTP_DLL

#include <unordered_map>
#include <string>
#include <vector>

#include "JSONParser.h"
#include "HTTPUtility.h"

#pragma comment (lib, "JSON.lib")

namespace web
{
	/// @brief HTTP parser
	class HTTP_API HTTPParser final
	{
	private:
		struct readOnlyBuffer : public std::streambuf
		{
			readOnlyBuffer(std::string_view view);
		};

	public:
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

	public:
		static inline const std::string contentLengthHeader = "Content-Length";
		static inline const std::string contentTypeHeader = "Content-Type";
		static inline const std::string transferEncodingHeader = "Transfer-Encoding";
		static inline const std::string utf8Encoded = "charset=utf-8";
		static inline const std::string chunkEncoded = "chunked";
		static inline constexpr std::string_view crlfcrlf = "\r\n\r\n";
		static inline constexpr std::string_view crlf = "\r\n";
		static inline constexpr std::string_view urlEncoded = "application/x-www-form-urlencoded";
		static inline constexpr std::string_view jsonEncoded = "application/json";

	private:
		std::unordered_map<std::string, std::string, insensitiveStringHash, insensitiveStringEqual> headers;
		std::unordered_map<std::string, std::string> keyValueParameters;
		std::pair<responseCodes, std::string> response;	//code - response message
		std::string method;
		std::string httpVersion;
		std::string parameters;
		std::string body;
		std::vector<std::string> chunks;
		json::JSONParser jsonParser;

	private:
		void parseKeyValueParameter(std::string_view rawParameters);

	public:
		HTTPParser() = default;

		HTTPParser(const std::string& HTTPMessage);

		HTTPParser(const std::vector<char>& HTTPMessage);

		HTTPParser(const HTTPParser& other);

		HTTPParser(HTTPParser&& other) noexcept;

		HTTPParser& operator = (const HTTPParser& other);

		HTTPParser& operator = (HTTPParser&& other) noexcept;

		void parse(std::string_view HTTPMessage);

		const std::string& getMethod() const;

		const std::string& getHTTPVersion() const;

		const std::string& getParameters() const;

		const std::unordered_map<std::string, std::string>& getKeyValueParameters() const;

		const std::pair<responseCodes, std::string>& getFullResponse() const;

		responseCodes getResponseCode() const;

		const std::string& getResponseMessage() const;

		const std::unordered_map<std::string, std::string, insensitiveStringHash, insensitiveStringEqual>& getHeaders() const;

		const std::string& getBody() const;

		const std::vector<std::string>& getChunks() const;

		const json::JSONParser& getJSON() const;

		/// @brief Set HTTP to output stream
		/// @param outputStream std::ostream subclass instance
		/// @param parser const reference to HTTPParser instance
		/// @return outputStream
		friend HTTP_API std::ostream& operator << (std::ostream& outputStream, const HTTPParser& parser);

		friend HTTP_API std::istream& operator >> (std::istream& inputStream, HTTPParser& parser);

		~HTTPParser() = default;
	};
}
