/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file TDoubleBuffer.hpp
/// @author Alexandru Delegeanu
/// @version 0.1
/// @brief A thread-safe double-buffering data structure.
///

#pragma once

#include <atomic>
#include <mutex>
#include <utility>

namespace Graphite::Common::DataStructures {

/**
 * @brief A thread-safe double-buffering data structure.
 * Manages two instances of T: 'front' for reading and 'back' for writing.
 */
template <typename T>
class TDoubleBuffer
{
public: // Public fields
    T front{};
    T back{};

public: // Constructors, Copy / Move operations
    TDoubleBuffer() = default;
    ~TDoubleBuffer() = default;

    TDoubleBuffer(const TDoubleBuffer& other)
    {
        std::lock_guard lock{other.m_mutex};
        front = other.front;
        back = other.back;
        m_has_changes.store(
            other.m_has_changes.load(std::memory_order_acquire), std::memory_order_release);
    }

    TDoubleBuffer& operator=(const TDoubleBuffer& other)
    {
        if (this == &other)
            return *this;

        // Lock both to ensure consistent state during deep copy
        std::scoped_lock lock{m_mutex, other.m_mutex};
        front = other.front;
        back = other.back;

        bool had_changes = other.m_has_changes.load(std::memory_order_acquire);
        m_has_changes.store(had_changes, std::memory_order_release);

        return *this;
    }

    TDoubleBuffer(TDoubleBuffer&& other) noexcept
    {
        std::lock_guard lock{other.m_mutex};

        front = std::move(other.front);
        back = std::move(other.back);

        // Take the dirty flag state and reset the source
        m_has_changes.store(
            other.m_has_changes.exchange(false, std::memory_order_acq_rel), std::memory_order_release);
    }

    TDoubleBuffer& operator=(TDoubleBuffer&& other) noexcept
    {
        if (this == &other)
            return *this;

        std::scoped_lock lock{m_mutex, other.m_mutex};

        front = std::move(other.front);
        back = std::move(other.back);

        bool had_changes = other.m_has_changes.exchange(false, std::memory_order_acq_rel);
        m_has_changes.store(had_changes, std::memory_order_release);

        return *this;
    }

public: // Public API
    /**
     * @brief Swaps pointers between front and back buffers.
     * @return true if data was swapped, false if no changes were pending.
     */
    bool SyncFront()
    {
        if (!m_has_changes.load(std::memory_order_acquire))
        {
            return false;
        }

        {
            std::lock_guard lock{m_mutex};
            std::swap(front, back);
            m_has_changes.store(false, std::memory_order_release);
        }
        return true;
    }

    void MarkDirty() noexcept { m_has_changes.store(true, std::memory_order_release); }

    bool IsDirty() const noexcept { return m_has_changes.load(std::memory_order_acquire); }

private: // Internal fields
    mutable std::mutex m_mutex;
    std::atomic<bool> m_has_changes{false};
};

} // namespace Graphite::Common::DataStructures
