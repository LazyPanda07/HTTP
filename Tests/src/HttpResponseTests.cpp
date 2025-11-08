#include <gtest/gtest.h>

#include "HttpUtility.h"

TEST(ResponseCodes, GetMessageFromCode)
{
	ASSERT_EQ(web::getMessageFromCode(200), "OK");
	ASSERT_EQ(web::getMessageFromCode(web::ResponseCodes::ok), "OK");

	ASSERT_EQ(web::getMessageFromCode(1000), "Unknown response code");
}
