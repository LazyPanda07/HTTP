#include "HttpParserException.h"

namespace web::exceptions
{
	HttpParserException::HttpParserException(std::string_view message) :
		runtime_error(message.data())
	{
		
	}
}
