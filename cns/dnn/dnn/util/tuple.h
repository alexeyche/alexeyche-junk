
#include <tuple>
#include <utility>

namespace NDnn {

	template<std::size_t I = 0, typename FuncT, typename... Tp>
	inline typename std::enable_if<I == sizeof...(Tp), void>::type
	  ForEach(std::tuple<Tp...> &, FuncT) // Unused arguments are given no names.
	  { }

	template<std::size_t I = 0, typename FuncT, typename... Tp>
	inline typename std::enable_if<I < sizeof...(Tp), void>::type
	  ForEach(std::tuple<Tp...>& t, FuncT f)
	  {
	    f(std::get<I>(t));
	    ForEach<I + 1, FuncT, Tp...>(t, f);
	  }

	template<std::size_t I = 0, typename FuncT, typename... Tp>
	inline typename std::enable_if<I == sizeof...(Tp), void>::type
	  ForEachEnumerate(std::tuple<Tp...> &, FuncT) // Unused arguments are given no names.
	  { }

	template<std::size_t I = 0, typename FuncT, typename... Tp>
	inline typename std::enable_if<I < sizeof...(Tp), void>::type
	  ForEachEnumerate(std::tuple<Tp...>& t, FuncT f)
	  {
	    f(I, std::get<I>(t));
	    ForEachEnumerate<I + 1, FuncT, Tp...>(t, f);
	  }

	
} // namespace NDnn
