/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file TThreadSafeQueue.hpp
/// @author Alexandru Delegeanu
/// @version 0.1
/// @brief
///

#pragma once

#include <any>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <queue>

namespace Graphite::Application {

template <typename ActionEnum>
struct TAppAction
{
    ActionEnum type;
    std::any payload;
};

template <typename T>
class TThreadSafeQueue
{
public:
    void Push(T const& item)
    {
        std::lock_guard lock(m_mutex);
        m_queue.push(item);
        m_cv.notify_one();
    }

    bool WaitAndPop(T& out_item, std::atomic<bool>& is_running)
    {
        std::unique_lock lock(m_mutex);
        m_cv.wait(lock, [this, &is_running]() { return !m_queue.empty() || !is_running.load(); });

        if (!is_running.load() && m_queue.empty())
        {
            return false;
        }

        out_item = m_queue.front();
        m_queue.pop();
        return true;
    }

private:
    std::queue<T> m_queue;
    std::mutex m_mutex;
    std::condition_variable m_cv;
};

} // namespace Graphite::Application
