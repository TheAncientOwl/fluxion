/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file TDoubleBufferedVector.hpp
/// @author Alexandru Delegeanu
/// @version 0.1
/// @brief Reference counter.
///

#pragma once

#include <atomic>
#include <memory>
#include <mutex>
#include <vector>

namespace Graphite::Common::DataStructures {

template <typename Item>
struct TDoubleBufferedVector
{
public: // Types
    using ItemPtrType = std::shared_ptr<Item>;
    using StorageType = std::vector<ItemPtrType>;

public: // Constructors / op=
    TDoubleBufferedVector() = default;
    ~TDoubleBufferedVector() = default;

    TDoubleBufferedVector(TDoubleBufferedVector const& other)
    {
        std::lock_guard lock{other.m_mutex};

        front = other.front;
        back = other.back;

        bool had_changes = other.m_has_changes.load(std::memory_order_acquire);
        m_has_changes.store(had_changes, std::memory_order_release);
    }

    TDoubleBufferedVector& operator=(TDoubleBufferedVector const& other)
    {
        if (this == &other)
        {
            return *this;
        }

        std::scoped_lock lock{m_mutex, other.m_mutex};

        front = other.front;
        back = other.back;

        bool had_changes = other.m_has_changes.load(std::memory_order_acquire);
        m_has_changes.store(had_changes, std::memory_order_release);

        return *this;
    }

    TDoubleBufferedVector(TDoubleBufferedVector&& other) noexcept
    {
        std::lock_guard lock{other.m_mutex};

        front = std::move(other.front);
        back = std::move(other.back);

        // Atomically read the state and reset the other
        m_has_changes.store(
            other.m_has_changes.exchange(false, std::memory_order_acq_rel), std::memory_order_release);
    }
    TDoubleBufferedVector& operator=(TDoubleBufferedVector&& other) noexcept
    {
        if (this == &other)
        {
            return *this;
        }

        std::scoped_lock lock{m_mutex, other.m_mutex};

        front = std::move(other.front);
        back = std::move(other.back);

        bool had_changes = other.m_has_changes.exchange(false, std::memory_order_acq_rel);
        m_has_changes.store(had_changes, std::memory_order_release);

        return *this;
    }

public: // API
    bool SyncFront()
    {
        if (!m_has_changes.load(std::memory_order_acquire))
        {
            return false;
        }

        std::lock_guard lock{m_mutex};
        front = back;
        m_has_changes.store(false, std::memory_order_release);
        return true;
    }

    void MarkDataHasChanges() { m_has_changes.store(true, std::memory_order_release); }

public: // Fields
    StorageType front{};
    StorageType back{};

private: // Fields
    mutable std::mutex m_mutex;
    std::atomic<bool> m_has_changes{false};
};

} // namespace Graphite::Common::DataStructures
