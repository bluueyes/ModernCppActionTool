#include <concepts>

namespace reaction{
    struct VoidWrapper;
    template<typename T , typename U>
    concept Convertable = std::is_convertible_v<std::decay_t<T>,std::decay_t<U>>;

    struct varExpr;
    template<typename T>
    concept IsVarExpr = std::is_same_v<T,varExpr>;

    template<typename T>
    concept IsConstVar=std::is_const_v<std::remove_reference_t<T>> ;

    template<typename T>
    concept IsVoidType = std::is_void_v<T> || std::is_same_v<T,VoidWrapper>;

    class FieldBase;
    template<typename T>
    concept HasField = requires(T t)
    {
        {t.getID()} -> std::same_as<uint64_t>;
        requires std::is_base_of_v<FieldBase,std::decay_t<T>>;
    };

    class ObserverNode;
    template<typename T>
    concept IsReactNode = requires(T t)
    {
        {t.shared_from_this()} -> std::same_as<std::shared_ptr<ObserverNode>>;
    };

    template<typename T>
    concept IsDataReact = requires(T t)
    {
        typename T::valueType;
        requires IsReactNode<T> && !IsVoidType<typename T::valueType>;
    };
}