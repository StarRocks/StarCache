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

#include "star_cache.h"

#include "star_cache_impl.h"

namespace starrocks::starcache {

StarCache::StarCache() {
    _cache_impl = new StarCacheImpl;
}

const CacheOptions* StarCache::options() {
    return _cache_impl->options();
}

StarCache::~StarCache() {
    delete _cache_impl;
}

Status StarCache::init(const CacheOptions& options) {
    return _cache_impl->init(options);
}

Status StarCache::set(const CacheKey& cache_key, const IOBuf& buf, WriteOptions* options) {
    return _cache_impl->set(cache_key, buf, options);
}

Status StarCache::get(const CacheKey& cache_key, IOBuf* buf, ReadOptions* options) {
    return _cache_impl->get(cache_key, buf, options);
}

Status StarCache::read(const CacheKey& cache_key, off_t offset, size_t size, IOBuf* buf, ReadOptions* options) {
    return _cache_impl->read(cache_key, offset, size, buf, options);
}

Status StarCache::remove(const CacheKey& cache_key) {
    return _cache_impl->remove(cache_key);
}

Status StarCache::pin(const CacheKey& cache_key) {
    return _cache_impl->pin(cache_key);
}

Status StarCache::unpin(const CacheKey& cache_key) {
    return _cache_impl->unpin(cache_key);
}

} // namespace starrocks::starcache
