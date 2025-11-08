#pragma once

#include "HttpUtility.h"

namespace web::exceptions
{
	/**
	 * @brief Parsing HTTP exception
	 */
	class HTTP_API HttpParserException : public std::runtime_error
	{
	public:
		HttpParserException(std::string_view message);

		~HttpParserException() = default;
	};
}
