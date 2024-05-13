#include "gtest/gtest.h"

#include "HTTPBuilder.h"

#include "HTTPTestUtils.h"

TEST(Builder, GET)
{
	std::string getRequest = web::HTTPBuilder("HTTP/2")
		.getRequest()
		.parameters("search?").parameters("q", "test")
		.headers
		(
			"Host", "www.bing.com",
			"User-Agent", "curl/7.54.0",
			"Accept", "*/*"
		).build();
	static constexpr size_t npos = std::string::npos;

	ASSERT_EQ(getRequest.size(), getGetRequest().size());

	ASSERT_NE(getRequest.find("GET /search?q=test HTTP/2"), npos);
	ASSERT_NE(getRequest.find("Host: www.bing.com"), npos);
	ASSERT_NE(getRequest.find("User-Agent: curl/7.54.0"), npos);
	ASSERT_NE(getRequest.find("Accept: */*"), npos);
	ASSERT_NE(getRequest.find("\r\n\r\n"), npos);
}

TEST(Builder, POST)
{
	json::JSONBuilder json(CP_UTF8);

	json["stringValue"] = "qwe";
	json["intValue"] = 1500LL;
	json["doubleValue"] = 228.322;
	json["nullValue"] = nullptr;

	std::string postRequest = web::HTTPBuilder()
		.postRequest()
		.parameters("post")
		.headers
		(
			"Host", "httpbin.org",
			"Connection", "close",
			"Accept", "*/*",
			"User-Agent", "Mozilla/4.0 (compatible; esp8266 Lua; Windows NT 5.1)"
		)
		.build(json);
	static constexpr size_t npos = std::string::npos;

	ASSERT_EQ(postRequest.size(), getPostRequest().size());

	ASSERT_NE(postRequest.find("Host: httpbin.org"), npos);
	ASSERT_NE(postRequest.find("Connection: close"), npos);
	ASSERT_NE(postRequest.find("Accept: */*"), npos);
	ASSERT_NE(postRequest.find("User-Agent: Mozilla/4.0 (compatible; esp8266 Lua; Windows NT 5.1)"), npos);
	ASSERT_NE(postRequest.find("User-Agent: Mozilla/4.0 (compatible; esp8266 Lua; Windows NT 5.1)"), npos);
	ASSERT_NE(postRequest.find("Content-Length: 96"), npos);
	ASSERT_NE(postRequest.find("Content-Type: application/json"), npos);
	ASSERT_NE(postRequest.find(getPostRequestJSON()), npos);
}

TEST(Builder, CONNECT)
{
	std::string connectRequest = web::HTTPBuilder().
		connectRequest().
		parameters("server.example.com:80").
		headers
		(
			"Host", "server.example.com:80",
			"Proxy-Authorization", "basic aGVsbG86d29ybGQ="
		).
		build();
	static constexpr size_t npos = std::string::npos;

	ASSERT_NE(connectRequest.find("CONNECT server.example.com:80 HTTP/1.1"), npos);
	ASSERT_NE(connectRequest.find("Host: server.example.com:80"), npos);
	ASSERT_NE(connectRequest.find("Proxy-Authorization: basic aGVsbG86d29ybGQ="), npos);
}

TEST(Builder, Streams)
{
	web::HTTPBuilder builder = web::HTTPBuilder("HTTP/2")
		.getRequest()
		.parameters("search?").parameters("q", "test")
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
	ASSERT_THROW(web::HTTPBuilder("HTTP.1/0"), std::runtime_error);
}
