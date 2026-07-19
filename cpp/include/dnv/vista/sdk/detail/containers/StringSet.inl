#include <utility>

namespace dnv::vista::sdk
{
    inline StringSet::StringSet()
    {
        m_buckets.resize(INITIAL_CAPACITY);
    }

    inline StringSet::StringSet(std::initializer_list<std::string> init)
        : StringSet{}
    {
        reserve(init.size());
        for (const auto& key : init)
        {
            insert(key);
        }
    }

    template <typename K>
    inline uint32_t StringSet::hashOf(const K& key) const noexcept
    {
        return internal::hashString(std::string_view{ key });
    }

    inline bool StringSet::shouldResize() const noexcept
    {
        return (m_size + 1) * 100 >= m_buckets.size() * MAX_LOAD_FACTOR_PERCENT;
    }

    inline void StringSet::resize()
    {
        std::vector<Bucket> oldBuckets = std::move(m_buckets);

        m_buckets.clear();
        m_buckets.resize(oldBuckets.size() * 2);
        m_mask = m_buckets.size() - 1;
        m_size = 0;

        for (auto& bucket : oldBuckets)
        {
            if (bucket.occupied)
            {
                insertHashed(std::move(*bucket.key), bucket.hash);
            }
        }
    }

    inline bool StringSet::insertHashed(std::string&& key, uint32_t hash)
    {
        size_t pos = static_cast<size_t>(hash) & m_mask;
        uint32_t distance = 0;
        Bucket incoming{ std::move(key), hash, distance, true };

        for (;;)
        {
            Bucket& slot = m_buckets[pos];

            if (!slot.occupied)
            {
                slot = std::move(incoming);
                ++m_size;
                return true;
            }

            if (slot.hash == incoming.hash && *slot.key == *incoming.key)
            {
                return false;
            }

            if (slot.distance < incoming.distance)
            {
                std::swap(slot, incoming);
            }

            pos = (pos + 1) & m_mask;
            ++incoming.distance;
        }
    }

    template <typename K>
    inline bool StringSet::insert(const K& key)
    {
        if (shouldResize())
        {
            resize();
        }

        const uint32_t hash = hashOf(key);
        return insertHashed(std::string{ key }, hash);
    }

    inline bool StringSet::insert(std::string&& key)
    {
        if (shouldResize())
        {
            resize();
        }

        const uint32_t hash = hashOf(key);
        return insertHashed(std::move(key), hash);
    }

    template <typename K>
    inline bool StringSet::contains(const K& key) const noexcept
    {
        return find(key) != nullptr;
    }

    template <typename K>
    inline const std::string* StringSet::find(const K& key) const noexcept
    {
        if (m_buckets.empty())
        {
            return nullptr;
        }

        const uint32_t hash = hashOf(key);
        size_t pos = static_cast<size_t>(hash) & m_mask;
        uint32_t distance = 0;

        for (;;)
        {
            const Bucket& slot = m_buckets[pos];

            if (!slot.occupied || distance > slot.distance)
            {
                return nullptr;
            }

            if (slot.hash == hash && *slot.key == key)
            {
                return &*slot.key;
            }

            pos = (pos + 1) & m_mask;
            ++distance;
        }
    }

    template <typename K>
    inline bool StringSet::erase(const K& key)
    {
        if (m_buckets.empty())
        {
            return false;
        }

        const uint32_t hash = hashOf(key);
        size_t pos = static_cast<size_t>(hash) & m_mask;
        uint32_t distance = 0;

        for (;;)
        {
            Bucket& slot = m_buckets[pos];

            if (!slot.occupied || distance > slot.distance)
            {
                return false;
            }

            if (slot.hash == hash && *slot.key == key)
            {
                break;
            }

            pos = (pos + 1) & m_mask;
            ++distance;
        }

        size_t current = pos;
        for (;;)
        {
            const size_t next = (current + 1) & m_mask;
            Bucket& nextSlot = m_buckets[next];

            if (!nextSlot.occupied || nextSlot.distance == 0)
            {
                m_buckets[current] = Bucket{};
                break;
            }

            m_buckets[current] = std::move(nextSlot);
            --m_buckets[current].distance;
            nextSlot = Bucket{};

            current = next;
        }

        --m_size;
        return true;
    }

    inline void StringSet::reserve(size_t minCapacity)
    {
        size_t newCapacity = m_buckets.empty() ? INITIAL_CAPACITY : m_buckets.size();
        while (newCapacity * MAX_LOAD_FACTOR_PERCENT < minCapacity * 100)
        {
            newCapacity *= 2;
        }

        if (newCapacity <= m_buckets.size())
        {
            return;
        }

        std::vector<Bucket> oldBuckets = std::move(m_buckets);
        m_buckets.clear();
        m_buckets.resize(newCapacity);
        m_mask = newCapacity - 1;
        m_size = 0;

        for (auto& bucket : oldBuckets)
        {
            if (bucket.occupied)
            {
                insertHashed(std::move(*bucket.key), bucket.hash);
            }
        }
    }

    inline size_t StringSet::size() const noexcept
    {
        return m_size;
    }

    inline bool StringSet::isEmpty() const noexcept
    {
        return m_size == 0;
    }

    inline StringSet::Iterator StringSet::begin() const noexcept
    {
        return Iterator{ &m_buckets, 0 };
    }

    inline StringSet::Iterator StringSet::end() const noexcept
    {
        return Iterator{ &m_buckets, m_buckets.size() };
    }

    inline StringSet::Iterator StringSet::cbegin() const noexcept
    {
        return begin();
    }

    inline StringSet::Iterator StringSet::cend() const noexcept
    {
        return end();
    }

    inline StringSet::Iterator::Iterator(const std::vector<Bucket>* buckets, size_t index)
        : m_buckets{ buckets },
          m_index{ index }
    {
        skipEmpty();
    }

    inline StringSet::Iterator::reference StringSet::Iterator::operator*() const
    {
        return *(*m_buckets)[m_index].key;
    }

    inline StringSet::Iterator::pointer StringSet::Iterator::operator->() const
    {
        return &*(*m_buckets)[m_index].key;
    }

    inline StringSet::Iterator& StringSet::Iterator::operator++()
    {
        ++m_index;
        skipEmpty();
        return *this;
    }

    inline StringSet::Iterator StringSet::Iterator::operator++(int)
    {
        Iterator temp = *this;
        ++m_index;
        skipEmpty();
        return temp;
    }

    inline bool StringSet::Iterator::operator==(const Iterator& other) const
    {
        return m_index == other.m_index;
    }

    inline void StringSet::Iterator::skipEmpty()
    {
        while (m_index < m_buckets->size() && !(*m_buckets)[m_index].occupied)
        {
            ++m_index;
        }
    }
} // namespace dnv::vista::sdk
