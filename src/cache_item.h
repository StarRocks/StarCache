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

#include <atomic>
#include <shared_mutex>

#include "block_item.h"

namespace starrocks::starcache {

enum CacheState { IN_MEM, IN_DISK, RELEASED };

struct CacheItem {
    // An unique id generated internally
    // CacheId cache_id;
    // A key string passed by api
    CacheKey cache_key;
    // The block lists belong to this cache item
    BlockItem* blocks;
    // The cache object size
    size_t size;
    // The expire time of this cache item
    uint64_t expire_time;

    CacheItem() = default;
    CacheItem(const CacheKey& cache_key_, size_t size_, uint64_t expire_time_)
            : cache_key(cache_key_), size(size_), expire_time(expire_time_), _state(0) {
        blocks = new BlockItem[block_count()];
    }
    ~CacheItem() { delete[] blocks; }

    size_t block_count() {
        if (size == 0) {
            return 0;
        }
        return (size - 1) / config::FLAGS_block_size + 1;
    }

    size_t block_size(uint32_t block_index) {
        int64_t tail_size = size - block_index * config::FLAGS_block_size;
        DCHECK(tail_size >= 0);
        return tail_size < config::FLAGS_block_size ? tail_size : config::FLAGS_block_size;
    }

    void set_state(const CacheState& state) {
        std::unique_lock<std::shared_mutex> wlck(_mutex);
        _state |= (1ul << state);
    }

    void reset_state(const CacheState& state) {
        std::unique_lock<std::shared_mutex> wlck(_mutex);
        _state &= ~(1ul << state);
    }

    uint8_t state() {
        std::shared_lock<std::shared_mutex> rlck(_mutex);
        return _state;
    }

    bool is_in_mem() {
        std::shared_lock<std::shared_mutex> rlck(_mutex);
        return _state & (1ul << IN_MEM);
    }

    bool is_in_disk() {
        std::shared_lock<std::shared_mutex> rlck(_mutex);
        return _state & (1ul << IN_DISK);
    }

    void clear_mem_blocks() {
        std::shared_lock<std::shared_mutex> rlck(_mutex);
        for (size_t i = 0; i < block_count(); ++i) {
            blocks[i].mem_block_item = nullptr;
        }
    }

    void clear_disk_blocks() {
        std::shared_lock<std::shared_mutex> rlck(_mutex);
        for (size_t i = 0; i < block_count(); ++i) {
            blocks[i].disk_block_item = nullptr;
        }
    }

    bool is_released() {
        std::shared_lock<std::shared_mutex> rlck(_mutex);
        return _state & (1ul << RELEASED);
    }

    bool release() {
        std::unique_lock<std::shared_mutex> wlck(_mutex);
        if (_state & (1ul << RELEASED)) {
            return false;
        }
        _state |= (1ul << RELEASED);
        return true;
    }

    bool release_if_empty() {
        std::unique_lock<std::shared_mutex> wlck(_mutex);
        if (_state & (1ul << RELEASED)) {
            return false;
        }
        for (size_t i = 0; i < block_count(); ++i) {
            if (blocks[i].mem_block_item || blocks[i].disk_block_item) {
                return false;
            }
        }
        // All blocks are empty, release the cache item
        _state |= (1ul << RELEASED);
        return true;
    }

private:
    // Indicate current state
    uint8_t _state;
    std::shared_mutex _mutex;
};

using CacheItemPtr = std::shared_ptr<CacheItem>;

} // namespace starrocks::starcache
