#pragma once

#include <string>
#include <stdexcept>
#include <format>

#include "HTTPUtility.h"
#include "CheckAtCompileTime.h"
#include "JSONBuilder.h"

namespace web
{
	/// @brief HTTP builder
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
		std::vector<std::string> _chunks;

	public:
		/// @brief Make HTTP parsed data with zero chunk
		/// @param chunks Data to convert
		/// @param preCalculateSize Pre allocate result string size(requires additional pass)
		/// @return 
		static std::string getChunks(const std::vector<std::string>& chunks, bool preCalculateSize = false);

		/// @brief Make HTTP parsed chunk
		/// @param chunk 
		/// @return 
		static std::string getChunk(std::string_view chunk);

	public:
		HTTPBuilder(std::string_view fullHTTPVersion = "HTTP/1.1");

		HTTPBuilder(const HTTPBuilder& other) = default;

		HTTPBuilder(HTTPBuilder&& other) noexcept = default;

		HTTPBuilder& operator = (const HTTPBuilder& other) = default;

		HTTPBuilder& operator = (HTTPBuilder&& other) noexcept = default;

		/// @brief Set GET request
		/// @return Self
		HTTPBuilder& getRequest();

		/// @brief Set POST request
		/// @return Self
		HTTPBuilder& postRequest();

		/// @brief Set PUT request
		/// @return Self
		HTTPBuilder& putRequest();

		/// @brief Set HEAD request
		/// @return Self
		HTTPBuilder& headRequest();

		/// @brief Set OPTIONS request
		/// @return Self
		HTTPBuilder& optionsRequest();

		/// @brief Set DELETE request
		/// @return Self
		HTTPBuilder& deleteRequest();

		/// @brief Set CONNECT request
		/// @return Self
		HTTPBuilder& connectRequest();

		/// @brief Set TRACE request
		/// @return Self
		HTTPBuilder& traceRequest();

		/// @brief Set PATCH request
		/// @return Self
		HTTPBuilder& patchRequest();

		/// @brief Append key - value parameters
		/// @tparam StringT 
		/// @tparam T 
		/// @tparam ...Args 
		/// @param name 
		/// @param value 
		/// @param ...args 
		/// @return Self
		template<typename StringT, typename T, typename... Args>
		HTTPBuilder& parameters(StringT&& name, T&& value, Args&&... args);

		template<typename... Args>
		HTTPBuilder& parametersWithRoute(std::string_view route, Args&&... args);

		/// @brief Set parameters
		/// @param parameters 
		/// @return Self
		HTTPBuilder& parameters(std::string_view parameters);

		HTTPBuilder& responseCode(responseCodes code);

		HTTPBuilder& responseCode(int code, std::string_view responseMessage);

		HTTPBuilder& HTTPVersion(std::string_view httpVersion);

		/// @brief Append header - value
		/// @tparam StringT 
		/// @tparam T 
		/// @tparam ...Args 
		/// @param name 
		/// @param value 
		/// @param ...args 
		/// @return Self
		template<typename StringT, typename T, typename... Args>
		HTTPBuilder& headers(StringT&& name, T&& value, Args&&... args);

		HTTPBuilder& chunks(const std::vector<std::string>& chunks);

		HTTPBuilder& chunk(std::string_view chunk);

		std::string build(std::string_view data = "", const std::unordered_map<std::string, std::string>& additionalHeaders = {}) const;

		std::string build(const json::JSONBuilder& builder, std::unordered_map<std::string, std::string> additionalHeaders = {}) const;

		std::string build(const std::unordered_map<std::string, std::string>& urlEncoded, std::unordered_map<std::string, std::string> additionalHeaders = {}) const;

		HTTPBuilder& clear();

		/// @brief Set HTTP to output stream
		/// @param outputStream std::ostream subclass instance
		/// @param parser const reference to HTTPBuilder instance
		/// @return outputStream
		friend HTTP_API std::ostream& operator << (std::ostream& outputStream, const HTTPBuilder& builder);

		~HTTPBuilder() = default;
	};

	template<typename StringT, typename T, typename... Args>
	HTTPBuilder& HTTPBuilder::parameters(StringT&& name, T&& value, Args&&... args)
	{
		static_assert(std::is_convertible_v<decltype(name), std::string_view>, "Wrong StringT type");

		if (_parameters.empty())
		{
			_parameters = "/?";
		}

		if constexpr (std::is_arithmetic_v<T>)
		{
			_parameters += std::format("{}={}&", web::encodeUrl(static_cast<std::string_view>(name)), std::to_string(value));
		}
		else if constexpr (std::is_convertible_v<decltype(value), std::string_view>)
		{
			_parameters += std::format("{}={}&", web::encodeUrl(static_cast<std::string_view>(name)), web::encodeUrl(static_cast<std::string_view>(value)));
		}
		else if constexpr (std::is_convertible_v<decltype(value), std::string>)
		{
			_parameters += std::format("{}={}&", web::encodeUrl(static_cast<std::string_view>(name)), web::encodeUrl(static_cast<std::string>(value)));
		}
		else
		{
			throw std::logic_error("Bad type of T, it must be converted to string or arithmetic type");
		}

		return this->parameters(std::forward<Args>(args)...);
	}

	template<typename... Args>
	HTTPBuilder& HTTPBuilder::parametersWithRoute(std::string_view route, Args&&... args)
	{
		if (route.starts_with('/'))
		{
			_parameters = route;
		}
		else
		{
			_parameters = "/";

			_parameters += route;
		}

		if (!route.ends_with('?'))
		{
			_parameters += '?';
		}

		return this->parameters(std::forward<Args>(args)...);
	}

	template<typename StringT, typename T, typename... Args>
	HTTPBuilder& HTTPBuilder::headers(StringT&& name, T&& value, Args&&... args)
	{
		if constexpr (::utility::StringConversion<StringT>::value)
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
			else if constexpr (::utility::StringConversion<std::remove_reference_t<T>>::value)
			{
				_headers += static_cast<std::string>(name) + std::string(": ") + static_cast<std::string>(value) + std::string("\r\n");
			}
			else if constexpr (::utility::checkBegin<T>::value && ::utility::checkEnd<T>::value)
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
				else if constexpr (::utility::StringConversion<decltype(checkValueType)>::value)
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
