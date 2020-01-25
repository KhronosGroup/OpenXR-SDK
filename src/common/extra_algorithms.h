// Copyright (c) 2017-2020 The Khronos Group Inc.
// Copyright (c) 2017-2019 Valve Corporation
// Copyright (c) 2017-2019 LunarG, Inc.
// Copyright (c) 2019 Collabora, Ltd.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Author: Ryan Pavlik <ryan.pavlik@collabora.com>
//

/*!
 * @file
 *
 * Additional functions along the lines of the standard library algorithms.
 */

#pragma once

#include <algorithm>
#include <vector>

/// Like std::remove_if, except it works on associative containers and it actually removes this.
///
/// The iterator stuff in here is subtle - .erase() invalidates only that iterator, but it returns a non-invalidated iterator to the
/// next valid element which we can use instead of incrementing.
template <typename T, typename Pred>
static inline void map_erase_if(T &container, Pred &&predicate) {
    for (auto it = container.begin(); it != container.end();) {
        if (predicate(*it)) {
            it = container.erase(it);
        } else {
            ++it;
        }
    }
}

/*!
 * Moves all elements matching the predicate to the end of the vector then erases them.
 *
 * Combines the two parts of the erase-remove idiom to simplify things and avoid accidentally using the wrong erase overload.
 */
template <typename T, typename Alloc, typename Pred>
static inline void vector_remove_if_and_erase(std::vector<T, Alloc> &vec, Pred &&predicate) {
    auto b = vec.begin();
    auto e = vec.end();
    vec.erase(std::remove_if(b, e, std::forward<Pred>(predicate)), e);
}
