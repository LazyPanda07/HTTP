#pragma once

#include <vector>
#include <unordered_map>
#include <functional>

#include "HTTPUtility.h"
#include "JsonParser.h"

namespace web
{
	/// @brief HTTP parser
	class HTTP_API HttpParser
	{
	private:
		struct ReadOnlyBuffer : public std::streambuf
		{
			ReadOnlyBuffer(std::string_view view);
		};

	private:
		static const std::unordered_map<std::string_view, std::function<void(HttpParser&, std::string_view)>> contentTypeParsers;

	public:
		static inline const std::string contentLengthHeader = "Content-Length";
		static inline const std::string contentTypeHeader = "Content-Type";
		static inline const std::string transferEncodingHeader = "Transfer-Encoding";
		static inline const std::string utf8Encoded = "charset=utf-8";
		static inline const std::string chunkEncoded = "chunked";
		static inline constexpr std::string_view crlfcrlf = "\r\n\r\n";

	public:
		static inline constexpr std::string_view urlEncoded = "application/x-www-form-urlencoded";
		static inline constexpr std::string_view jsonEncoded = "application/json";
		static inline constexpr std::string_view multipartEncoded = "multipart/form-data";

	private:
		std::unordered_map<std::string, std::string, InsensitiveStringHash, InsensitiveStringEqual> headers;
		std::vector<Multipart> multiparts;
		std::unordered_map<std::string, std::string> queryParameters;
		json::JsonParser jsonParser;
		std::pair<int, std::string> response;	// code - response message
		std::string method;
		std::string httpVersion;
		std::string parameters;
		std::string body;
		std::vector<std::string> chunks;
		std::string rawData;
		size_t chunksSize;
		bool parsed;

	private:
		std::string mergeChunks() const;

	private:
		void parseQueryParameter(std::string_view rawParameters);

		void parseMultipart(std::string_view data);

		void parseContentType();

	private:
		void parseChunkEncoded(std::string_view HTTPMessage, bool isUTF8);

	public:
		HttpParser();

		HttpParser(const std::string& HTTPMessage);

		HttpParser(const std::vector<char>& HTTPMessage);

		HttpParser(const HttpParser& other) = default;

		HttpParser(HttpParser&& other) noexcept = default;

		HttpParser& operator = (const HttpParser& other) = default;

		HttpParser& operator = (HttpParser&& other) noexcept = default;

		void parse(std::string_view HTTPMessage);

		const std::string& getMethod() const;

		double getHTTPVersion() const;

		const std::string& getParameters() const;

		const std::unordered_map<std::string, std::string>& getQueryParameters() const;

		const std::pair<int, std::string>& getFullResponse() const;

		int getResponseCode() const;

		const std::string& getResponseMessage() const;

		const HeadersMap& getHeaders() const;

		const std::string& getBody() const;

		const std::vector<std::string>& getChunks() const;

		const json::JsonParser& getJson() const;
		
		const std::string& getRawData() const;

		const std::vector<Multipart>& getMultiparts() const;

		operator bool() const;

		friend HTTP_API std::ostream& operator << (std::ostream& outputStream, const HttpParser& parser);

		friend HTTP_API std::istream& operator >> (std::istream& inputStream, HttpParser& parser);

		~HttpParser() = default;
	};
}
