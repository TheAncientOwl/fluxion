/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file Ref.hpp
/// @author Alexandru Delegeanu
/// @version 0.1
/// @brief Reference counter.
///

#include <atomic>
#include <cstddef>

namespace Graphite::Common {

/**
 * @class RefCounted
 * @brief Base class that provides reference counting capabilities.
 */
class RefCounted
{
public:
    /**
     * @brief Self explanatory.
     */
    RefCounted() = default;
    /**
     * @brief Self explanatory.
     */
    virtual ~RefCounted() = default;

    /**
     * @brief Self explanatory.
     */
    inline void IncrementRefCount() { ++m_ref_count; };
    /**
     * @brief Self explanatory.
     */
    inline void DecrementRefCount() { --m_ref_count; };
    /**
     * @brief Self explanatory.
     */
    inline std::uint32_t GetRefCount() const { return m_ref_count; }

private:
    mutable std::atomic<std::uint32_t> m_ref_count{0};
};

/**
 * @class Ref
 * @brief Smart pointer that manages reference counting for RefCounted objects.
 * @tparam T Type of the referenced object, must derive from RefCounted.
 */
template <typename T>
class Ref
{
public:
    /**
     * @brief Self explanatory.
     */
    Ref() = default;

    /**
     * @brief Construct a Ref from nullptr.
     * */
    Ref(std::nullptr_t _) : m_instance{nullptr} {}

    /**
     * @brief Construct a Ref from a raw pointer.
     * @param instance Pointer to the object to manage.
     */
    Ref(T* instance) : m_instance{instance}
    {
        static_assert(std::is_base_of<RefCounted, T>::value, "Class is not RefCounted");
        IncrementRefCount();
    }

    /**
     * @brief Copy constructor.
     * @param other Another Ref to copy from.
     */
    Ref(Ref<T> const& other)
    {
        m_instance = other.m_instance;
        IncrementRefCount();
    }

    /**
     * @brief Template copy constructor for compatible Ref types.
     * @tparam T2 Other type that is base or derived from T.
     * @param other Another Ref of type T2 to copy from.
     */
    template <typename T2>
        requires(std::is_base_of_v<T2, T> || std::is_base_of_v<T, T2>)
    Ref(Ref<T2> const& other)
    {
        m_instance = (T*)other.m_instance;
        IncrementRefCount();
    }

    /**
     * @brief Destructor decrements reference count and deletes object if count reaches zero.
     * */
    ~Ref() { DecrementRefCount(); }

    /**
     * @brief Self explanatory.
     */
    T* operator->() { return m_instance; }
    /**
     * @brief Self explanatory.
     */
    T const* operator->() const { return m_instance; }

    /**
     * @brief Self explanatory.
     */
    T& operator*() { return *m_instance; }
    /**
     * @brief Self explanatory.
     */
    T const& operator*() const { return *m_instance; }

    /**
     * @brief Cast this Ref to another Ref type.
     * @tparam T2 Target type that is base or derived from T.
     * @return Ref<T2> instance pointing to the same object.
     */
    template <typename T2>
        requires(std::is_base_of_v<T2, T> || std::is_base_of_v<T, T2>)
    Ref<T2> As() const
    {
        return Ref<T2>(*this);
    }

    /**
     * @brief Create a new Ref instance by constructing a new object.
     * @tparam Args Argument types for the constructor.
     * @param args Arguments to forward to the constructor.
     * @return Ref<T> managing the newly created object.
     */
    template <typename... Args>
    static Ref<T> Create(Args&&... args)
    {
        return Ref<T>(new T(std::forward<Args>(args)...));
    }

private:
    /**
     * @brief Increment ref count.
     */
    void IncrementRefCount()
    {
        if (m_instance)
        {
            m_instance->IncrementRefCount();
        }
    }

    /**
     * @brief Decrement reference count and deletes object if count reaches zero.
     */
    void DecrementRefCount()
    {
        if (m_instance)
        {
            m_instance->DecrementRefCount();

            if (m_instance->GetRefCount() == 0)
            {
                delete m_instance;
                m_instance = nullptr;
            }
        }
    }

private:
    T* m_instance{};
};

} // namespace Graphite::Common
