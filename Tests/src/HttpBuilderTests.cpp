#include <gtest/gtest.h>

#include "HttpBuilder.h"
#include "HttpParser.h"

#include "HttpTestUtils.h"

TEST(Builder, GET)
{
	std::string getRequest = web::HttpBuilder()
		.getRequest()
		.parameters("search?").queryParameters("q", "test")
		.headers
		(
			"Host", "www.bing.com",
			"User-Agent", "curl/7.54.0",
			"Accept", "*/*"
		).build();

	ASSERT_EQ(getRequest.size(), getGetRequest().size()) << getRequest << std::endl;

	ASSERT_NE(getRequest.find("GET /search?q=test HTTP/1.1"), std::string::npos);
	ASSERT_NE(getRequest.find("Host: www.bing.com"), std::string::npos);
	ASSERT_NE(getRequest.find("User-Agent: curl/7.54.0"), std::string::npos);
	ASSERT_NE(getRequest.find("Accept: */*"), std::string::npos);
	ASSERT_NE(getRequest.find("\r\n\r\n"), std::string::npos);
}

TEST(Builder, POST)
{
	json::JsonBuilder json(CP_UTF8);

	json["stringValue"] = "qwe";
	json["intValue"] = 1500LL;
	json["doubleValue"] = 228.322;
	json["nullValue"] = nullptr;

	std::string postRequest = web::HttpBuilder()
		.postRequest()
		.parameters("post")
		.headers
		(
			"Host", "httpbin.org",
			"Connection", "close",
			"Accept", "*/*",
			"User-Agent", "Mozilla/4.0 (compatible; esp8266 Lua; Windows NT 5.1)",
			"Empty-Header", ""
		)
		.build(json);
	json::JsonParser parser = web::HttpParser(postRequest).getJson();

	ASSERT_EQ(postRequest.size(), getPostRequest().size());

	ASSERT_NE(postRequest.find("Host: httpbin.org"), std::string::npos);
	ASSERT_NE(postRequest.find("Connection: close"), std::string::npos);
	ASSERT_NE(postRequest.find("Accept: */*"), std::string::npos);
	ASSERT_NE(postRequest.find("User-Agent: Mozilla/4.0 (compatible; esp8266 Lua; Windows NT 5.1)"), std::string::npos);
	ASSERT_NE(postRequest.find("User-Agent: Mozilla/4.0 (compatible; esp8266 Lua; Windows NT 5.1)"), std::string::npos);
	ASSERT_NE(postRequest.find("Content-Length: 96"), std::string::npos);
	ASSERT_NE(postRequest.find("Content-Type: application/json"), std::string::npos);

	ASSERT_EQ(parser.get<std::string>("stringValue"), "qwe");
	ASSERT_EQ(parser.get<int>("intValue"), 1500);
	ASSERT_EQ(parser.get<double>("doubleValue"), 228.322000);
	ASSERT_EQ(parser.get<std::nullptr_t>("nullValue"), nullptr);
}

TEST(Builder, CONNECT)
{
	std::string connectRequest = web::HttpBuilder().
		connectRequest().
		parameters("server.example.com:80").
		headers
		(
			"Host", "server.example.com:80",
			"Proxy-Authorization", "basic aGVsbG86d29ybGQ="
		).
		build();
	
	ASSERT_NE(connectRequest.find("CONNECT server.example.com:80 HTTP/1.1"), std::string::npos);
	ASSERT_NE(connectRequest.find("Host: server.example.com:80"), std::string::npos);
	ASSERT_NE(connectRequest.find("Proxy-Authorization: basic aGVsbG86d29ybGQ="), std::string::npos);
}

TEST(Builder, Streams)
{
	web::HttpBuilder builder = web::HttpBuilder()
		.getRequest()
		.parameters("search?").queryParameters("q", "test")
		.headers
		(
			"Host", "www.bing.com",
			"User-Agent", "curl/7.54.0",
			"Accept", "*/*"
		);
	std::string getRequest = builder.build();
	std::ostringstream os;

	os << builder;

	ASSERT_EQ(getRequest, os.str());
}

TEST(Builder, WrongHTTPVersion)
{
	ASSERT_THROW(web::HttpBuilder("HTTP.1/0"), std::runtime_error);
}
