#pragma once

#include <map>
#include <string>
#include <vector>

namespace web
{
	class HTTPParser
	{
	private:
		std::string method;
		std::string httpVersion;
		std::map<std::string, std::string> parameters;
		std::pair<std::string, std::string> response;	//code - response message

		std::map<std::string, std::string> body;
		std::string data;

	public:
		HTTPParser(const std::string& HTTPMessage);

		const std::string& getMethod() const;

		const std::string& getHTTPVersion() const;

		const std::map<std::string, std::string>& getParameters() const;

		const std::pair<std::string, std::string>& getResponse() const;

		const std::map<std::string, std::string>& getBody() const;

		const std::string& getData() const;

		~HTTPParser() = default;
	};
}