#include "HTTPTestUtils.h"

#include <fstream>
#include <sstream>

std::string loadGetRequest()
{
	return "GET /search?q=test HTTP/2\r\nHost: www.bing.com\r\nUser-Agent: curl/7.54.0\r\nAccept: */*\r\n\r\n";
}

std::string loadPostRequest()
{
	return "POST /post HTTP/1.1\r\nHost: httpbin.org\r\nConnection: close\r\nAccept: */*\r\nUser-Agent: Mozilla/4.0 (compatible; esp8266 Lua; Windows NT 5.1)\r\nContent-Length: 96\r\n\r\n
{
  "stringValue": "qwe",
  "intValue": 1500,
  "doubleValue": 228.322000,
  "nullValue": null
}";
}
