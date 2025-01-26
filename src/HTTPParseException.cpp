#include "HTTPParseException.h"

using namespace std;

namespace web::exceptions
{
	HTTPParseException::HTTPParseException(string_view message) :
		runtime_error(string(message.begin(), message.end()))
	{
		
	}
}
