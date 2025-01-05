#pragma once

#include <vector>
#include <unordered_map>
#include <functional>

#include "HTTPUtility.h"
#include "JSONParser.h"

namespace web
{
	/// @brief HTTP parser
	class HTTP_API HTTPParser
	{
	private:
		struct readOnlyBuffer : public std::streambuf
		{
			readOnlyBuffer(std::string_view view);
		};

	private:
		static const std::unordered_map<std::string_view, std::function<void(HTTPParser&, std::string_view)>> contentTypeParsers;

	public:
		static inline const std::string contentLengthHeader = "Content-Length";
		static inline const std::string contentTypeHeader = "Content-Type";
		static inline const std::string transferEncodingHeader = "Transfer-Encoding";
		static inline const std::string utf8Encoded = "charset=utf-8";
		static inline const std::string chunkEncoded = "chunked";
		static inline constexpr std::string_view crlfcrlf = "\r\n\r\n";
		static inline constexpr std::string_view crlf = "\r\n";

	public:
		static inline constexpr std::string_view urlEncoded = "application/x-www-form-urlencoded";
		static inline constexpr std::string_view jsonEncoded = "application/json";
		static inline constexpr std::string_view octetStreamEncoded = "application/octet-stream";
		static inline constexpr std::string_view multipartEncoded = "multipart/form-data";

	private:
		std::unordered_map<std::string, std::string, insensitiveStringHash, insensitiveStringEqual> headers;
		std::unordered_map<std::string, std::string> keyValueParameters;
		json::JSONParser jsonParser;
		std::pair<int, std::string> response;	// code - response message
		std::string method;
		std::string httpVersion;
		std::string parameters;
		std::string body;
		std::vector<std::string> chunks;
		std::string rawData;
		size_t chunksSize;

	private:
		std::string mergeChunks() const;

	private:
		void parseKeyValueParameter(std::string_view rawParameters);

		void parseContentType();

	private:
		void parseChunkEncoded(std::string_view HTTPMessage, bool isUTF8);

	public:
		HTTPParser() = default;

		HTTPParser(const std::string& HTTPMessage);

		HTTPParser(const std::vector<char>& HTTPMessage);

		HTTPParser(const HTTPParser& other) = default;

		HTTPParser(HTTPParser&& other) noexcept = default;

		HTTPParser& operator = (const HTTPParser& other) = default;

		HTTPParser& operator = (HTTPParser&& other) noexcept = default;

		void parse(std::string_view HTTPMessage);

		const std::string& getMethod() const;

		double getHTTPVersion() const;

		const std::string& getParameters() const;

		const std::unordered_map<std::string, std::string>& getKeyValueParameters() const;

		const std::pair<int, std::string>& getFullResponse() const;

		int getResponseCode() const;

		const std::string& getResponseMessage() const;

		const HeadersMap& getHeaders() const;

		const std::string& getBody() const;

		const std::vector<std::string>& getChunks() const;

		const json::JSONParser& getJSON() const;
		
		const std::string& getRawData() const;

		/// @brief Set HTTP to output stream
		/// @param outputStream std::ostream subclass instance
		/// @param parser const reference to HTTPParser instance
		/// @return outputStream
		friend HTTP_API std::ostream& operator << (std::ostream& outputStream, const HTTPParser& parser);

		friend HTTP_API std::istream& operator >> (std::istream& inputStream, HTTPParser& parser);

		~HTTPParser() = default;
	};
}
