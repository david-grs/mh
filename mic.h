#pragma once

#include "ht.h"
#include "filter.h"

#include <vector>
#include <tuple>
#include <type_traits>
#include <utility>
#include <map>

#ifdef __cpp_template_auto

template <typename C, typename T>
std::pair<C, T> get_type_member(T C::*);

template <typename C, typename T>
std::pair<C, T> get_type_mem_fun(T(C::*) ());

template <typename C, typename T>
std::pair<C, T> get_type_const_mem_fun(T(C::*) () const);

template <auto M>
struct member
{
    using class_type = typename decltype(get_type_member(M))::first_type;
    using type = typename decltype(get_type_member(M))::second_type;

    type operator()(const class_type& c) const { return c.*M; }
};

template <auto M>
struct mem_fun
{
    using class_type = typename decltype(get_type_mem_fun(M))::first_type;
    using type = typename decltype(get_type_mem_fun(M))::second_type;

    type operator()(const class_type& c) const { return (c.*M)(); }
};

template <auto M>
struct const_mem_fun
{
    using class_type = typename decltype(get_type_const_mem_fun(M))::first_type;
    using type = typename decltype(get_type_const_mem_fun(M))::second_type;

    type operator()(const class_type& c) const { return (c.*M)(); }
};


#else

template <typename C, typename T, T C::* M>
struct member
{
    using type = T;
    type operator()(const C& c) const { return c.*M; }
};

template <typename C, typename T, T(C::* M) ()>
struct mem_fun
{
    using type = T;
    type operator()(const C& c) const { return (c.*M)(); }
};

template <typename C, typename T, T(C::* M) () const>
struct const_mem_fun
{
    using type = T;
    type operator()(const C& c) const { return (c.*M)(); }
};

#endif


template <typename Tag, typename Index>
struct unordered
{
    using tag = Tag;
    using index = Index;
};

template <typename Tag, typename Index>
struct ordered
{
    using tag = Tag;
    using index = Index;
};

struct sequence {};

template <typename... Args>
struct mic_index;

template <typename Object>
struct mic_index<Object>
{
    using key_type = void;
};

template <typename Index>
struct index_traits;

template <typename Index>
struct index_base
{
    using key_type = typename index_traits<Index>::key_type;
    using value_type = typename index_traits<Index>::value_type;
    using iterator = typename index_traits<Index>::iterator;
    using const_iterator = typename index_traits<Index>::const_iterator;

    template <typename... Args>
    std::pair<iterator, bool> emplace(Args&&... args)
    {
        return emplace_unique(std::forward<Args>(args)...);
    }

private:
    template <typename Pair>
    std::pair<iterator, bool> emplace_unique(Pair&& p)
    {
        return emplace_pair(std::forward<Pair>(p), __can_extract_key<key_type, Pair>());
    }

    template <typename Pair>
    std::pair<iterator, bool> emplace_pair(Pair&& p, __extract_key_first)
    {
        return index_traits<Index>::emplace_unique_key(static_cast<Index&>(*this), p.first, std::forward<Pair>(p));
    }

    template <typename Pair>
    std::pair<iterator, bool> emplace_pair(Pair&& p, __extract_key_fail)
    {
        return emplace_unique_impl(std::forward<Pair>(p));
    }

    template <typename First, typename Second, typename RawFirst = typename std::decay<First>::type>
    typename std::enable_if_t<std::is_same<key_type, RawFirst>::value, std::pair<iterator, bool>>
    emplace_unique(First&& first, Second&& second)
    {
        return index_traits<Index>::emplace_unique_key(static_cast<Index&>(*this), std::forward<First>(first), std::forward<First>(first), std::forward<Second>(second));
    }

    template <typename First, typename Second, typename RawFirst = typename std::decay<First>::type>
    typename std::enable_if_t<!std::is_same<key_type, RawFirst>::value && std::is_constructible<key_type, RawFirst>::value, std::pair<iterator, bool>>
    emplace_unique(First&& first, Second&& second)
    {
        return index_traits<Index>::emplace_unique_key(static_cast<Index&>(*this), key_type(first), std::forward<Second>(second));
    }

    template <typename... Args>
    std::pair<iterator, bool> emplace_unique(Args&&... args)
    {
        return emplace_unique_impl(std::forward<Args>(args)...);
    }

    template <typename... Args>
    std::pair<iterator, bool> emplace_unique_impl(Args&&... args)
    {
        value_type p(std::forward<Args>(args)...);
        return index_traits<Index>::emplace_unique_key(static_cast<Index&>(*this), p.first, std::move(p));
    }
};

template <typename Object, typename Tag, typename Index>
struct mic_index<Object, unordered<Tag, Index>> : public ht<typename Index::type, Object*>
{
    using base = index_base<mic_index<Object, unordered<Tag, Index>>>;

    using key_type = typename index_traits<mic_index>::key_type;
    using value_type = typename index_traits<mic_index>::value_type;
    using iterator = typename index_traits<mic_index>::iterator;
    using const_iterator = typename index_traits<mic_index>::const_iterator;
    using index_type = Index;
    using tag = Tag;

    explicit mic_index() :
        ht<key_type, Object*>(empty_key_t<key_type>(key_type{}))
    {}
};


template <typename Object, typename Tag, typename Index>
struct mic_index<Object, ordered<Tag, Index>> : public std::map<typename Index::type, Object*>
{
    using key_type = typename index_traits<mic_index>::key_type;
    using value_type = typename index_traits<mic_index>::value_type;
    using iterator = typename index_traits<mic_index>::iterator;
    using const_iterator = typename index_traits<mic_index>::const_iterator;
    using index_type = Index;
    using tag = Tag;
};

template <typename Object, typename Tag, typename Index>
struct index_traits<mic_index<Object, unordered<Tag, Index>>>
{
    using key_type = typename Index::type;
    using value_type = typename ht<key_type, Object*>::value_type;
    using iterator = typename ht<key_type, Object*>::iterator;
    using const_iterator = typename ht<key_type, Object*>::const_iterator;

    template <typename... Args>
    static auto emplace_unique_key(mic_index<Object, unordered<Tag, Index>>& index, Args&&... args)
    {
        return index.emplace_unique_key(std::forward<Args>(args)...);
    }
};

template <typename Object, typename Tag, typename Index>
struct index_traits<mic_index<Object, ordered<Tag, Index>>>
{
    using key_type = typename Index::type;
    using value_type = std::pair<const key_type, Object*>;
    using iterator = int;
    using const_iterator = int;

    template <typename... Args>
    static auto emplace_unique_key(mic_index<Object, ordered<Tag, Index>>& index, Args&&... args)
    {
        return index.emplace_unique_key(std::forward<Args>(args)...);
    }
};

namespace detail
{

template <typename X, typename TupleT, std::size_t Index = 0>
struct get_index_from_key
{
    static_assert(Index < std::tuple_size<TupleT>::value, "type not found");

    static constexpr const std::size_t value =
         std::conditional<std::is_same<X, typename std::tuple_element_t<Index, TupleT>::key_type>::value,
                           std::integral_constant<std::size_t, Index>,
                           get_index_from_key<X, TupleT, Index + 1>>::type::value;
};

template <typename Tag, typename TupleT, std::size_t Index = 0>
struct get_index_from_tag
{
    static_assert(Index < std::tuple_size<TupleT>::value, "type not found");

    static constexpr std::size_t value =
        std::conditional<std::is_same<Tag, typename std::tuple_element_t<Index, TupleT>::tag>::value,
                                  std::integral_constant<std::size_t, Index>,
                                  get_index_from_tag<Tag, TupleT, Index + 1>>::type::value;
};

struct for_each_t
{
private:
    template <typename TupleT, typename F, std::size_t... Is>
    void impl(TupleT&& t, F f, std::index_sequence<Is...>)
    {
        (void)std::initializer_list<int>{int{}, (f(std::get<Is>(std::forward<TupleT>(t))), void(), int{})...};
    }

public:
    template <typename TupleT, typename F>
    void operator()(TupleT&& t, F f)
    {
        impl(std::forward<TupleT>(t), std::ref(f),
             std::make_index_sequence<std::tuple_size<typename std::decay<TupleT>::type>::value>());
    }
};

template <std::size_t Index>
struct for_each_if_not_t
{
private:
    template <typename TupleT, typename F, std::size_t... Is>
    void impl(TupleT&& t, F f, std::index_sequence<Is...>)
    {
        (void)std::initializer_list<int>{int{}, (f(std::get<Is>(std::forward<TupleT>(t))), void(), int{})...};
    }

public:
    template <typename TupleT, typename F>
    void operator()(TupleT&& t, F f)
    {
        impl(std::forward<TupleT>(t), std::ref(f),
             filter<Index, std::make_index_sequence<std::tuple_size<typename std::decay<TupleT>::type>::value>>()());
    }
};

}

template <typename Object, typename... Args>
struct mic
{
    using size_type = std::size_t;

private:
    using indices = std::tuple<mic_index<Object, Args>...>;

    template <std::size_t Index, bool IsConst = false>
    struct index_view_base
    {
        static constexpr std::size_t index = Index;

        template <typename T>
        explicit index_view_base(T* _t) : t(_t)
        {}

        template <typename... TArgs>
        auto find(TArgs&&... args) { return this->t->template find<Index>(std::forward<TArgs>(args)...); }

        auto begin()        { return std::get<Index>(this->t->__indices).begin(); }
        auto begin() const  { return std::get<Index>(this->t->__indices).begin(); }
        auto cbegin() const { return std::get<Index>(this->t->__indices).cbegin(); }

        auto end()        { return std::get<Index>(this->t->__indices).end(); }
        auto end() const  { return std::get<Index>(this->t->__indices).end(); }
        auto cend() const { return std::get<Index>(this->t->__indices).cend(); }

    protected:
        typename std::conditional<IsConst, const mic*, mic*>::type t;
    };

public:
    template <std::size_t Index>
    struct index_view : public index_view_base<Index>
    {
        using index_view_base<Index>::index_view_base;

        template <typename... TArgs>
        auto emplace(TArgs&&... args) { return this->t->template emplace<Index>(std::forward<TArgs>(args)...); }
    };

    template <std::size_t Index>
    struct const_index_view : public index_view_base<Index, true>
    {
        using index_view_base<Index, true>::index_view_base;
    };

    template <std::size_t Index>
    auto index() { return index_view<Index>(this); }

    template <std::size_t Index>
    auto index() const { return const_index_view<Index>(this); }

    template <typename Tag>
    auto index()
    {
        constexpr const std::size_t Index = get_index_from_tag<Tag>();
        return index<Index>();
    }

    template <typename Tag>
    auto index() const
    {
        constexpr const std::size_t Index = get_index_from_tag<Tag>();
        return index<Index>();
    }

    template <typename K>
    auto find(K&& k) const
    {
        constexpr const std::size_t Index = get_index_from_key<K>();
        return std::get<Index>(__indices).find(std::forward<K>(k));
    }

    template <typename K>
    size_type erase(K&& k)
    {
        constexpr const std::size_t Index = get_index_from_key<K>();
        auto it = std::get<Index>(__indices).find(std::forward<K>(k));

        if (it == std::get<Index>(__indices).cend())
            return 0;

        const Object* obj = it->second;
        std::get<Index>(__indices).erase(it);

        // TODO optimize for delete: adding pointers back to other structs
        detail::for_each_if_not_t<Index>()(__indices, [obj](auto&& x)
        {
            using M = typename std::decay<decltype(x)>::type ::index_type;
            const auto& k = M()(*obj);
            x.erase(k);
        });

        return 1;
    }

    auto begin()        { return index_view<0>(this).begin(); }
    auto begin() const  { return const_index_view<0>(this).begin(); }
    auto cbegin() const { return const_index_view<0>(this).cbegin(); }

    auto end()        { return index_view<0>(this).end(); }
    auto end() const  { return const_index_view<0>(this).end(); }
    auto cend() const { return const_index_view<0>(this).cend(); }

    bool empty() const { return _data.empty(); }
    size_type size() const { return _data.size(); }
    size_type max_size() const { return _data.max_size(); }

    void insert(Object&& obj)
    {
        _data.push_back(std::move(obj));

        Object* inserted = &_data.back();
        detail::for_each_t()(__indices, [inserted](auto&& x)
        {
            using M = typename std::decay<decltype(x)>::type ::index_type;
            auto k = M()(*inserted);
            x.emplace(k, inserted);
        });
    }

    void clear()
    {
        detail::for_each_t()(__indices, [](auto&& x) { x.clear(); });
        _data.clear();
    }

private:
    template <std::size_t Index, typename K>
    auto find(K&& k) const
    {
        return std::get<Index>(__indices).find(std::forward<K>(k));
    }

    template <std::size_t Index, typename K>
    auto find(K&& k)
    {
        return std::get<Index>(__indices).find(std::forward<K>(k));
    }

    template <std::size_t Index, typename... TArgs>
    auto emplace(TArgs&&... args)
    {
        auto p = std::get<Index>(__indices).emplace(std::forward<TArgs>(args)...);
        if (p.second)
            _data.emplace_back(Object{});
        return p;
    }

    template <typename T>
    static constexpr std::size_t get_index_from_key() { return detail::get_index_from_key<T, indices>::value; }

    template <typename Tag>
    static constexpr std::size_t get_index_from_tag() { return detail::get_index_from_tag<Tag, indices>::value; }

    std::vector<Object> _data;
    indices __indices;
};
