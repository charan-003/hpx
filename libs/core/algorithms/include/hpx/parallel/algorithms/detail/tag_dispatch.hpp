//  Copyright (c) 2026 Sai Charan Arvapally
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <hpx/config.hpp>
// TODO(tag_invoke-removal): drop with has_legacy_tag_invoke in upcoming PR.
#include <hpx/modules/tag_invoke.hpp>

#include <utility>

namespace hpx::detail {

    // Detects whether an ADL customization point `hpx_invoke(tag, args...)`
    // has been defined for the given CPO `Tag`. This is the replacement hook
    // for external customizations of core algorithm CPOs (e.g. by
    // full/segmented_algorithms), used in place of `tag_invoke`.
    template <typename Tag, typename... Args>
    concept has_hpx_invoke = requires(Tag const& tag, Args&&... args) {
        hpx_invoke(tag, HPX_FORWARD(Args, args)...);
    };

    // TODO(tag_invoke-removal): drop once full/segmented_algorithms migrate to
    // `hpx_invoke` and upcoming PR removes the legacy probe below.
    template <typename Tag, typename... Args>
    concept has_legacy_tag_invoke = requires(Tag const& tag, Args&&... args) {
        tag_invoke(tag, HPX_FORWARD(Args, args)...);
    };

    template <typename Tag, typename... Args>
    concept has_invoke_default = requires(
        Args&&... args) { Tag::invoke_default(HPX_FORWARD(Args, args)...); };

    template <typename Tag, typename Base, typename... Args>
    inline constexpr bool tag_dispatch_is_nothrow_v =
        has_hpx_invoke<Tag, Args&&...> ?
        noexcept(
            hpx_invoke(std::declval<Tag const&>(), std::declval<Args>()...)) :
        has_legacy_tag_invoke<Tag, Args&&...> ?
        hpx::functional::is_nothrow_tag_invocable_v<Tag, Args&&...> :
        has_invoke_default<Tag, Args&&...> ?
        noexcept(Tag::invoke_default(std::declval<Args>()...)) :
        noexcept(std::declval<Base const&>()(std::declval<Args>()...));

    // CRTP mixin providing the dispatch logic for converted algorithm CPOs
    // that used to derive (possibly indirectly) from
    // hpx::functional::detail::tag_fallback<Tag>. Dispatch order:
    //
    //   1. hpx_invoke(tag, args...)       external customization (new hook)
    //   2. tag_invoke(tag, args...)       legacy external customization
    //   3. Tag::invoke_default(args...) algorithm default (was tag_fallback_invoke)
    //   4. Base::operator()(args...)      sender/policy/scheduler (e.g.
    //                                     tag_parallel_algorithm)
    //
    // `Base` keeps working exactly as before (unmodified); this mixin only
    // adds an additional, higher-priority overload on top without touching
    // shared infrastructure, so each CPO can be converted independently of
    // every other one.
    template <typename Tag, typename Base>
    struct tag_dispatch : Base
    {
        template <typename... Args>
            requires(has_hpx_invoke<Tag, Args...> ||
                has_legacy_tag_invoke<Tag, Args...> ||
                has_invoke_default<Tag, Args...> ||
                requires(Base const& base, Args&&... args) {
                    base(HPX_FORWARD(Args, args)...);
                })
        HPX_HOST_DEVICE HPX_FORCEINLINE constexpr decltype(auto) operator()(
            Args&&... args) const
            noexcept(tag_dispatch_is_nothrow_v<Tag, Base, Args...>)
        {
            Tag const& tag = static_cast<Tag const&>(*this);
            if constexpr (has_hpx_invoke<Tag, Args...>)
            {
                return hpx_invoke(tag, HPX_FORWARD(Args, args)...);
            }
            // TODO(tag_invoke-removal): remove this branch in upcoming PR
            else if constexpr (has_legacy_tag_invoke<Tag, Args...>)
            {
                return tag_invoke(tag, HPX_FORWARD(Args, args)...);
            }
            else if constexpr (has_invoke_default<Tag, Args...>)
            {
                return Tag::invoke_default(HPX_FORWARD(Args, args)...);
            }
            else
            {
                return static_cast<Base const&>(*this)(
                    HPX_FORWARD(Args, args)...);
            }
        }
    };
}    // namespace hpx::detail
