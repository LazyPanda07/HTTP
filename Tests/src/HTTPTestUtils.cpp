#include "HTTPTestUtils.h"

std::string getGetRequest()
{
	return
		"GET /search?q=test HTTP/2\r\n"
		"Host: www.bing.com\r\n"
		"User-Agent: curl/7.54.0\r\n"
		"Accept: */*\r\n"
		"\r\n";
}

std::string getPostRequest()
{
	using namespace std::string_literals;

	return
		"POST /post HTTP/1.1\r\n"
		"Host: httpbin.org\r\n"
		"Connection: close\r\n"
		"Accept: */*\r\n"
		"User-Agent: Mozilla/4.0 (compatible; esp8266 Lua; Windows NT 5.1)\r\n"
		"Content-Length: 96\r\n"
		"Content-Type: application/json\r\n"
		"\r\n"s +
		getPostRequestJSON();
}

std::string getPoseRequestWithoutSpaces()
{
	return
		"POST / HTTP/1.1\r\n"
		"Host:127.0.0.1:8080\r\n"
		"User-Agent:curl/8.10.1\r\n"
		"Accept:*/*\r\n"
		"Content-Type:application/octet-stream\r\n"
		"Content-Length:579959121\r\n"
		"Expect:100-continue\r\n"
		"\r\n";
}

std::string getPostRequestJSON()
{
	return R"({
  "stringValue": "qwe",
  "intValue": 1500,
  "doubleValue": 228.322000,
  "nullValue": null
})";
}

std::string getHTTPResponse()
{
	return
		"HTTP/1.1 200 OK\r\n"
		"Access-Control-Allow-Origin: *\r\n"
		"Connection: Keep-Alive\r\n"
		"Content-Encoding: gzip\r\n"
		"Content-Type: text/html; charset=utf-8\r\n"
		"Date: Wed, 10 Aug, 2016 13:17:18 GMT\r\n"
		"Keep-Alive: timeout=5, max=999\r\n"
		"Server: Apache\r\n"
		"X-Frame-Options: DENY\r\n"
		"\r\n";
}

std::string getCONNECTRequest()
{
	return
		"CONNECT server.example.com:80 HTTP/1.1\r\n"
		"Host: server.example.com:80\r\n"
		"Proxy-Authorization: basic aGVsbG86d29ybGQ=\r\n"
		"\r\n";
}

std::string getMultipartRequest()
{
	return
		"POST /upload HTTP/1.1\r\n"
		"Host: example.com\r\n"
		"Content-Type: multipart/form-data; boundary=----WebKitFormBoundary7MA4YWxkTrZu0gW\r\n"
		"Content-Length: 549\r\n"
		"\r\n"
		"------WebKitFormBoundary7MA4YWxkTrZu0gW\r\n"
		"Content-Disposition: form-data; name=\"field1\"\r\n"
		"\r\n"
		"value1\r\n"
		"------WebKitFormBoundary7MA4YWxkTrZu0gW\r\n"
		"Content-Disposition: form-data; name=\"field2\"\r\n"
		"\r\n"
		"value2\r\n"
		"------WebKitFormBoundary7MA4YWxkTrZu0gW\r\n"
		"Content-Disposition: form-data; name=\"file\"; filename=\"example.txt\"\r\n"
		"Content-Type: text/plain\r\n"
		"\r\n"
		"This is the content of the file being uploaded.\r\n"
		"------WebKitFormBoundary7MA4YWxkTrZu0gW--\r\n";
}
