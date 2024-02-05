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

	ASSERT_EQ(getRequest, loadGetRequest());
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

	ASSERT_EQ(postRequest, loadPostRequest());
}
