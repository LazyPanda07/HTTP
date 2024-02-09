#include "HTTPTestUtils.h"

std::string getGetRequest()
{
	return "GET /search?q=test HTTP/2\r\nHost: www.bing.com\r\nUser-Agent: curl/7.54.0\r\nAccept: */*\r\n\r\n";
}

std::string getPostRequest()
{
	using namespace std::string_literals;

	return "POST /post HTTP/1.1\r\nHost: httpbin.org\r\nConnection: close\r\nAccept: */*\r\nUser-Agent: Mozilla/4.0 (compatible; esp8266 Lua; Windows NT 5.1)\r\nContent-Type: application/json\r\nContent-Length: 96\r\n\r\n"s + 
R"({
  "stringValue": "qwe",
  "intValue": 1500,
  "doubleValue": 228.322000,
  "nullValue": null
})";
}

std::string getHTTPResponse()
{
  return "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: *\r\n"
  "Connection: Keep-Alive\r\nContent-Encoding: gzip\r\nContent-Type: text/html; charset=utf-8\r\n"
  "Date: Wed, 10 Aug, 2016 13:17:18 GMT\r\n"
  "Keep-Alive: timeout=5, max=999\r\n"
  "Server: Apache\r\n"
  "X-Frame-Options: DENY\r\n\r\n";
}
