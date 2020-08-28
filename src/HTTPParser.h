#pragma once

#include <unordered_map>
#include <string>
#include <vector>

namespace web
{
	class HTTPParser
	{
	private:
		std::string method;
		std::string httpVersion;
		std::string parameters;
		std::pair<std::string, std::string> response;	//code - response message

		std::unordered_map<std::string, std::string> headers;
		std::string body;

	private:
		void parsing(std::string_view&& HTTPMessage);

	public:
		HTTPParser(const std::string& HTTPMessage);

		HTTPParser(const std::vector<char>& HTTPMessage);

		const std::string& getMethod() const;

		const std::string& getHTTPVersion() const;

		const std::string& getParameters() const;

		const std::pair<std::string, std::string>& getResponse() const;

		const std::unordered_map<std::string, std::string>& getHeaders() const;

		const std::string& getBody() const;

		~HTTPParser() = default;
	};
}