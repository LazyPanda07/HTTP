#pragma once

#ifdef HTTP_DLL
#define HTTP_API __declspec(dllexport)
#define JSON_DLL
#else
#define HTTP_API
#endif // HTTP_DLL

#include <string>
#include <stdexcept>

#include "CheckAtCompileTime.h"
#include "JSONBuilder.h"
#include "HTTPUtility.h"

#pragma comment (lib, "JSON.lib")

namespace web
{
	class HTTP_API HTTPBuilder
	{
	private:
		HTTPBuilder& parameters();

	private:
		std::string method;
		std::string _parameters;
		std::string _responseCode;
		std::string _headers;
		std::string _HTTPVersion;

	public:
		HTTPBuilder();

		HTTPBuilder& getRequest();

		HTTPBuilder& postRequest();

		HTTPBuilder& putRequest();

		HTTPBuilder& headRequest();

		HTTPBuilder& optionsRequest();

		HTTPBuilder& deleteRequest();

		HTTPBuilder& connectRequest();

		HTTPBuilder& traceRequest();

		HTTPBuilder& patchRequest();

		template<typename StringT, typename T, typename... Args>
		HTTPBuilder& parameters(StringT&& name, T&& value, Args&&... args);

		HTTPBuilder& parameters(const std::string& parameters);

		HTTPBuilder& responseCode(ResponseCodes code);

		HTTPBuilder& HTTPVersion(const std::string& httpVersion);

		template<typename StringT, typename T, typename... Args>
		HTTPBuilder& headers(StringT&& name, T&& value, Args&&... args);

		std::string build(const std::string* const data = nullptr);

		std::string build(const json::JSONBuilder& builder);

		HTTPBuilder& clear();

		~HTTPBuilder() = default;
	};

	template<typename StringT, typename T, typename... Args>
	HTTPBuilder& HTTPBuilder::parameters(StringT&& name, T&& value, Args&&... args)
	{
		if (_parameters.empty())
		{
			_parameters = "/?";
		}

		if constexpr (utility::StringConversion<StringT>::value)
		{
			if constexpr (std::is_arithmetic_v<T>)
			{
				_parameters += static_cast<std::string>(name) + std::string("=") + std::to_string(value) + std::string("&");
			}
			else if constexpr (utility::StringConversion<T>::value)
			{
				_parameters += static_cast<std::string>(name) + std::string("=") + static_cast<std::string>(value) + std::string("&");
			}
			else
			{
				throw std::logic_error("Bad type of T, it must be converted to string or arithmetic type");
			}
		}
		else
		{
			throw std::logic_error("Bad type of StringT, it must be converted to string");
		}

		return parameters(std::forward<Args>(args)...);
	}

	template<typename StringT, typename T, typename... Args>
	HTTPBuilder& HTTPBuilder::headers(StringT&& name, T&& value, Args&&... args)
	{
		if constexpr (utility::StringConversion<StringT>::value)
		{
			if constexpr (sizeof...(args))
			{
				if (static_cast<std::string>(name) == "Content-Length" || static_cast<std::string>(name) == "content-length")
				{
					return headers(std::forward<Args>(args)...);
				}
			}

			if constexpr (std::is_arithmetic_v<std::remove_reference_t<T>>)
			{
				_headers += static_cast<std::string>(name) + std::string(": ") + std::to_string(value) + std::string("\r\n");
			}
			else if constexpr (utility::StringConversion<std::remove_reference_t<T>>::value)
			{
				_headers += static_cast<std::string>(name) + std::string(": ") + static_cast<std::string>(value) + std::string("\r\n");
			}
			else if constexpr (utility::checkBegin<T>::value && utility::checkEnd<T>::value)
			{
				_headers += static_cast<std::string>(name) + std::string(": ");

				auto checkValueType = *std::begin(value);

				if constexpr (std::is_arithmetic_v<decltype(checkValueType)>)
				{
					for (auto&& i : value)
					{
						_headers += std::to_string(i) + std::string(", ");
					}

					_headers.pop_back();	// delete space
					_headers.pop_back();	// delete ;

					_headers.insert(_headers.size(), "\r\n");
				}
				else if constexpr (utility::StringConversion<decltype(checkValueType)>::value)
				{
					for (auto&& i : value)
					{
						_headers += static_cast<std::string>(i) + std::string(", ");
					}

					_headers.pop_back();	// delete space
					_headers.pop_back();	// delete ;

					_headers.insert(_headers.size(), "\r\n");
				}
				else
				{
					throw std::logic_error("Bad type of values in class T, it must be converted to string or arithmetic type");
				}
			}
			else
			{
				throw std::logic_error("Bad type of T, it must be converted to string or arithmetic type");
			}
		}
		else
		{
			throw std::logic_error("Bad type of StringT, it must be converted to string");
		}

		if constexpr (sizeof...(args))
		{
			return headers(std::forward<Args>(args)...);
		}

		return *this;
	}
}
