#pragma once

#include "HTTPUtility.h"

namespace web::exceptions
{
	/**
	 * @brief Parsing HTTP exception
	 */
	class HTTP_API HTTPParseException : public std::runtime_error
	{
	public:
		HTTPParseException(std::string_view message);

		~HTTPParseException() = default;
	};
}
