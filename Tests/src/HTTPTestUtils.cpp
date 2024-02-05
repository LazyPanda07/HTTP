#include "HTTPTestUtils.h"

#include <fstream>
#include <sstream>
#include <format>

static std::string loadRequest(const std::string& requestFileName, bool binaryRead = true)
{
	std::ifstream data(std::format("data/{}", requestFileName), binaryRead ? std::ios::binary : std::ios::in);

	return (std::ostringstream() << data.rdbuf()).str();
}

std::string loadGetRequest()
{
	return loadRequest("get_request.txt");
}

std::string loadPostRequest()
{
	return loadRequest("post_request.txt") + loadRequest("post_request.json", false);
}
