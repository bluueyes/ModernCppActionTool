#include <iostream>
#include <string_view>
#include "reaction/react.h"
#include <tuple>

template<typename T>
struct my_warper
{
    inline static T m_value;
};

template<typename T>
struct Field
{
    /* data */
};

template<typename Type,auto ...field>
struct private_visitor
{
   friend constexpr auto get_private_field(const my_warper<Type>&) {
        constexpr auto tp = std::make_tuple(field...);
        return tp;
    }
};

#define REFL_PRIVATE(STRUCT,...) \
    inline constexpr auto get_private_field(const my_warper<STRUCT>&); \
    template struct private_visitor<STRUCT,##__VA_ARGS__>; 


struct Dog
{
	bool m_male;
};

struct Person
{
	bool m_male;
	Field<std::string> m_name;
	Field<int> m_age;
    int m_height;
};

class PersonPrivate {
	Field<std::string> m_name;
	Field<int> m_age;
	bool m_male;
};


class PersonPrivate2 {
	Field<std::string> m_name2;
	Field<int> m_age2;
	bool m_male2;
};

REFL_PRIVATE(PersonPrivate,&PersonPrivate::m_name,&PersonPrivate::m_age,&PersonPrivate::m_male);

REFL_PRIVATE(PersonPrivate2,&PersonPrivate2::m_name2,&PersonPrivate2::m_age2,&PersonPrivate2::m_male2);

template<typename T>
constexpr auto g_value = T{};

template<typename T>
inline constexpr T& get_global_value(){
    return my_warper<T>::m_value;
};

template<typename T>
struct Is_Field : std::false_type {};

template<typename T>
struct Is_Field<Field<T>> : std::true_type {};


template<auto MemberPtrs>
struct MemberPointerTraits{};

template<typename T, typename C,T C::*MemberPtr>
struct MemberPointerTraits<MemberPtr>
{
    using type = T;
    using class_type = C;
};

template<auto MemberPtrs>
using member_value_t  = typename MemberPointerTraits<MemberPtrs>::type;

template<typename T>
concept IsAggregate = std::is_aggregate_v<T>;

template<typename Tuple>
constexpr bool check_field(const Tuple& tup)
{
    bool found = false;
    std::apply([&](auto&&... args) {
       ((found = found || Is_Field<std::decay_t<decltype(args)>>{}), ...);
    }, tup);

    return found;
}

struct AnyType{
    template<typename T>
    operator T();
};

template<typename T>
consteval size_t countMember(auto&& ...Args){
    if constexpr (!requires{T{Args...};}){
        return sizeof...(Args)-1;
    }else{
        return countMember<T>(Args...,AnyType{});
    }
}

template<typename T>
constexpr size_t member_count_v = countMember<T>();

template <auto val>
inline constexpr std::string_view getFunName() {
#if defined(__GNUC__)
  constexpr std::string_view func_name = __PRETTY_FUNCTION__;
#elif defined(_MSC_VER)
  constexpr std::string_view func_name = __FUNCSIG__;
#endif
size_t pos1=func_name.find("val = ");
if(pos1==std::string_view::npos){
    return {};
}
size_t pos2=func_name.find(";",pos1);
if(pos2==std::string_view::npos){
    return {};
}
    return func_name.substr(pos1+6,pos2-pos1-6);
}


template<typename T,std::size_t n>
struct ReflectHelper
{};

#define REF_STRUCT(n,...) \
template<typename T> \
struct ReflectHelper<T,n> { \
    static constexpr auto getTuple() { \
        auto &&[__VA_ARGS__] = get_global_value<T>(); \
        return std::tie(__VA_ARGS__); \
    } \
    static constexpr auto reflectFieldImpl() \
    { \
        constexpr auto ref_tup = getTuple(); \
        return check_field(ref_tup); \
    } \
}; 

template<typename T>
constexpr bool reflectField()
{
    return ReflectHelper<T,member_count_v<T>>::reflectFieldImpl();
}


template<typename T>
struct ReflectField{
    static constexpr auto reflect(){
        constexpr auto tp  = get_private_field(my_warper<T>{});
        bool found = false;
        [&]<size_t ...Is>(std::index_sequence<Is...>)
        {
            ((found = found || Is_Field<member_value_t<std::get<Is>(tp)>>()), ...);
        }(std::make_index_sequence<std::tuple_size_v<decltype(tp)>>{});

        return found;
    }
};

template<IsAggregate T>
struct ReflectField<T>
{
    static constexpr auto reflect(){
        return ReflectHelper<T,member_count_v<T>>::reflectFieldImpl();
    }
};

template<typename T>
constexpr bool reflectField_v = ReflectField<T>::reflect();


REF_STRUCT(1,m1)
REF_STRUCT(2,m1,m2)
REF_STRUCT(3,m1,m2,m3)
REF_STRUCT(4,m1,m2,m3,m4)
REF_STRUCT(5,m1,m2,m3,m4,m5)

int main()
{
    static_assert(reflectField_v<PersonPrivate2>, "Field not found");
    static_assert(reflectField<Person>(), "Field not found");
    constexpr auto tp  = ReflectHelper<Person,member_count_v<Person>>::getTuple();
    
    [&]<size_t ...Is>(std::index_sequence<Is...>)
    {
        (std::cout <<...<<getFunName<&std::get<Is>(tp)>() );
    }(std::make_index_sequence<std::tuple_size_v<decltype(tp)>>{});

    std::cout << "\n";
    constexpr auto tp2  = get_private_field(my_warper<PersonPrivate2>{});
    [&]<size_t ...Is>(std::index_sequence<Is...>)
    {
        (std::cout <<...<<getFunName<get<Is>(tp2)>() );
    }(std::make_index_sequence<std::tuple_size_v<decltype(tp2)>>{});
}
