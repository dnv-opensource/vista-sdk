namespace dnv::vista::sdk
{
    template <typename TValue>
    inline PerfectHashMap<TValue>::PerfectHashMap(std::vector<std::pair<std::string, TValue>>&& items)
        : m_itemCount{ items.size() }
    {
        const size_t itemCount = items.size();

        if (itemCount == 0)
        {
            return;
        }

        {
            std::unordered_set<std::string_view> seen;
            seen.reserve(itemCount);
            for (size_t i = 0; i < itemCount; ++i)
            {
                if (!seen.insert(items[i].first).second)
                {
                    throw std::invalid_argument{ "PerfectHashMap: duplicate keys detected" };
                }
            }
        }

        size_t tableSize = 1;
        while (tableSize < itemCount)
        {
            tableSize <<= 1;
        }
        tableSize <<= 1;

        m_table.resize(tableSize);
        m_seeds.resize(tableSize, 0);

        std::vector<std::vector<std::pair<size_t, std::uint32_t>>> buckets(tableSize);
        for (size_t i = 0; i < itemCount; ++i)
        {
            std::uint32_t h = internal::hashString(items[i].first);
            buckets[static_cast<size_t>(h) & (tableSize - 1)].emplace_back(i, h);
        }

        std::vector<std::pair<size_t, std::vector<std::pair<size_t, std::uint32_t>>*>> indexedBuckets;
        indexedBuckets.reserve(itemCount);
        for (size_t i = 0; i < tableSize; ++i)
        {
            if (!buckets[i].empty())
            {
                indexedBuckets.emplace_back(i, &buckets[i]);
            }
        }

        std::sort(indexedBuckets.begin(), indexedBuckets.end(), [](const auto& a, const auto& b) {
            return a.second->size() > b.second->size();
        });

        for (const auto& [bucketIndex, bucketPtr] : indexedBuckets)
        {
            const auto& bucket = *bucketPtr;

            if (bucket.size() == 1)
            {
                m_table[bucketIndex].emplace(std::move(items[bucket[0].first]));
                m_seeds[bucketIndex] = -static_cast<std::int32_t>(bucketIndex + 1);
            }
            else
            {
                const size_t originalBucketIndex = bucket[0].second & (tableSize - 1);
                bool seedFound = false;

                for (std::uint32_t seed = 1; !seedFound; ++seed)
                {
                    if (seed == 0)
                    {
                        throw std::runtime_error{
                            "PerfectHashMap: seed search exhausted uint32_t range without finding a collision-free "
                            "assignment"
                        };
                    }

                    std::vector<size_t> positions;
                    positions.reserve(bucket.size());
                    bool collision = false;

                    for (const auto& [itemIndex, hashValue] : bucket)
                    {
                        size_t pos = internal::seedMix(seed, hashValue, tableSize);

                        if (m_table[pos].has_value() || (!buckets[pos].empty() && buckets[pos].size() == 1) ||
                            std::find(positions.begin(), positions.end(), pos) != positions.end())
                        {
                            collision = true;
                            break;
                        }

                        positions.push_back(pos);
                    }

                    if (!collision)
                    {
                        for (size_t i = 0; i < bucket.size(); ++i)
                        {
                            m_table[positions[i]].emplace(std::move(items[bucket[i].first]));
                        }
                        m_seeds[originalBucketIndex] = static_cast<std::int32_t>(seed);
                        seedFound = true;
                    }
                }
            }
        }
    }

    template <typename TValue>
    inline bool PerfectHashMap<TValue>::operator==(const PerfectHashMap& other) const noexcept
    {
        if (count() != other.count())
        {
            return false;
        }
        if (isEmpty())
        {
            return true;
        }
        for (const auto& [key, value] : *this)
        {
            const TValue* otherValue = other.find(key);
            if (!otherValue || value != *otherValue)
            {
                return false;
            }
        }
        return true;
    }

    template <typename TValue>
    template <typename K>
    inline const TValue& PerfectHashMap<TValue>::at(const K& key) const
    {
        if (const TValue* value = find(key))
        {
            return *value;
        }
        throw std::out_of_range{ "PerfectHashMap::at: key not found" };
    }

    template <typename TValue>
    template <typename K>
    inline bool PerfectHashMap<TValue>::contains(const K& key) const noexcept
    {
        if (m_table.empty())
        {
            return false;
        }

        const size_t tableSize = m_table.size();
        const std::uint32_t h = internal::hashString(std::string_view{ key });
        const size_t bucketIndex = static_cast<size_t>(h) & (tableSize - 1);
        const std::int32_t seed = m_seeds[bucketIndex];

        const size_t pos = (seed < 0) ? static_cast<size_t>(-seed - 1)
                                      : internal::seedMix(static_cast<std::uint32_t>(seed), h, tableSize);

        return m_table[pos].has_value() && m_table[pos]->first == key;
    }

    template <typename TValue>
    template <typename K>
    inline const TValue* PerfectHashMap<TValue>::find(const K& key) const noexcept
    {
        if (m_table.empty())
        {
            return nullptr;
        }

        const size_t tableSize = m_table.size();
        const std::uint32_t h = internal::hashString(std::string_view{ key });
        const size_t bucketIndex = static_cast<size_t>(h) & (tableSize - 1);
        const std::int32_t seed = m_seeds[bucketIndex];

        const size_t pos = (seed < 0) ? static_cast<size_t>(-seed - 1)
                                      : internal::seedMix(static_cast<std::uint32_t>(seed), h, tableSize);

        if (m_table[pos].has_value() && m_table[pos]->first == key)
        {
            return &m_table[pos]->second;
        }

        return nullptr;
    }

    template <typename TValue>
    inline PerfectHashMap<TValue>::size_type PerfectHashMap<TValue>::size() const noexcept
    {
        return m_table.size();
    }

    template <typename TValue>
    inline PerfectHashMap<TValue>::size_type PerfectHashMap<TValue>::count() const noexcept
    {
        return m_itemCount;
    }

    template <typename TValue>
    inline bool PerfectHashMap<TValue>::isEmpty() const noexcept
    {
        return m_itemCount == 0;
    }

    template <typename TValue>
    inline PerfectHashMap<TValue>::Iterator PerfectHashMap<TValue>::begin() const noexcept
    {
        return Iterator{ &m_table, 0 };
    }

    template <typename TValue>
    inline PerfectHashMap<TValue>::Iterator PerfectHashMap<TValue>::end() const noexcept
    {
        return Iterator{ &m_table, m_table.size() };
    }

    template <typename TValue>
    inline PerfectHashMap<TValue>::Iterator PerfectHashMap<TValue>::cbegin() const noexcept
    {
        return begin();
    }

    template <typename TValue>
    inline PerfectHashMap<TValue>::Iterator PerfectHashMap<TValue>::cend() const noexcept
    {
        return end();
    }

    template <typename TValue>
    inline PerfectHashMap<TValue>::Iterator::Iterator(
        const std::vector<std::optional<std::pair<std::string, TValue>>>* table, size_t index)
        : m_table{ table },
          m_index{ index }
    {
        skipEmpty();
    }

    template <typename TValue>
    inline PerfectHashMap<TValue>::Iterator::reference PerfectHashMap<TValue>::Iterator::operator*() const
    {
        return *(*m_table)[m_index];
    }

    template <typename TValue>
    inline PerfectHashMap<TValue>::Iterator::pointer PerfectHashMap<TValue>::Iterator::operator->() const
    {
        return &*(*m_table)[m_index];
    }

    template <typename TValue>
    inline PerfectHashMap<TValue>::Iterator& PerfectHashMap<TValue>::Iterator::operator++()
    {
        ++m_index;
        skipEmpty();
        return *this;
    }

    template <typename TValue>
    inline PerfectHashMap<TValue>::Iterator PerfectHashMap<TValue>::Iterator::operator++(int)
    {
        Iterator temp = *this;
        ++m_index;
        skipEmpty();
        return temp;
    }

    template <typename TValue>
    inline bool PerfectHashMap<TValue>::Iterator::operator==(const Iterator& other) const
    {
        return m_index == other.m_index;
    }

    template <typename TValue>
    inline void PerfectHashMap<TValue>::Iterator::skipEmpty()
    {
        while (m_index < m_table->size() && !(*m_table)[m_index].has_value())
        {
            ++m_index;
        }
    }
} // namespace dnv::vista::sdk
