#pragma once

#include <string>
#include <stdexcept>
#include <format>
#include <concepts>

#include "HttpUtility.h"
#include "CheckAtCompileTime.h"
#include "JsonBuilder.h"

namespace web
{
	/// @brief HTTP builder
	class HTTP_API HttpBuilder
	{
	public:
		/// @brief Make HTTP parsed data with zero chunk
		/// @param chunks Data to convert
		/// @param partialChunks If true does not append 0 at the end
		/// @param preCalculateSize Pre allocate result string size(requires additional pass)
		/// @return 
		static std::string getChunks(const std::vector<std::string>& chunks, bool partialChunks, bool preCalculateSize = false);

		/// @brief Make HTTP parsed chunk
		/// @param chunk 
		/// @return 
		static std::string getChunk(std::string_view chunk);


	private:
		template<typename KeyT, typename ValueT, typename... Args>
		static void fillAdditionalHeaders(std::unordered_map<std::string, std::string>& additionalHeaders, KeyT&& key, ValueT&& value, Args&&... args);

		template<concepts::HttpBuilderReturnType T>
		static T convert(std::string&& result);

	private:
		template<typename... AdditionalHeaders>
		std::string buildImplementation(std::string_view data, const std::unordered_map<std::string, std::string>& headers, AdditionalHeaders&&... keyValueAdditionalHeaders) const;

	private:
		std::string method;
		std::string _parameters;
		std::string _responseCode;
		std::string _headers;
		std::string _HTTPVersion;
		std::vector<std::string> _chunks;
		bool _partialChunks;

	public:
		HttpBuilder(std::string_view fullHTTPVersion = "HTTP/1.1");

		HttpBuilder(const HttpBuilder& other) = default;

		HttpBuilder(HttpBuilder&& other) noexcept = default;

		HttpBuilder& operator = (const HttpBuilder& other) = default;

		HttpBuilder& operator = (HttpBuilder&& other) noexcept = default;

		/// @brief Set GET request
		/// @return Self
		HttpBuilder& getRequest();

		/// @brief Set POST request
		/// @return Self
		HttpBuilder& postRequest();

		/// @brief Set PUT request
		/// @return Self
		HttpBuilder& putRequest();

		/// @brief Set HEAD request
		/// @return Self
		HttpBuilder& headRequest();

		/// @brief Set OPTIONS request
		/// @return Self
		HttpBuilder& optionsRequest();

		/// @brief Set DELETE request
		/// @return Self
		HttpBuilder& deleteRequest();

		/// @brief Set CONNECT request
		/// @return Self
		HttpBuilder& connectRequest();

		/// @brief Set TRACE request
		/// @return Self
		HttpBuilder& traceRequest();

		/// @brief Set PATCH request
		/// @return Self
		HttpBuilder& patchRequest();

		/// @brief Append key - value parameters
		/// @tparam StringT 
		/// @tparam T 
		/// @tparam ...Args 
		/// @param name 
		/// @param value 
		/// @param ...args 
		/// @return Self
		template<typename StringT, typename T, typename... Args>
		HttpBuilder& queryParameters(StringT&& name, T&& value, Args&&... args);

		template<typename... Args>
		HttpBuilder& parametersWithRoute(std::string_view route, Args&&... args);

		/// @brief Set parameters
		/// @param parameters 
		/// @return Self
		HttpBuilder& parameters(std::string_view parameters);

		/**
		 * @brief Sets the HTTP response code on the builder.
		 * @param code The response code to set (a value of type ResponseCodes).
		 * @return A reference to the same HttpBuilder instance, allowing method chaining.
		 */
		HttpBuilder& responseCode(ResponseCodes code);

		/**
		 * @brief Sets the HTTP response status code and its reason/message on the builder.
		 * @param code The HTTP status code to set (e.g., 200 for OK).
		 * @param responseMessage The reason phrase or response message associated with the status code (provided as a std::string_view).
		 * @return Reference to the same HttpBuilder instance, allowing method chaining.
		 */
		HttpBuilder& responseCode(int code, std::string_view responseMessage);

		/**
		 * @brief Sets the HTTP protocol version to use for the request being built.
		 * @param httpVersion The HTTP version string to set (valid versions: HTTP/0.9, HTTP/1.0, HTTP/1.1).
		 * @return Reference to the same HttpBuilder instance to allow method chaining.
		 */
		HttpBuilder& HTTPVersion(std::string_view httpVersion);

		/**
		 * @brief Sets the chunk data to include in the HTTP message.
		 * @param chunks A vector of strings where each element is a chunk of data to include in the HTTP message.
		 * @return A reference to the HttpBuilder instance, allowing method chaining.
		 */
		HttpBuilder& chunks(const std::vector<std::string>& chunks);

		/**
		 * @brief Sets the HTTP message chunks by taking ownership of the provided vector.
		 * @param chunks An rvalue reference to a vector of string chunks; the vector is moved into the builder (ownership transferred).
		 * @return A reference to this HttpBuilder instance to allow method chaining.
		 */
		HttpBuilder& chunks(std::vector<std::string>&& chunks);

		/**
		 * @brief Appends a data chunk to the HTTP message being built.
		 * @param chunk A std::string_view referencing the data to append as a chunk.
		 * @return A reference to the HttpBuilder instance, enabling method chaining.
		 */
		HttpBuilder& chunk(std::string_view chunk);

		/**
		 * @brief Clears the builder's internal state and allows method chaining.
		 * @return A reference to the HttpBuilder (typically *this) after its internal state has been cleared, enabling chaining of calls.
		 */
		HttpBuilder& clear();

		/**
		 * @brief Enables partial chunking for HTTP message bodies on the builder, modifying the builder's configuration.
		 * @return A reference to the HttpBuilder instance (allows method chaining).
		 */
		HttpBuilder& partialChunks();

		/// @brief Append header - value
		/// @tparam StringT 
		/// @tparam T 
		/// @tparam ...Args 
		/// @param name 
		/// @param value 
		/// @param ...args 
		/// @return Self
		template<typename StringT, typename T, typename... Args>
		HttpBuilder& headers(StringT&& name, T&& value, Args&&... args);

		/**
		 * @brief Builds an HTTP message (request or response) and returns it in the requested type.
		 * @tparam ReturnT The return type for the built HTTP message. Must satisfy concepts::HttpBuilderReturnType. Defaults to std::string.
		 * @param data Optional message body or payload to include in the built HTTP message. Defaults to an empty string.
		 * @param additionalHeaders Optional map of header names to values to include or override in the built HTTP message. Defaults to an empty map.
		 * @return The constructed HTTP message represented as the specified ReturnT (by default a std::string).
		 */
		template<concepts::HttpBuilderReturnType ReturnT = std::string>
		ReturnT build(std::string_view data = "", const std::unordered_map<std::string, std::string>& additionalHeaders = {}) const;

		/**
		 * @brief Builds an HTTP representation from a JSON builder, optionally including additional headers, and returns it as the specified type.
		 * @tparam ReturnT The return type to produce for the built result; constrained by concepts::HttpBuilderReturnType. Defaults to std::string.
		 * @param builder The json::JsonBuilder providing the content to convert into an HTTP representation.
		 * @param additionalHeaders Optional map of header name/value pairs to include in the built HTTP output (defaults to empty).
		 * @return The built HTTP representation of the builder, returned as type ReturnT (typically a string by default).
		 */
		template<concepts::HttpBuilderReturnType ReturnT = std::string>
		ReturnT build(const json::JsonBuilder& builder, const std::unordered_map<std::string, std::string>& additionalHeaders = {}) const;

		/**
		 * @brief Builds an HTTP representation from URL-encoded parameters and optional additional headers.
		 * @tparam ReturnT The return type produced by the builder. Must satisfy concepts::HttpBuilderReturnType; defaults to std::string.
		 * @param urlEncoded A map of URL-encoded key-value pairs to include in the built HTTP representation.
		 * @param additionalHeaders An optional map of additional HTTP headers to include. Defaults to an empty map.
		 * @return The constructed HTTP representation as the specified ReturnT.
		 */
		template<concepts::HttpBuilderReturnType ReturnT = std::string>
		ReturnT build(const std::unordered_map<std::string, std::string>& urlEncoded, const std::unordered_map<std::string, std::string>& additionalHeaders = {}) const;

		/// @brief Set HTTP to output stream
		/// @param outputStream std::ostream subclass instance
		/// @param parser const reference to HttpBuilder instance
		/// @return outputStream
		friend HTTP_API std::ostream& operator << (std::ostream& outputStream, const HttpBuilder& builder);

		~HttpBuilder() = default;
	};
}

namespace web
{
	template<typename KeyT, typename ValueT, typename... Args>
	void HttpBuilder::fillAdditionalHeaders(std::unordered_map<std::string, std::string>& additionalHeaders, KeyT&& key, ValueT&& value, Args&&... args)
	{
		static_assert(sizeof...(args) % 2 == 0);

		additionalHeaders.try_emplace(std::forward<KeyT>(key), std::forward<ValueT>(value));

		if constexpr (sizeof...(args))
		{
			HttpBuilder::fillAdditionalHeaders(additionalHeaders, std::forward<Args>(args)...);
		}
	}

	template<concepts::HttpBuilderReturnType T>
	T HttpBuilder::convert(std::string&& result)
	{
		if constexpr (std::same_as<T, std::string>)
		{
			return result;
		}
		else if constexpr (std::same_as<T, std::vector<char>>)
		{
			return std::vector<char>(std::make_move_iterator(result.begin()), std::make_move_iterator(result.end()));
		}

		throw std::runtime_error("Wrong conversion");
	}

	template<typename... AdditionalHeaders>
	std::string HttpBuilder::buildImplementation(std::string_view data, const std::unordered_map<std::string, std::string>& headers, AdditionalHeaders&&... keyValueAdditionalHeaders) const
	{
		std::string result;
		std::unordered_map<std::string, std::string> additionalHeaders;

		if (data.size())
		{
			additionalHeaders["Content-Length"] = std::to_string(data.size());
		}
		else if (_chunks.size())
		{
			additionalHeaders["Transfer-Encoding"] = "chunked";
		}

		if (method.empty())
		{
			result = std::format("{} {}{}{}", _HTTPVersion, _responseCode, constants::crlf, _headers);
		}
		else
		{
			result = method + ' ';

			if (_parameters.empty() && method != "CONNECT")
			{
				result += "/";
			}

			result += std::format("{} {}{}{}", _parameters, _HTTPVersion, constants::crlf, _headers);
		}

		for (const auto& [header, value] : headers)
		{
			result += std::format("{}: {}{}", header, value, constants::crlf);
		}

		if constexpr (sizeof...(keyValueAdditionalHeaders))
		{
			HttpBuilder::fillAdditionalHeaders(additionalHeaders, std::forward<AdditionalHeaders>(keyValueAdditionalHeaders)...);
		}

		for (const auto& [header, value] : additionalHeaders)
		{
			result += std::format("{}: {}{}", header, value, constants::crlf);
		}

		result += constants::crlf;

		if (data.size())
		{
			result += data;
		}
		else if (_chunks.size())
		{
			result += HttpBuilder::getChunks(_chunks, _partialChunks);
		}

		return result;
	}

	template<typename StringT, typename T, typename... Args>
	HttpBuilder& HttpBuilder::queryParameters(StringT&& name, T&& value, Args&&... args)
	{
		static_assert(std::convertible_to<decltype(name), std::string_view>, "Wrong StringT type");

		if (_parameters.empty())
		{
			_parameters = "/?";
		}

		if constexpr (std::is_arithmetic_v<T>)
		{
			_parameters += std::format("{}={}&", web::encodeUrl(static_cast<std::string_view>(name)), std::to_string(value));
		}
		else if constexpr (std::convertible_to<decltype(value), std::string_view>)
		{
			_parameters += std::format("{}={}&", web::encodeUrl(static_cast<std::string_view>(name)), web::encodeUrl(static_cast<std::string_view>(value)));
		}
		else if constexpr (std::convertible_to<decltype(value), std::string>)
		{
			_parameters += std::format("{}={}&", web::encodeUrl(static_cast<std::string_view>(name)), web::encodeUrl(static_cast<std::string>(value)));
		}
		else
		{
			throw std::logic_error("Bad type of T, it must be converted to string or arithmetic type");
		}

		if constexpr (requires { requires std::invocable<decltype(&HttpBuilder::queryParameters<Args...>), decltype(this), Args...>; })
		{
			return this->queryParameters(std::forward<Args>(args)...);
		}

		_parameters.pop_back(); // remove '&'

		return *this;
	}

	template<typename... Args>
	HttpBuilder& HttpBuilder::parametersWithRoute(std::string_view route, Args&&... args)
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

		return this->queryParameters(std::forward<Args>(args)...);
	}

	template<typename StringT, typename T, typename... Args>
	HttpBuilder& HttpBuilder::headers(StringT&& name, T&& value, Args&&... args)
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

	template<concepts::HttpBuilderReturnType ReturnT>
	ReturnT HttpBuilder::build(std::string_view data, const std::unordered_map<std::string, std::string>& additionalHeaders) const
	{
		return HttpBuilder::convert<ReturnT>(this->buildImplementation(data, additionalHeaders));
	}

	template<concepts::HttpBuilderReturnType ReturnT>
	ReturnT HttpBuilder::build(const json::JsonBuilder& builder, const std::unordered_map<std::string, std::string>& additionalHeaders) const
	{
		return HttpBuilder::convert<ReturnT>(this->buildImplementation(builder.build(), additionalHeaders, "Content-Type", "application/json"));
	}

	template<concepts::HttpBuilderReturnType ReturnT>
	ReturnT HttpBuilder::build(const std::unordered_map<std::string, std::string>& urlEncoded, const std::unordered_map<std::string, std::string>& additionalHeaders) const
	{
		std::string body;

		for (const auto& [key, value] : urlEncoded)
		{
			body += std::format("{}={}&", web::encodeUrl(key), web::encodeUrl(value));
		}

		if (body.size())
		{
			body.pop_back(); // remove last &
		}

		return HttpBuilder::convert<ReturnT>(this->buildImplementation(body, additionalHeaders, "Content-Type", "application/x-www-form-urlencoded"));
	}
}
