#ifndef __COMPILE_TIME_CHECK__
#define __COMPILE_TIME_CHECK__

#include <type_traits>
#include <string>

#define CREATE_CHECK_CLASS(checkMethodName, ...)	\
		private:	\
		template<typename T>	\
		static constexpr decltype(std::declval<T>().checkMethodName(__VA_ARGS__), std::true_type()) checked(int);	\
		\
		template<typename>	\
		static constexpr std::false_type checked(...);	\
		\
		public:	\
		using type = decltype(checked<dataContainer>(0));	\
		enum \
		{	\
			value = type::value	\
		};	\

namespace utility
{
	template<typename dataContainer>
	class checkSize
	{
		CREATE_CHECK_CLASS(size);
	};

	template<typename dataContainer>
	class checkResize
	{
		CREATE_CHECK_CLASS(resize, 0);
	};

	template<typename dataContainer>
	class checkData
	{
		CREATE_CHECK_CLASS(data);
	};

	template<typename dataContainer>
	class checkBegin
	{
		CREATE_CHECK_CLASS(begin);
	};

	template<typename dataContainer>
	class checkEnd
	{
		CREATE_CHECK_CLASS(end);
	};

	template<typename dataContainer>
	class checkInitializerListConstructor
	{
	protected:
		template<typename T>
		static constexpr decltype(T(std::begin(T()), std::end(T())), std::true_type()) checked(int);

		template<typename>
		static constexpr std::false_type checked(...);

	public:
		using type = decltype(checked<dataContainer>(0));
		enum
		{
			value = type::value
		};
	};

	template<typename T>
	class StringConversion
	{
	private:
		template<typename U>
		static constexpr decltype(std::declval<std::string>().append(std::declval<U>()), std::true_type()) check(int);

		template<typename>
		static constexpr std::false_type check(...);

	public:
		using type = decltype(check<T>(0));

		enum
		{
			value = type::value
		};
	};
}

#endif