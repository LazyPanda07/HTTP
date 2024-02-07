#include "gtest/gtest.h"

#include "HTTPParser.h"

#include "HTTPTestUtils.h"

TEST(Parser, Request)
{
    
}

TEST(Parser, Response)
{
	web::HTTPParser parser(getHTTPResponse());

    const auto& headers = parser.getHeaders();

    ASSERT_EQ(parser.getHTTPVersion(), "1.1");
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
