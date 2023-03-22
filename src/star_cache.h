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

#include "common/types.h"

namespace starrocks::starcache {

class StarCacheImpl;
class StarCache {
public:
    StarCache();
    ~StarCache();

    // Init the starcache instance with given options
    Status init(const CacheOptions& options);

    // Get the starcache options
    const CacheOptions* options();

    // Set the whole cache object.
    // If the `cache_key` exists, replace the cache data.
    // If the `ttl_seconds` is 0 (default), no ttl restriction will be set.
    Status set(const std::string& cache_key, const IOBuf& buf, uint64_t ttl_seconds = 0);

    // Get the whole cache object.
    // If no such object, return ENOENT error
    Status get(const std::string& cache_key, IOBuf* buf);

    // Read the partial cache object with given range.
    // If the range exceeds the object size, only read the range within the object size.
    // Only if all the data in the valid range exists in the cache will it return success, otherwise will return ENOENT.
    Status read(const std::string& cache_key, off_t offset, size_t size, IOBuf* buf);

    // Remove the cache object.
    // If no such object, return ENOENT error
    Status remove(const std::string& cache_key);

    // Set the cache object ttl.
    // If the `ttl_seconds` is 0, the origin ttl restriction will be removed.
    Status set_ttl(const std::string& cache_key, uint64_t ttl_seconds) { return Status::OK(); }

    // Pin the cache object in cache.  The cache object will not be evicted by eviction policy.
    // The object still can be removed in the cases:
    // 1. calling remove api
    // 2. the ttl is timeout
    Status pin(const std::string& cache_key);

    // UnPin the cache object in cache.
    // If the object is not exist or has been removed, return ENOENT error.
    Status unpin(const std::string& cache_key);

private:
    StarCacheImpl* _cache_impl = nullptr;
};

} // namespace starrocks::starcache
