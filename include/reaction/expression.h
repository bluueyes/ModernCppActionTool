#include <tuple>
#include "reaction/resource.h"
#include <utility>
#include <functional>

namespace reaction
{
    struct calcExpr{};
    struct varExpr{};

    template<typename Func, typename... Args>
    class ReactImpl;

    template<typename ReactType>
    class React;

    template<typename T>
    struct ExpressionTraits
    {
        using type = T;
    };

    template<typename T>
    struct ExpressionTraits<React<ReactImpl<T>>>
    {
        using type = T;
    };

    template<typename Func,typename... Args>
    struct ExpressionTraits<React<ReactImpl<Func,Args...>>>
    {
        using type = std::invoke_result_t<Func, typename ExpressionTraits<Args>::type...>;
    };

    template<typename Func,typename... Args>
    using ReturnType = typename ExpressionTraits<React<ReactImpl<Func,Args...>>>::type;

    template<typename Func, typename... Args>
    class Expression : public Resource<ReturnType<Func, Args...>>
    {   
    public:
        using exprType = calcExpr;
        using valueType=ReturnType<Func,Args...>;

        template<typename F,typename... A>
        Expression(F&& func,A&&... args)
            : Resource<ReturnType<Func,Args...>>(), m_func(std::forward<F>(func)), m_args(std::forward<A>(args)...)
        {
            this->updateObserver(std::forward<A>(args)...);
            evaluate();
        }

    private:
        void valueChange() override{
            evaluate();
        }

        void evaluate()
        {
            if constexpr(IsVoidType<valueType>)
            {
                [&]<std::size_t... I>(std::index_sequence<I...>)
                {
                    std::invoke(m_func, std::get<I>(m_args).get().get()...);
                    return;
                }(std::make_index_sequence<std::tuple_size_v<decltype(m_args)>>{});
                
            }
            else{
                auto result = [&]<std::size_t... I>(std::index_sequence<I...>)
                {
                    return std::invoke(m_func, std::get<I>(m_args).get().get()...);
                }(std::make_index_sequence<std::tuple_size_v<decltype(m_args)>>{});
            
                this->updateValue(result);
            }

        }

        Func m_func;
        std::tuple<std::reference_wrapper<Args>...> m_args;
    };

    template<typename Type>
    class Expression<Type> : public Resource<Type>
    {
    public:
        using exprType = varExpr;
        using valueType = Type;
        using Resource<Type>::Resource;

    };


}