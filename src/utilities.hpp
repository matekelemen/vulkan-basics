#pragma once

// --- STL Includes ---
#include <iterator>
#include <compare>


namespace concepts {


template <class TIt, class TValue = void>
concept Iterator
= !std::is_same_v<typename std::iterator_traits<TIt>::iterator_category, void>
  && (std::is_same_v<TValue,void> || std::is_same_v<TValue,typename std::iterator_traits<TIt>::value_type>);


template <class T, class TValue = void>
concept Container
= requires (T instance)
{
    typename T::value_type;
    {instance.begin()};
    {instance.end()};
} && (std::is_same_v<TValue,void>
      || std::is_same_v<TValue,typename T::value_type>);


template <class T, class ... TArgs>
concept Comparison
= requires (T instance, TArgs... r_args)
{
    {instance(std::forward<TArgs>(r_args)...)} -> std::same_as<std::strong_ordering>;
};


} // namespace concepts
