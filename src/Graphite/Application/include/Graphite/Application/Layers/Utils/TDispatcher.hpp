/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file TDispatcher.hpp
/// @author Alexandru Delegeanu
/// @version 0.1
/// @brief App layer.
///

#include <utility>

namespace Graphite::Application::Layers::Utils {

/**
 * @tparam TDerived The class inheriting (CRTP)
 * @tparam TActionValue The specific value from that Enum
 * @tparam TPayload The data structure being sent
 */
template <typename TDerived, auto TActionValue, typename TPayload>
class TDispatcher
{
public:
    void Dispatch(TPayload&& payload)
    {
        // 1. Get the derived pointer
        auto* derived = static_cast<TDerived*>(this);

        // 2. Access the application through a standard getter
        // This allows the derived class to store the app however it wants
        auto app = derived->GetApplication();

        // 3. Push the specific action value baked in at compile-time
        app->PushAction(TActionValue, std::move(payload));
    }
};

} // namespace Graphite::Application::Layers::Utils
