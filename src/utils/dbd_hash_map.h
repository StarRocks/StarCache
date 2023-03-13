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

#include <butil/containers/doubly_buffered_data.h>
#include <parallel_hashmap/phmap.h>

#include <atomic>

namespace starrocks::starcache {

template <typename K, typename V>
class DBDHashMap {
public:
    bool add(const K& key, const V& value) {
        auto add_func = [&](Map& m, const K& k, const V& v) {
            auto ret = m.insert(std::make_pair(k, v));
            if (ret.second) {
                return 1;
            }
            return 0;
        };
        return _dbd_map.Modify(add_func, key, value) != 0;
    }

    void update(const K& key, const V& value) {
        auto add_func = [&](Map& m, const K& k, const V& v) {
            m[k] = v;
            return 1;
        };
        _dbd_map.Modify(add_func, key, value);
    }

    bool get(const K& key, V* value) {
        DBDMap::ScopedPtr ptr;
        if (_dbd_map.Read(&ptr) != 0) {
            return false;
        }

        auto iter = ptr->find(key);
        if (iter != ptr->end()) {
            *value = iter->second;
            return true;
        }
        return false;
    }

    bool remove(const K& key) {
        auto remove_func = [&](Map& m, const K& k) {
            size_t removed = m.erase(k);
            if (removed > 0) {
                return 1;
            }
            return 0;
        };
        return _dbd_map.Modify(remove_func, key) != 0;
    }

    void list(std::vector<K>* keys) {
        DBDMap::ScopedPtr ptr;
        if (_dbd_map.Read(&ptr) != 0) {
            return;
        }
        keys->reserve(ptr->size());
        for (auto iter = ptr->begin(); iter != ptr->end(); ++iter) {
            keys->push_back(iter->first);
        }
    }

private:
    typedef phmap::flat_hash_map<K, V> Map;
    typedef butil::DoublyBufferedData<Map> DBDMap;

    DBDMap _dbd_map;
};

} // namespace starrocks::starcache
