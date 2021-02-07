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

#pragma comment (lib, "JSON.lib")

namespace web
{
	class HTTP_API HTTPParser
	{
	private:
		std::unordered_map<std::string, std::string> headers;
		std::unordered_map<std::string, std::string> keyValueParameters;
		std::pair<short, std::string> response;	//code - response message
		std::string method;
		std::string httpVersion;
		std::string parameters;
		std::string body;
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

		const std::pair<short, std::string>& getResponse() const;

		const std::unordered_map<std::string, std::string>& getHeaders() const;

		const std::string& getBody() const;

		const json::JSONParser& getJSON() const;

		~HTTPParser() = default;
	};
}