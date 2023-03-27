// Copyright 2023-present StarRocks, Inc. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include <shared_mutex>

namespace starrocks::starcache {

template <typename T>
class EvictionPolicy {
public:
    class Handle {
    public:
        Handle(EvictionPolicy* policy, void* hdl) : _policy(policy), _hdl(hdl) {}
        ~Handle() {
            release();
        }

        void release() {
            if (_hdl && _policy) {
                _policy->release(_hdl);
                _hdl = nullptr;
            }
        }

    private:
        EvictionPolicy* _policy;
        void* _hdl;
    };

    using HandlePtr = std::shared_ptr<Handle>;

    virtual ~EvictionPolicy() = default;

    // Add the given id to the eviction component
    virtual bool add(const T& id, size_t size) = 0;

    // Record the hit of the id
    virtual HandlePtr touch(const T& id) = 0;

    // Evict some items in current component
    virtual void evict(size_t count, std::vector<T>* evicted) = 0;

    // Evict some items in current component to store given `id`
    // This function is useful when evicting some items which are devided into
    // different buckets by hash of keys.
    virtual void evict_for(const T& id, size_t count, std::vector<T>* evicted) = 0;

    // Release the item handle returned by touch
    virtual void release(void* hdl) = 0;

    // Remove the given id from the eviction component, used for `pin` function.
    virtual void remove(const T& id) = 0;

    // Clear all items in the eviction component
    virtual void clear() = 0;
};

} // namespace starrocks::starcache
