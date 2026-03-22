/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file DynArray.hpp
/// @author Alexandru Delegeanu
/// @version 0.1
/// @brief Dynamic array.
///

#pragma once

#include <utility>

#include "Graphite/Logger/Logger.hpp"

namespace Graphite::Common {

/**
 * @class DynArray
 * @brief A fixed-capacity, dynamically allocated array with manual memory management.
 *
 * Provides a simple dynamic array implementation with constant-time access and support for move semantics.
 * Capacity is fixed at construction time.
 * Elements are constructed in-place and destroyed on destruction.
 *
 * @tparam T Type of the elements stored in the array.
 */
template <typename T>
class DynArray
{
public:
    using value_type = T; ///< The type of elements stored.
    using size_type = std::size_t; ///< Type used for size and indexing.
    using iterator = T*; ///< Iterator type.
    using const_iterator = const T*; ///< Const iterator type.

    /**
     * @brief Construct a DynArray with a given capacity.
     * @param capacity The maximum number of elements the array can hold.
     *
     * The size is initialized to zero.
     */
    DynArray(size_type const capacity = 0)
        : m_data(static_cast<T*>(::operator new(sizeof(T) * capacity))), m_size(0), m_capacity(capacity)
    {
    }

    /**
     * @brief Deleted copy constructor. DynArray is not copyable.
     */
    DynArray(const DynArray&) = delete;
    /**
     * @brief Deleted copy assignment operator. DynArray is not copyable.
     */
    DynArray& operator=(const DynArray&) = delete;

    /**
     * @brief Move constructor.
     * @param other The DynArray to move from.
     *
     * Transfers ownership of the underlying data.
     */
    DynArray(DynArray&& other) noexcept
        : m_data(std::exchange(other.m_data, nullptr))
        , m_size(std::exchange(other.m_size, 0))
        , m_capacity(std::exchange(other.m_capacity, 0))
    {
    }

    /**
     * @brief Move assignment operator.
     * @param other The DynArray to move from.
     * @return Reference to this DynArray.
     *
     * Transfers ownership of the underlying data.
     */
    DynArray& operator=(DynArray&& other) noexcept
    {
        if (this != &other)
        {
            delete[] m_data;
            m_data = std::exchange(other.m_data, nullptr);
            m_size = std::exchange(other.m_size, 0);
            m_capacity = std::exchange(other.m_capacity, 0);
        }
        return *this;
    }

    /**
     * @brief Destructor. Destroys all elements and deallocates memory.
     */
    ~DynArray()
    {
        for (size_t i = 0; i < m_size; ++i)
            m_data[i].~T();

        ::operator delete(m_data);
    }

    /**
     * @brief Returns the number of elements in the array.
     * @return The current size.
     */
    size_type size() const noexcept { return m_size; }
    /**
     * @brief Returns the maximum capacity of the array.
     * @return The capacity.
     */
    size_type capacity() const noexcept { return m_capacity; }

    /**
     * @brief Adds an element to the end of the array by copy.
     * @param value The value to add.
     *
     * @throws Assertion failure if the array is full.
     */
    void push_back(const T& value)
    {
        GRAPHITE_ASSERT(
            m_size < m_capacity, "DynArray Cannot push back because it will exceed allocated space");

        new (&m_data[m_size++]) T(value);
    }

    /**
     * @brief Adds an element to the end of the array by move.
     * @param value The value to move.
     *
     * @throws Assertion failure if the array is full.
     */
    void push_back(T&& value)
    {
        GRAPHITE_ASSERT(
            m_size < m_capacity, "Cannot push back because it will exceed allocated space");

        new (&m_data[m_size++]) T(std::move(value));
    }

    /**
     * @brief Constructs an element in-place at the end of the array.
     * @tparam Args Argument types for T's constructor.
     * @param args Arguments to forward to T's constructor.
     * @return Reference to the newly constructed element.
     *
     * @throws Assertion failure if the array is full.
     */
    template <typename... Args>
    T& emplace_back(Args&&... args)
    {
        GRAPHITE_ASSERT(m_size < m_capacity, "Cannot emplace_back beyond capacity");
        return *new (&m_data[m_size++]) T(std::forward<Args>(args)...);
    }

    /**
     * @brief Accesses the element at the given index.
     * @param idx Index of the element.
     * @return Reference to the element.
     */
    T& operator[](size_type idx) { return m_data[idx]; }
    /**
     * @brief Accesses the element at the given index (const overload).
     * @param idx Index of the element.
     * @return Const reference to the element.
     */
    const T& operator[](size_type idx) const { return m_data[idx]; }

    /**
     * @brief Returns an iterator to the beginning.
     * @return Iterator to the first element.
     */
    iterator begin() { return m_data; }
    /**
     * @brief Returns an iterator to the end.
     * @return Iterator to one-past-the-last element.
     */
    iterator end() { return m_data + m_size; }
    /**
     * @brief Returns a const iterator to the beginning.
     * @return Const iterator to the first element.
     */
    const_iterator begin() const { return m_data; }
    /**
     * @brief Returns a const iterator to the end.
     * @return Const iterator to one-past-the-last element.
     */
    const_iterator end() const { return m_data + m_size; }
    /**
     * @brief Returns a const iterator to the beginning.
     * @return Const iterator to the first element.
     */
    const_iterator cbegin() const { return m_data; }
    /**
     * @brief Returns a const iterator to the end.
     * @return Const iterator to one-past-the-last element.
     */
    const_iterator cend() const { return m_data + m_size; }

private:
    T* m_data; ///< Pointer to the array data.
    size_type m_size; ///< Current number of elements.
    size_type m_capacity; ///< Maximum capacity.
};

} // namespace Graphite::Common
