#include "HTTPParserException.h"

using namespace std;

namespace web::exceptions
{
	HttpParserException::HttpParserException(string_view message) :
		runtime_error(string(message.begin(), message.end()))
	{
		
	}
}
