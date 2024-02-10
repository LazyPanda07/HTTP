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
	size_t npos = std::string::npos;

	ASSERT_EQ(getRequest.size(), getGetRequest().size());

	ASSERT_TRUE(getRequest.find("GET /search?q=test HTTP/2") != npos);
	ASSERT_TRUE(getRequest.find("Host: www.bing.com") != npos);
	ASSERT_TRUE(getRequest.find("User-Agent: curl/7.54.0") != npos);
	ASSERT_TRUE(getRequest.find("Accept: */*") != npos);
	ASSERT_TRUE(getRequest.find("\r\n\r\n") != npos);
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
	size_t npos = std::string::npos;

	ASSERT_EQ(postRequest.size(), getPostRequest().size());

	ASSERT_TRUE(postRequest.find("Host: httpbin.org") != npos);
	ASSERT_TRUE(postRequest.find("Connection: close") != npos);
	ASSERT_TRUE(postRequest.find("Accept: */*") != npos);
	ASSERT_TRUE(postRequest.find("User-Agent: Mozilla/4.0 (compatible; esp8266 Lua; Windows NT 5.1)") != npos);
	ASSERT_TRUE(postRequest.find("User-Agent: Mozilla/4.0 (compatible; esp8266 Lua; Windows NT 5.1)") != npos);
	ASSERT_TRUE(postRequest.find("Content-Length: 96") != npos);
	ASSERT_TRUE(postRequest.find("Content-Type: application/json") != npos);
	ASSERT_TRUE(postRequest.find(getPostRequestJSON()) != npos);
}
