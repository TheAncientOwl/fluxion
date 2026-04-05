/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file TDoubleBuffer.hpp
/// @author Alexandru Delegeanu
/// @version 0.3
/// @brief A thread-safe double-buffering data structure.
///

#pragma once

#include <atomic>
#include <concepts>
#include <mutex>
#include <utility>

namespace Graphite::Common::DataStructures {

namespace Concepts {

template <typename F, typename T>
concept BackBufferCallable = std::invocable<F, T&> && std::same_as<std::invoke_result_t<F, T&>, void>;

} // namespace Concepts

enum class EDoubleBufferSyncPolicy
{
    Swap = 0,
    Copy = 1,
    SwapLocking = 2,
    CopyLocking = 3,
};

/**
 * @brief A thread-safe double-buffering data structure.
 * Manages two instances of T: 'front' for reading and 'back' for writing.
 */
template <typename T, EDoubleBufferSyncPolicy TSyncPolicy>
class TDoubleBuffer
{
public: // Threading API
    bool IsDirty() const noexcept { return m_has_changes.load(std::memory_order_acquire); }

private: // Threading API
    void MarkDirty() noexcept { m_has_changes.store(true, std::memory_order_release); }

public: // IO API
    /**
     * @brief Initialize the front and back buffers.
     *
     * @param front Self explanatory
     * @param back Self explanatory
     */
    void Init(T&& front, T&& back)
    {
        m_front = std::move(front);
        m_back = std::move(back);
    }

    /**
     * @brief Get the Back buffer.
     * @note NOT THREAD SAFE!
     *
     * @return const& to the back buffer.
     */
    inline T const& GetBack() const noexcept { return m_back; }

    /**
     * @brief Get the Front buffer.
     * @note NOT THREAD SAFE!
     *
     * @return const& to the front buffer.
     */
    inline T const& GetFront() const noexcept { return m_front; }

    // --- Swap ---
    /**
     * @brief Swaps front and back buffers.
     * @return true if data was swapped, false if no changes were pending.
     * @note Only available when TSyncPolicy == EDoubleBufferSyncPolicy::FrontSwap.
     * @warning The 'back' buffer will contain the previous 'front' buffer after call.
     */
    bool SyncFrontBufferSwap()
        requires(
            TSyncPolicy == EDoubleBufferSyncPolicy::Swap ||
            TSyncPolicy == EDoubleBufferSyncPolicy::SwapLocking)
    {
        return InternalSync([](T& front, T& back) { std::swap(front, back); });
    }

    /**
     * @brief Update the back buffer, usually after it was swapped with the front one.
     * @note Thread Safe
     *
     * @tparam TBufferPreparer Here you can reserve / allocate memory for the back buffer if needed.
     * @tparam TBufferUpdater Here you can modify the back buffer.
     */
    template <Concepts::BackBufferCallable<T> TBufferPreparer, Concepts::BackBufferCallable<T> TBufferUpdater>
    void UpdateBackBufferSwapLocking(TBufferPreparer&& buffer_preparer, TBufferUpdater&& buffer_updater)
        requires(TSyncPolicy == EDoubleBufferSyncPolicy::SwapLocking)
    {
        {
            std::lock_guard lock{m_mutex};
            std::invoke(std::forward<TBufferPreparer>(buffer_preparer), m_back);
            std::invoke(std::forward<TBufferUpdater>(buffer_updater), m_back);
        }
        MarkDirty();
    }

    /**
     * @brief Update the back buffer, usually after it was swapped with the front one.
     * @note Not Thread Safe
     *
     * @tparam TBufferPreparer Here you can reserve / allocate memory for the back buffer if needed.
     * @tparam TBufferUpdater Here you can modify the back buffer.
     */
    template <Concepts::BackBufferCallable<T> TBufferPreparer, Concepts::BackBufferCallable<T> TBufferUpdater>
    void UpdateBackBufferSwap(TBufferPreparer&& buffer_preparer, TBufferUpdater&& buffer_updater)
        requires(TSyncPolicy == EDoubleBufferSyncPolicy::Swap)
    {
        std::invoke(std::forward<TBufferPreparer>(buffer_preparer), m_back);
        std::invoke(std::forward<TBufferUpdater>(buffer_updater), m_back);
        MarkDirty();
    }

    // --- Copy ---
    /**
     * @brief Copy back buffer to front buffer.
     * @return true if data was copied, false if no changes were pending.
     * @note Only available when TSyncPolicy == EDoubleBufferSyncPolicy::FrontSwap.
     */
    bool SyncFrontBufferCopy()
        requires(
            TSyncPolicy == EDoubleBufferSyncPolicy::Copy ||
            TSyncPolicy == EDoubleBufferSyncPolicy::CopyLocking)
    {
        return InternalSync([](T& front, T& back) { front = back; });
    }

    /**
     * @brief Update the back buffer.
     * @note Thread Safe
     *
     * @tparam TBufferUpdater Here you can modify the back buffer.
     */
    template <Concepts::BackBufferCallable<T> TBufferUpdater>
    void UpdateBackBufferCopyLocking(TBufferUpdater&& buffer_updater)
        requires(TSyncPolicy == EDoubleBufferSyncPolicy::CopyLocking)
    {
        {
            std::lock_guard lock{m_mutex};
            std::invoke(std::forward<TBufferUpdater>(buffer_updater), m_back);
        }
        MarkDirty();
    }

    /**
     * @brief Update the back buffer.
     * @note Not Thread Safe
     *
     * @tparam TBufferUpdater Here you can modify the back buffer.
     */
    template <Concepts::BackBufferCallable<T> TBufferUpdater>
    void UpdateBackBufferCopy(TBufferUpdater&& buffer_updater)
        requires(TSyncPolicy == EDoubleBufferSyncPolicy::Copy)
    {
        {
            std::lock_guard lock{m_mutex};
            std::invoke(std::forward<TBufferUpdater>(buffer_updater), m_back);
        }
        MarkDirty();
    }

public: // Constructors, Copy / Move Operations
    TDoubleBuffer() = default;
    ~TDoubleBuffer() = default;

    TDoubleBuffer(const TDoubleBuffer& other)
    {
        std::lock_guard lock{other.m_mutex};
        m_front = other.m_front;
        m_back = other.m_back;
        m_has_changes.store(
            other.m_has_changes.load(std::memory_order_acquire), std::memory_order_release);
    }

    TDoubleBuffer& operator=(const TDoubleBuffer& other)
    {
        if (this == &other)
            return *this;

        // Lock both to ensure consistent state during deep copy
        std::scoped_lock lock{m_mutex, other.m_mutex};
        m_front = other.m_front;
        m_back = other.m_back;

        bool had_changes = other.m_has_changes.load(std::memory_order_acquire);
        m_has_changes.store(had_changes, std::memory_order_release);

        return *this;
    }

    TDoubleBuffer(TDoubleBuffer&& other) noexcept
    {
        std::lock_guard lock{other.m_mutex};

        m_front = std::move(other.m_front);
        m_back = std::move(other.m_back);

        // Take the dirty flag state and reset the source
        m_has_changes.store(
            other.m_has_changes.exchange(false, std::memory_order_acq_rel), std::memory_order_release);
    }

    TDoubleBuffer& operator=(TDoubleBuffer&& other) noexcept
    {
        if (this == &other)
            return *this;

        std::scoped_lock lock{m_mutex, other.m_mutex};

        m_front = std::move(other.m_front);
        m_back = std::move(other.m_back);

        bool had_changes = other.m_has_changes.exchange(false, std::memory_order_acq_rel);
        m_has_changes.store(had_changes, std::memory_order_release);

        return *this;
    }

private: // Internal API
    /**
     * @brief Reduce internal syncing mutex boilerplate
     * @see TDoubleBuffer<T>::SyncSwap
     * @see TDoubleBuffer<T>::SyncCopy
     */
    template <typename Func>
    inline bool InternalSync(Func&& op)
    {
        if (!m_has_changes.load(std::memory_order_acquire))
        {
            return false;
        }

        std::lock_guard lock{m_mutex};
        op(m_front, m_back);
        m_has_changes.store(false, std::memory_order_release);

        return true;
    }

private: // Fields
    // --- Data ---
    T m_front{};
    T m_back{};

    // --- Threading ---
    mutable std::mutex m_mutex;
    std::atomic<bool> m_has_changes{false};
};

template <typename T>
using TCopyDoubleBuffer = TDoubleBuffer<T, EDoubleBufferSyncPolicy::Copy>;

template <typename T>
using TSwapDoubleBuffer = TDoubleBuffer<T, EDoubleBufferSyncPolicy::Swap>;

template <typename T>
using TCopyLockingDoubleBuffer = TDoubleBuffer<T, EDoubleBufferSyncPolicy::CopyLocking>;

template <typename T>
using TSwapLockingDoubleBuffer = TDoubleBuffer<T, EDoubleBufferSyncPolicy::SwapLocking>;

} // namespace Graphite::Common::DataStructures
