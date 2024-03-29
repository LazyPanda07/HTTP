#include "gtest/gtest.h"

#include "HTTPParser.h"

#include "HTTPTestUtils.h"

TEST(Parser, Request)
{
	web::HTTPParser parser(getPostRequest());
	const json::JSONParser& jsonParser = parser.getJSON();
	const web::HeadersMap& headers = parser.getHeaders();

	ASSERT_EQ(parser.getMethod(), "POST");
	ASSERT_EQ(parser.getParameters(), "post");

	ASSERT_EQ(headers.at("Host"), "httpbin.org");
	ASSERT_EQ(headers.at("Connection"), "close");
	ASSERT_EQ(headers.at("Accept"), "*/*");
	ASSERT_EQ(headers.at("User-Agent"), "Mozilla/4.0 (compatible; esp8266 Lua; Windows NT 5.1)");
	ASSERT_EQ(headers.at("Content-Length"), "96");

	ASSERT_EQ(jsonParser.getString("stringValue"), "qwe");
	ASSERT_EQ(jsonParser.getInt("intValue"), 1500);
	ASSERT_EQ(jsonParser.getDouble("doubleValue"), 228.322);
	ASSERT_EQ(jsonParser.getNull("nullValue"), nullptr);
}

TEST(Parser, Response)
{
	web::HTTPParser parser(getHTTPResponse());
	const web::HeadersMap& headers = parser.getHeaders();

	ASSERT_EQ(parser.getHTTPVersion(), 1.1);
	ASSERT_EQ(parser.getResponseCode(), web::responseCodes::ok);
	ASSERT_EQ(parser.getResponseMessage(), "OK");
	ASSERT_EQ(headers.at("Connection"), "Keep-Alive");
	ASSERT_EQ(headers.at("Access-Control-Allow-Origin"), "*");
	ASSERT_EQ(headers.at("Content-Encoding"), "gzip");
	ASSERT_EQ(headers.at("Content-Type"), "text/html; charset=utf-8");
	ASSERT_EQ(headers.at("Date"), "Wed, 10 Aug, 2016 13:17:18 GMT");
	ASSERT_EQ(headers.at("Keep-Alive"), "timeout=5, max=999");
	ASSERT_EQ(headers.at("Server"), "Apache");
	ASSERT_EQ(headers.at("X-Frame-Options"), "DENY");
}
