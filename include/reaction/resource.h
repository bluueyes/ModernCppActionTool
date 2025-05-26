#include <memory>
#include "observerNode.h"

namespace reaction
{

    template<typename Type>
    class Resource : public ObserverNode
    {
    public:
    Resource() :m_res(nullptr){};

    template<typename T>
    Resource(T&& res) :m_res(std::make_unique<Type>(std::forward<T>(res))){}
    Resource(const Resource&) = delete;
    Resource& operator=(const Resource&) = delete;
    Resource(Resource&&) = default;
    Resource& operator=(Resource&&) = default;

    virtual ~Resource() = default;

    Type& getValue() const
    {
        if(!m_res)
        {
            throw std::runtime_error("Resource is not initialized");
        }
       return *m_res;
    }

    template<typename T>
    void updateValue(T&& t) {
        if(!m_res){
            m_res=std::make_unique<Type>(std::forward<T>(t));
        }else{
            *m_res=std::forward<T>(t);
        }
    }

    Type* getRawPtr() const
    {
        if(!m_res)
        {
            throw std::runtime_error("Resource is not initialized");
        }
        return m_res.get();
    }


    private:
        std::unique_ptr<Type> m_res;
    };

    struct VoidWrapper {};

    template<>
    class Resource<VoidWrapper> : public ObserverNode
    {
    public:
        VoidWrapper getValue() const
        {
            return VoidWrapper{};
        }
    };
}