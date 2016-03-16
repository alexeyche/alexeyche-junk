#pragma once

#include <type_traits>

namespace NDnn {



	template<typename, typename T>
	struct HasMethod {
	    static_assert(
	        std::integral_constant<T, false>::value,
	        "Second template parameter needs to be of function type.");
	};

	// specialization that does the checking

	template<typename C, typename Ret, typename... Args>
	struct HasMethod<C, Ret(Args...)> {
	private:
	    template<typename T>
	    static constexpr auto check(T*)
	    -> typename
	        std::is_same<
	            decltype( std::declval<T>().serialize( std::declval<Args>()... ) ),
	            Ret    // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	        >::type;  // attempt to call it and see if the return type is correct

	    template<typename>
	    static constexpr std::false_type check(...);

	    typedef decltype(check<C>(0)) type;

	public:
	    static constexpr bool value = type::value;
	};

} // namespace NDnn