#include <cstdint>
#include <atomic>
#include <unordered_map>
#include <unordered_set>

namespace reaction
{

class UniqueID{

public:
    UniqueID() : m_id(generate()){

    }

    operator uint64_t() const {
        return m_id;
    }

    bool operator==(const UniqueID& other) const {
        return m_id == other.m_id;
    }



private:
    uint64_t generate() {
        static std::atomic<uint64_t> counter{0};
        return counter.fetch_add(1, std::memory_order_relaxed);
    }

    uint64_t m_id;

};
}

namespace std
{
    template<>
    struct hash<reaction::UniqueID>
    {
        size_t operator()(const reaction::UniqueID& id) const
        {
            return std::hash<uint64_t>()(id);
        }
    };
}