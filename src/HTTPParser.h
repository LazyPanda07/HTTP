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
	class HTTP_API HTTPParser
	{
	public:
		struct insensitiveStringHash
		{
			size_t operator () (const std::string& value) const;
		};

		struct insensitiveStringEqual
		{
			bool operator () (const std::string& left, const std::string& right) const;
		};

	public:
		static const std::string contentLengthHeader;
		static const std::string contentTypeHeader;
		static const std::string transferEncodingHeader;
		static const std::string utf8Encoded;
		static const std::string chunkEncoded;
		static constexpr std::string_view crlfcrlf = "\r\n\r\n";
		static constexpr std::string_view crlf = "\r\n";
		static constexpr std::string_view urlEncoded = "application/x-www-form-urlencoded";
		static constexpr std::string_view jsonEncoded = "application/json";

	private:
		std::unordered_map<std::string, std::string, insensitiveStringHash, insensitiveStringEqual> headers;
		std::unordered_map<std::string, std::string> keyValueParameters;
		std::pair<ResponseCodes, std::string> response;	//code - response message
		std::string method;
		std::string httpVersion;
		std::string parameters;
		std::string body;
		std::vector<std::string> chunks;
		json::JSONParser jsonParser;

	private:
		void parseKeyValueParameter(std::string_view rawParameters);

		void parse(std::string_view&& HTTPMessage);

	public:
		HTTPParser(const std::string& HTTPMessage);

		HTTPParser(const std::vector<char>& HTTPMessage);

		const std::string& getMethod() const;

		const std::string& getHTTPVersion() const;

		const std::string& getParameters() const;

		const std::unordered_map<std::string, std::string>& getKeyValueParameters() const;

		const std::pair<ResponseCodes, std::string>& getFullResponse() const;

		ResponseCodes getResponseCode() const;

		std::string getResponseMessage() const;

		const std::unordered_map<std::string, std::string, insensitiveStringHash, insensitiveStringEqual>& getHeaders() const;

		const std::string& getBody() const;

		const std::vector<std::string>& getChunks() const;

		const json::JSONParser& getJSON() const;

		~HTTPParser() = default;
	};
}