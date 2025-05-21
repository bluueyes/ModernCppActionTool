#include "reaction/expression.h"


namespace reaction
{
    template<typename T, typename... Args>
    class ReactImpl : public Expression<T, Args...> 
   {
    public:
        using exprType = typename Expression<T, Args...>::exprType; 
        using valueType = typename Expression<T,Args...>::valueType;
        using Expression<T, Args...>::Expression;

        template<typename U>
        void operator=(U&& other){
            value(std::forward<U>(other));
        }

        decltype(auto) get() const{
            return this->getValue();    
        }
        template<typename U>
            requires Convertable<valueType,U> && IsVarExpr<exprType> && (!IsConstVar<valueType>)
        void value(U&& v){
            this->updateValue(std::forward<U>(v));
            this->notify();
        }

        void addWeakRef(){
            m_weakRef++;
        }

        void removeWeakRef(){
            if(--m_weakRef == 0){
                if constexpr(HasField<valueType>)
                {
                    FieldGraph::getInstance().removeObj(this->getValue().getID());
                }else{
                    ObserverGraph::getInstance().removeNode(this->shared_from_this());
                }
            }
        }

    private:
        std::atomic<int> m_weakRef{0};

    };

    template<typename ReactType>
    class React
    {
    public:
        explicit React(std::shared_ptr<ReactType> ptr=nullptr) : m_weakPtr(ptr) {
            if(auto ptr=m_weakPtr.lock())
            {
                ptr->addWeakRef();
            }
        }

        ~React() {
            if(auto ptr=m_weakPtr.lock())
            {
                ptr->removeWeakRef();
            }
        }

        React(const React& other):m_weakPtr(other.m_weakPtr)
        {
            if(auto ptr=m_weakPtr.lock())
            {
                ptr->addWeakRef();
            }

        }

        React(React&& other)noexcept : m_weakPtr(std::move(other.m_weakPtr))
        {
            other.m_weakPtr.reset();
        }

        React& operator=(const React& other)
        {
            if(this != &other)
            {
                if(auto ptr=m_weakPtr.lock())
                {
                    ptr->removeWeakRef();
                }
                m_weakPtr=other.m_weakPtr;
                if(auto ptr=m_weakPtr.lock())
                {
                    ptr->addWeakRef();
                }
            }
            return *this;
        }
        React& operator=(React&& other)noexcept
        {
            if(this != &other)
            {
                if(auto ptr=m_weakPtr.lock())
                {
                    ptr->removeWeakRef();
                }
                m_weakPtr=std::move(other.m_weakPtr);
                other.m_weakPtr.reset();
            }
            return *this;
        }

        ReactType* operator->() const
        {
            if(auto ptr=m_weakPtr.lock())
            {
                return ptr.get();
            }
            else
            {
                throw std::runtime_error("Weak pointer expired");
            }
        }
        ReactType& operator*() const
        {
            if(auto ptr=m_weakPtr.lock())
            {
                return *ptr;
            }
            else
            {
                throw std::runtime_error("Weak pointer expired");
            }
        }

        explicit operator bool() const
        {
            return !m_weakPtr.expired();
        }

        decltype(auto) get() const
            requires(IsDataReact<ReactType>)
        {
            return getPtr()->get();
        }

        template<typename T>
        void value(T&& t){
            getPtr()->value(std::forward<T>(t));
        }

        std::shared_ptr<ReactType> getPtr() const
        {
            if(auto ptr=m_weakPtr.lock())
            {
                return ptr;
            }
            else
            {
                throw std::runtime_error("Weak pointer expired");
            }
        }

    private:
        std::weak_ptr<ReactType> m_weakPtr;
        
    };

    template<typename T>
    using Field = React<ReactImpl<std::decay_t<T>>>;

    class FieldBase
    {
    public:
        template<typename T>
        auto field(T&& t){
            auto ptr = std::make_shared<ReactImpl<std::decay_t<T>>>(std::forward<T>(t));
            FieldGraph::getInstance().addObj(m_id,ptr->shared_from_this());
            return React(ptr);
        }

        uint64_t getID() const
        {
            return m_id;
        }
    private:
        UniqueID m_id;
    };

    

    template<typename Func,typename... Args>
    auto calc(Func&& t, Args&&... args)
    {
        auto ptr=std::make_shared<ReactImpl<std::decay_t<Func>,std::decay_t<Args>...>>(std::forward<Func>(t), std::forward<Args>(args)...);
        ObserverGraph::getInstance().addNode(ptr);
        return React(ptr);
    }

    template<typename Func,typename... Args>
    auto action(Func&& t, Args&&... args)
    {
        auto ptr=std::make_shared<ReactImpl<std::decay_t<Func>,std::decay_t<Args>...>>(std::forward<Func>(t), std::forward<Args>(args)...);
        ObserverGraph::getInstance().addNode(ptr);
        return React(ptr);
    }

    template<typename SrcType>
    auto var(SrcType&& t){
        
        auto ptr = std::make_shared<ReactImpl<std::decay_t<SrcType>>>(std::forward<SrcType>(t));

        if constexpr(HasField<SrcType>)
        {
            FieldGraph::getInstance().bindField(t.getID(),ptr->shared_from_this());           
        }
        ObserverGraph::getInstance().addNode(ptr);
        return React(ptr);
    }
    
    template<typename SrcType>
    auto constVar(SrcType&& t){
        auto ptr = std::make_shared<ReactImpl<const std::decay_t<SrcType>>>(std::move(t));
        ObserverGraph::getInstance().addNode(ptr);
        return React(ptr);
    }

    
}
