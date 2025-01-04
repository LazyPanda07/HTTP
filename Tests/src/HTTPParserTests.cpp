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
	ASSERT_EQ(parser.getResponseCode(), web::ResponseCodes::ok);
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

TEST(Parser, CONNECT)
{
	web::HTTPParser parser(getCONNECTRequest());
	const web::HeadersMap& headers = parser.getHeaders();

	ASSERT_EQ(parser.getParameters(), "server.example.com:80");

	ASSERT_EQ(headers.at("Host"), "server.example.com:80");
	ASSERT_EQ(headers.at("Proxy-Authorization"), "basic aGVsbG86d29ybGQ=");
}

TEST(Parser, Streams)
{
	static constexpr size_t npos = std::string::npos;

	{
		std::string data(getCONNECTRequest());
		web::HTTPParser parser;
		std::ostringstream os;
		std::istringstream is(data);

		is >> parser;

		os << parser;

		data = os.str();

		ASSERT_NE(data.find("CONNECT server.example.com:80 HTTP/1.1\r\n"), npos);
		ASSERT_NE(data.find("Host: server.example.com:80\r\n"), npos);
		ASSERT_NE(data.find("Proxy-Authorization: basic aGVsbG86d29ybGQ=\r\n"), npos);
	}

	{
		std::string data(getGetRequest());
		web::HTTPParser parser;
		std::ostringstream os;
		std::istringstream is(data);

		is >> parser;

		os << parser;

		data = os.str();

		ASSERT_NE(data.find("GET /search?q=test HTTP/2\r\n"), npos);
		ASSERT_NE(data.find("Host: www.bing.com\r\n"), npos);
		ASSERT_NE(data.find("User-Agent: curl/7.54.0\r\n"), npos);
		ASSERT_NE(data.find("Accept: */*\r\n"), npos);
	}
}

TEST(Parser, Parameters)
{
	ASSERT_EQ(web::HTTPParser(getGetRequest()).getKeyValueParameters().at("q"), "test");
}
