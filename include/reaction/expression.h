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
        using rawType = std::invoke_result_t<Func, typename ExpressionTraits<Args>::type...>;
        using type = std::conditional_t<IsVoidType<rawType>, VoidWrapper, rawType>;
    };

    template<typename Func,typename... Args>
    using ReturnType = typename ExpressionTraits<React<ReactImpl<Func,Args...>>>::type;

    template<typename Func, typename... Args>
    class Expression : public Resource<ReturnType<Func, Args...>>
    {   
    public:
        using exprType = calcExpr;
        using valueType=ReturnType<Func,Args...>;

        template<typename F, typename... A>
        void setSource(F && func,A&&... args)
        {
            if constexpr(std::convertible_to<ReturnType<std::decay_t<F>,std::decay_t<A>...>, valueType>)
            {
                this->updateObserver(std::forward<A>(args)...);
                setFunctor(createFunc(std::forward<F>(func), std::forward<A>(args)...));
                evaluate();
            }

        }

    private:
        template<typename F, typename... A>
        auto createFunc(F && func, A&&... args) 
        {
            return [func=std::forward<F>(func), ...args=args.getPtr()]() {
                if constexpr(IsVoidType<valueType>)
                {   
                    std::invoke(func, args->get()...);
                    return VoidWrapper{};
                }
                else
                {
                    return std::invoke(func, args->get()...);
                }
            };
        }


        void valueChange() override{
            evaluate();
            this->notify();
        }

        void evaluate()
        {
            if constexpr(IsVoidType<valueType>)
            {
                std::invoke(m_func);
                
            }
            else{
                this->updateValue(std::invoke(m_func));
            }

        }

        void setFunctor(const std::function<valueType()>& func)
        {
            m_func = func;
        }

        std::function<valueType()> m_func;
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