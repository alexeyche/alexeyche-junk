#ifndef MEMOIZE_H
#define MEMOIZE_H

#include <map>
#include <tuple>
#include <cstdio>

template <typename ReturnType, typename... Args>
std::function<ReturnType (Args...)> memoize(std::function<ReturnType (Args...)> func)
{
    std::map<std::tuple<Args...>, ReturnType> cache;
    return ([=](Args... args) mutable  {
            std::tuple<Args...> t(args...);

            if (cache.find(t) == cache.end())
            {
                ReturnType r = func(args...);
                cache[t] = r;
            }
            return cache[t];
    });
}


#endif
