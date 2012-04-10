/******************************************************************************\
 *           ___        __                                                    *
 *          /\_ \    __/\ \                                                   *
 *          \//\ \  /\_\ \ \____    ___   _____   _____      __               *
 *            \ \ \ \/\ \ \ '__`\  /'___\/\ '__`\/\ '__`\  /'__`\             *
 *             \_\ \_\ \ \ \ \L\ \/\ \__/\ \ \L\ \ \ \L\ \/\ \L\.\_           *
 *             /\____\\ \_\ \_,__/\ \____\\ \ ,__/\ \ ,__/\ \__/.\_\          *
 *             \/____/ \/_/\/___/  \/____/ \ \ \/  \ \ \/  \/__/\/_/          *
 *                                          \ \_\   \ \_\                     *
 *                                           \/_/    \/_/                     *
 *                                                                            *
 * Copyright (C) 2011, 2012                                                   *
 * Dominik Charousset <dominik.charousset@haw-hamburg.de>                     *
 *                                                                            *
 * This file is part of libcppa.                                              *
 * libcppa is free software: you can redistribute it and/or modify it under   *
 * the terms of the GNU Lesser General Public License as published by the     *
 * Free Software Foundation, either version 3 of the License                  *
 * or (at your option) any later version.                                     *
 *                                                                            *
 * libcppa is distributed in the hope that it will be useful,                 *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of             *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                       *
 * See the GNU Lesser General Public License for more details.                *
 *                                                                            *
 * You should have received a copy of the GNU Lesser General Public License   *
 * along with libcppa. If not, see <http://www.gnu.org/licenses/>.            *
\******************************************************************************/


#ifndef LIBCPPA_UTIL_TYPE_LIST_HPP
#define LIBCPPA_UTIL_TYPE_LIST_HPP

#include <typeinfo>
#include <type_traits>

#include "cppa/util/tbind.hpp"
#include "cppa/util/if_else.hpp"
#include "cppa/util/type_pair.hpp"
#include "cppa/util/void_type.hpp"

namespace cppa {

// forward declarations
class uniform_type_info;
uniform_type_info const* uniform_typeid(std::type_info const&);

} // namespace cppa

namespace cppa { namespace util {

/**
 * @defgroup MetaProgramming Metaprogramming utility.
 */

/**
 * @addtogroup MetaProgramming
 * @{
 */

template<typename... Types> struct type_list;

template<>
struct type_list<>
{
    typedef void_type head;
    typedef void_type back;
    typedef type_list<> tail;
    static const size_t size = 0;
};

/**
 * @brief A list of types.
 */
template<typename Head, typename... Tail>
struct type_list<Head, Tail...>
{

    typedef Head head;

    typedef type_list<Tail...> tail;

    typedef typename if_else_c<sizeof...(Tail) == 0,
                               Head,
                               wrapped<typename tail::back> >::type
            back;

    static const size_t size =  sizeof...(Tail) + 1;

};

template<typename T>
struct is_type_list
{
    static constexpr bool value = false;
};

template<typename... Ts>
struct is_type_list<type_list<Ts...> >
{
    static constexpr bool value = true;
};

// static list zip(list, list)

/**
 * @brief Zips two lists of equal size.
 *
 * Creates a list formed from the two lists @p ListA and @p ListB,
 * e.g., tl_zip<type_list<int,double>,type_list<float,string>>::type
 * is type_list<type_pair<int,float>,type_pair<double,string>>.
 */
template<class ListA, class ListB,
         template<typename, typename> class Fun = to_type_pair>
struct tl_zip;

template<typename... LhsElements, typename... RhsElements,
         template<typename, typename> class Fun>
struct tl_zip<type_list<LhsElements...>, type_list<RhsElements...>, Fun>
{
    static_assert(sizeof...(LhsElements) ==
                  sizeof...(RhsElements),
                  "Lists have different size");
    typedef type_list<typename Fun<LhsElements, RhsElements>::type...> type;
};

template<class ListA>
struct tl_unzip;

template<typename... Elements>
struct tl_unzip< type_list<Elements...> >
{
    typedef type_list<typename Elements::first...> first;
    typedef type_list<typename Elements::second...> second;
};

// static list zip_with_index(list)

template<bool Done, class List, size_t Pos, size_t...>
struct tl_zip_with_index_impl;

template<class List, size_t Pos, size_t... Range>
struct tl_zip_with_index_impl<false, List, Pos, Range...>
        : tl_zip_with_index_impl<List::size == (Pos + 1), List, (Pos + 1), Range..., Pos>
{
};

template<typename... Ts, size_t Pos, size_t... Range>
struct tl_zip_with_index_impl<true, type_list<Ts...>, Pos, Range...>
{
    typedef type_list<type_pair<std::integral_constant<size_t, Range>, Ts>...>
            type;
};


template<class List>
struct tl_zip_with_index
{
    typedef typename tl_zip_with_index_impl<false, List, 0>::type type;
};

template<>
struct tl_zip_with_index<type_list<> >
{
    typedef type_list<> type;
};

// list reverse()

template<class From, typename... Elements>
struct tl_reverse_impl;

template<typename T0, typename... T, typename... E>
struct tl_reverse_impl<type_list<T0, T...>, E...>
{
    typedef typename tl_reverse_impl<type_list<T...>, T0, E...>::type type;
};

template<typename... E>
struct tl_reverse_impl<type_list<>, E...>
{
    typedef type_list<E...> type;
};

/**
 * @brief Creates a new list wih elements in reversed order.
 */
template<class List>
struct tl_reverse
{
    typedef typename tl_reverse_impl<List>::type type;
};

// bool find(list, type)

/**
 * @brief Finds the first element of type @p What beginning at
 *        index @p Pos.
 */
template<class List, template<typename> class Predicate, int Pos = 0>
struct tl_find_impl;

template<template<typename> class Predicate, int Pos>
struct tl_find_impl<type_list<>, Predicate, Pos>
{
    static constexpr int value = -1;
};

template<template<typename> class Predicate, int Pos,
         typename Head, typename... Tail>
struct tl_find_impl<type_list<Head, Tail...>, Predicate, Pos>
{
    static constexpr int value =
            Predicate<Head>::value
            ? Pos
            : tl_find_impl<type_list<Tail...>, Predicate, Pos+1>::value;
};

/**
 * @brief Finds the first element of type @p What beginning at
 *        index @p Pos.
 */
template<class List, typename What, int Pos = 0>
struct tl_find
{
    static constexpr int value =
            tl_find_impl<List,
                         tbind<std::is_same, What>::template type,
                         Pos
            >::value;
};

/**
 * @brief Finds the first element satisfying @p Predicate beginning at
 *        index @p Pos.
 */
template<class List, template<typename> class Predicate, int Pos = 0>
struct tl_find_if
{
    static constexpr int value = tl_find_impl<List, Predicate, Pos>::value;
};

// list first_n(size_t)

template<size_t N, class List, typename... T>
struct tl_first_n_impl;

template<class List, typename... T>
struct tl_first_n_impl<0, List, T...>
{
    typedef type_list<T...> type;
};

template<size_t N, typename L0, typename... L, typename... T>
struct tl_first_n_impl<N, type_list<L0, L...>, T...>
{
    typedef typename tl_first_n_impl<N-1, type_list<L...>, T..., L0>::type type;
};

/**
 * @brief Creates a new list from the first @p N elements of @p List.
 */
template<class List, size_t N>
struct tl_first_n
{
    static_assert(List::size >= N, "List::size < N");
    typedef typename tl_first_n_impl<N, List>::type type;
};

template<class List>
struct tl_first_n<List, 0>
{
    typedef type_list<> type;
};

// list last_n(size_t)

template<size_t TargetSize, size_t Size, class List, typename... T>
struct tl_last_n_impl;

template<size_t TargetSize, size_t Size, typename T0, typename... T>
struct tl_last_n_impl<TargetSize, Size, type_list<>, T0, T...>
{
    typedef typename tl_last_n_impl<
                TargetSize, Size-1,
                type_list<>, T...
            >::type
            type;
};

template<size_t Size, typename... T>
struct tl_last_n_impl<Size, Size, type_list<>, T...>
{
    typedef type_list<T...> type;
};

template<size_t TargetSize, size_t Size, typename L0, typename... L, typename... T>
struct tl_last_n_impl<TargetSize, Size, type_list<L0, L...>, T...>
{
    typedef typename tl_last_n_impl<TargetSize, Size,
                                    type_list<L...>, T..., L0>::type type;
};

/**
 * @brief Creates a new list from the first @p N elements of @p List.
 */
template<class List, size_t N>
struct tl_last_n
{
    static_assert(List::size >= N, "List::size < N");
    typedef typename tl_last_n_impl<N, List::size, List>::type type;
};

template<class List>
struct tl_last_n<List, 0>
{
    typedef type_list<> type;
};

// bool forall(predicate)

/**
 * @brief Tests whether a predicate holds for all elements of a list.
 */
template<class List, template<typename> class Predicate>
struct tl_forall
{
    static constexpr bool value =
               Predicate<class List::head>::value
            && tl_forall<class List::tail, Predicate>::value;
};

template<template<typename> class Predicate>
struct tl_forall<type_list<>, Predicate>
{
    static constexpr bool value = true;
};

/**
 * @brief Tests whether a predicate holds for some of the elements of a list.
 */
template<class List, template<typename> class Predicate>
struct tl_exists
{
    static constexpr bool value =
               Predicate<class List::head>::value
            || tl_exists<class List::tail, Predicate>::value;
};

template<template<typename> class Predicate>
struct tl_exists<type_list<>, Predicate>
{
    static constexpr bool value = false;
};


// size_t count(predicate)

/**
 * @brief Counts the number of elements in the list which satisfy a predicate.
 */
template<class List, template<typename> class Predicate>
struct tl_count
{
    static constexpr size_t value =
              (Predicate<class List::head>::value ? 1 : 0)
            + tl_count<class List::tail, Predicate>::value;
};

template<template<typename> class Predicate>
struct tl_count<type_list<>, Predicate>
{
    static constexpr size_t value = 0;
};

// size_t count_not(predicate)

/**
 * @brief Counts the number of elements in the list which satisfy a predicate.
 */
template<class List, template<typename> class Predicate>
struct tl_count_not
{
    static constexpr size_t value =
              (Predicate<class List::head>::value ? 0 : 1)
            + tl_count_not<class List::tail, Predicate>::value;
};

template<template<typename> class Predicate>
struct tl_count_not<type_list<>, Predicate>
{
    static constexpr size_t value = 0;
};

// bool zipped_forall(predicate)

/**
 * @brief Tests whether a predicate holds for all elements of a zipped list.
 */
template<class ZippedList, template<typename, typename> class Predicate>
struct tl_zipped_forall
{
    typedef typename ZippedList::head head;
    static constexpr bool value =
               Predicate<typename head::first, typename head::second>::value
            && tl_zipped_forall<typename ZippedList::tail, Predicate>::value;
};

template<template<typename, typename> class Predicate>
struct tl_zipped_forall<type_list<>, Predicate>
{
    static constexpr bool value = true;
};

template<class ListA, class ListB>
struct tl_concat_impl;

/**
 * @brief Concatenates two lists.
 */
template<typename... ListATypes, typename... ListBTypes>
struct tl_concat_impl<type_list<ListATypes...>, type_list<ListBTypes...> >
{
    typedef type_list<ListATypes..., ListBTypes...> type;
};

// static list concat(list, list)

/**
 * @brief Concatenates lists.
 */
template<class... Lists>
struct tl_concat;

template<class List0>
struct tl_concat<List0>
{
    typedef List0 type;
};

template<class List0, class List1, class... Lists>
struct tl_concat<List0, List1, Lists...>
{
    typedef typename tl_concat<
                typename tl_concat_impl<List0, List1>::type,
                Lists...
            >::type
            type;
};

// list push_back(list, type)

template<class List, typename What>
struct tl_push_back;

/**
 * @brief Appends @p What to given list.
 */
template<typename... ListTypes, typename What>
struct tl_push_back<type_list<ListTypes...>, What>
{
    typedef type_list<ListTypes..., What> type;
};

template<class List, typename What>
struct tl_push_front;

/**
 * @brief Appends @p What to given list.
 */
template<typename... ListTypes, typename What>
struct tl_push_front<type_list<ListTypes...>, What>
{
    typedef type_list<What, ListTypes...> type;
};

// list map(list, trait)

/**
 * @brief Creates a new list by applying a "template function" to each element.
 */
template<class List, template<typename> class Fun>
struct tl_map;

template<template<typename> class Fun, typename... Elements>
struct tl_map<type_list<Elements...>, Fun>
{
    typedef type_list<typename Fun<Elements>::type...> type;
};

// list zipped_map(trait)

template<class List, template<typename, typename> class Fun>
struct tl_zipped_map;

/**
 * @brief Creates a new list by applying a "binary template function"
 *        to each element.
 */
template<typename... T, template<typename, typename> class Fun>
struct tl_zipped_map<type_list<T...>, Fun>
{
    typedef type_list<typename Fun<typename T::first,
                                   typename T::second>::type...> type;
};

// list pop_back()

/**
 * @brief Creates a new list wih all but the last element of @p List.
 */
template<class List>
struct tl_pop_back
{
    typedef typename tl_reverse<List>::type rlist;
    typedef typename tl_reverse<typename rlist::tail>::type type;
};

// type at(size_t)

template<size_t N, typename... E>
struct tl_at_impl;


template<size_t N, typename E0, typename... E>
struct tl_at_impl<N, E0, E...>
{
    typedef typename tl_at_impl<N-1, E...>::type type;
};

template<typename E0, typename... E>
struct tl_at_impl<0, E0, E...>
{
    typedef E0 type;
};

template<class List, size_t N>
struct tl_at;

/**
 * @brief Gets element at index @p N of @p List.
 */
template<size_t N, typename... E>
struct tl_at<type_list<E...>, N>
{
    static_assert(N < sizeof...(E), "N >= List::size");
    typedef typename tl_at_impl<N, E...>::type type;
};

// list prepend(type)

template<class List, typename What>
struct tl_prepend;

/**
 * @brief Creates a new list with @p What prepended to @p List.
 */
template<typename What, typename... T>
struct tl_prepend<type_list<T...>, What>
{
    typedef type_list<What, T...> type;
};


// list filter(predicate)
// list filter_not(predicate)

template<class List, bool... Selected>
struct tl_filter_impl;

template<>
struct tl_filter_impl< type_list<> >
{
    typedef type_list<> type;
};

template<typename T0, typename... T, bool... S>
struct tl_filter_impl<type_list<T0, T...>, false, S...>
{
    typedef typename tl_filter_impl<type_list<T...>, S...>::type type;
};

template<typename T0, typename... T, bool... S>
struct tl_filter_impl<type_list<T0, T...>, true, S...>
{
    typedef typename tl_prepend<typename tl_filter_impl<type_list<T...>, S...>::type, T0>::type type;
};

template<class List, template<typename> class Predicate>
struct tl_filter;

/**
 * @brief Create a new list containing all elements which satisfy @p Predicate.
 */
template<template<typename> class Predicate, typename... T>
struct tl_filter<type_list<T...>, Predicate>
{
    typedef typename tl_filter_impl<type_list<T...>, Predicate<T>::value...>::type type;
};

/**
 * @brief Creates a new list containing all elements which
 *        do not satisfy @p Predicate.
 */
template<class List, template<typename> class Predicate>
struct tl_filter_not;

template<template<typename> class Predicate>
struct tl_filter_not<type_list<>, Predicate>
{
    typedef type_list<> type;
};

template<template<typename> class Predicate, typename... T>
struct tl_filter_not<type_list<T...>, Predicate>
{
    typedef typename tl_filter_impl<type_list<T...>, !Predicate<T>::value...>::type type;
};

/**
 * @brief Creates a new list containing all elements which
 *        are equal to @p Type.
 */
template<class List, class Type>
struct tl_filter_type;

template<class Type, typename... T>
struct tl_filter_type<type_list<T...>, Type>
{
    typedef typename tl_filter_impl<type_list<T...>, std::is_same<T, Type>::value...>::type type;
};

/**
 * @brief Creates a new list containing all elements which
 *        are not equal to @p Type.
 */
template<class List, class Type>
struct tl_filter_not_type;

template<class Type, typename... T>
struct tl_filter_not_type<type_list<T...>, Type>
{
    typedef typename tl_filter_impl<type_list<T...>, (!std::is_same<T, Type>::value)...>::type type;
};

// list distinct(list)

/**
 * @brief Creates a new list from @p List without any duplicate elements.
 */
template<class List>
struct tl_distinct;

template<>
struct tl_distinct<type_list<> > { typedef type_list<> type; };

template<typename Head, typename... Tail>
struct tl_distinct<type_list<Head, Tail...> >
{
    typedef typename tl_concat<
                type_list<Head>,
                typename tl_distinct<
                    typename tl_filter_type<type_list<Tail...>, Head>::type
                >::type
            >::type
            type;
};

// list resize(list, size, fill_type)

template<class List, bool OldSizeLessNewSize,
         size_t OldSize, size_t NewSize, typename FillType>
struct tl_pad_right_impl;

template<class List, size_t OldSize, size_t NewSize, typename FillType>
struct tl_pad_right_impl<List, false, OldSize, NewSize, FillType>
{
    typedef typename tl_first_n<List, NewSize>::type type;
};

template<class List, size_t Size, typename FillType>
struct tl_pad_right_impl<List, false, Size, Size, FillType>
{
    typedef List type;
};

template<class List, size_t OldSize, size_t NewSize, typename FillType>
struct tl_pad_right_impl<List, true, OldSize, NewSize, FillType>
{
    typedef typename tl_pad_right_impl<
                typename tl_push_back<List, FillType>::type,
                (OldSize + 1) < NewSize,
                OldSize + 1,
                NewSize,
                FillType
            >::type
            type;
};

/**
 * @brief Resizes the list to contain @p NewSize elements and uses
 *        @p FillType to initialize the new elements with.
 */
template<class List, size_t NewSize, typename FillType>
struct tl_pad_right
{
    typedef typename tl_pad_right_impl<
            List, (List::size < NewSize), List::size, NewSize, FillType
            >::type
            type;
};

// bool pad_left(list, N)

template<class List, size_t OldSize, size_t NewSize, typename FillType>
struct tl_pad_left_impl
{
    typedef typename tl_pad_left_impl<
                typename tl_push_front<List, FillType>::type,
                OldSize + 1,
                NewSize,
                FillType
            >::type
            type;
};

template<class List, size_t Size, typename FillType>
struct tl_pad_left_impl<List, Size, Size, FillType>
{
    typedef List type;
};

/**
 * @brief Resizes the list to contain @p NewSize elements and uses
 *        @p FillType to initialize prepended elements with.
 */
template<class List, size_t NewSize, typename FillType = void_type>
struct tl_pad_left
{
    static_assert(NewSize >= List::size, "List too big");
    typedef typename tl_pad_left_impl<
                List, List::size, NewSize, FillType
            >::type
            type;
};

// bool is_zipped(list)

template<class List>
struct tl_is_zipped
{
    static constexpr bool value = tl_forall<List, is_type_pair>::value;
};

/**
 * @brief Removes trailing @p What elements from the end.
 */
template<class List, typename What>
struct tl_trim
{
    typedef typename util::if_else<
                std::is_same<class List::back, What>,
                typename tl_trim<typename tl_pop_back<List>::type, What>::type,
                util::wrapped<List>
            >::type
            type;
};

template<typename What>
struct tl_trim<type_list<>, What>
{
    typedef type_list<> type;
};

// list group_by(list, predicate)

template<bool Append, typename What, class Where>
struct tl_group_by_impl_step;

template<typename What, typename... Ts>
struct tl_group_by_impl_step<true, What, type_list<Ts...> >
{
    typedef type_list<type_list<Ts..., What> > type;
};

template<typename What, class List>
struct tl_group_by_impl_step<false, What, List>
{
    typedef type_list<List, type_list<What> > type;
};

template<class In, class Out, template<typename, typename> class Predicate>
struct tl_group_by_impl
{
    typedef typename Out::back last_group;
    typedef typename tl_group_by_impl_step<
                Predicate<typename In::head, typename last_group::back>::value,
                typename In::head,
                last_group
            >::type
            suffix;
    typedef typename tl_pop_back<Out>::type prefix;
    typedef typename tl_concat<prefix, suffix>::type new_out;

    typedef typename tl_group_by_impl<
                typename In::tail,
                new_out,
                Predicate
            >::type
            type;
};

template<template<typename, typename> class Predicate, typename Head, typename... Tail>
struct tl_group_by_impl<type_list<Head, Tail...>, type_list<>, Predicate>
{
    typedef typename tl_group_by_impl<
                type_list<Tail...>,
                type_list<type_list<Head> >,
                Predicate
            >::type
            type;
};

template<class Out, template<typename, typename> class Predicate>
struct tl_group_by_impl<type_list<>, Out, Predicate>
{
    typedef Out type;
};


template<class List, template<typename, typename> class Predicate>
struct tl_group_by
{
    typedef typename tl_group_by_impl<List, type_list<>, Predicate>::type type;
};

/**
 * @}
 */
} } // namespace cppa::util

#endif // LIBCPPA_UTIL_TYPE_LIST_HPP
