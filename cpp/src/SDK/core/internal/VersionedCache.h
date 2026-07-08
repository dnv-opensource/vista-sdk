/**
 * @file VersionedCache.h
 * @brief Thread-safe lazy-loading cache keyed by a version enum
 */

#pragma once

#include <mutex>
#include <shared_mutex>
#include <unordered_map>

namespace dnv::vista::sdk::internal
{
    /**
     * @brief Thread-safe cache of values keyed by a version enum
     * @tparam Key Version enum type used as the cache key
     * @tparam Value Cached value type
     */
    template <typename Key, typename Value>
    struct VersionedCache
    {
        std::shared_mutex mutex;
        std::unordered_map<Key, Value> entries;
    };

    /**
     * @brief Get a cached value for a key, loading and caching it if absent
     * @param cache The cache to look up and populate
     * @param key The version key to look up
     * @param load Function invoked with key to produce the value on a cache miss
     * @return Reference to the cached value
     */
    template <typename Key, typename Value, typename LoadFn>
    const Value& getOrLoad(VersionedCache<Key, Value>& cache, Key key, LoadFn load)
    {
        {
            std::shared_lock lock(cache.mutex);
            auto it = cache.entries.find(key);
            if (it != cache.entries.end())
            {
                return it->second;
            }
        }

        std::unique_lock lock(cache.mutex);

        auto it = cache.entries.find(key);
        if (it != cache.entries.end())
        {
            return it->second;
        }

        auto [emplacedIt, _] = cache.entries.emplace(key, load(key));

        return emplacedIt->second;
    }
} // namespace dnv::vista::sdk::internal
