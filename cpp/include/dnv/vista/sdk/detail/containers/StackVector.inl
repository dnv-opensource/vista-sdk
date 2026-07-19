#include <cstring>
#include <utility>

namespace dnv::vista::sdk
{
    template <typename T, size_t N>
    inline StackVector<T, N>::StackVector() noexcept
        : m_size{ 0 },
          m_usingStack{ true }
    {}

    template <typename T, size_t N>
    inline StackVector<T, N>::StackVector(std::initializer_list<T> init)
        : m_size{ 0 },
          m_usingStack{ true }
    {
        if (init.size() > N)
        {
            transitionToHeap();
            heapData().reserve(init.size());
        }
        for (const auto& val : init)
        {
            push_back(val);
        }
    }

    template <typename T, size_t N>
    inline StackVector<T, N>::StackVector(const StackVector& other)
        : m_size{ other.m_size },
          m_usingStack{ other.m_usingStack }
    {
        if (m_usingStack)
        {
            if constexpr (std::is_trivially_copyable_v<T>)
            {
                std::memcpy(m_stackStorage, other.m_stackStorage, m_size * sizeof(T));
            }
            else
            {
                for (size_type i = 0; i < m_size; ++i)
                {
                    new (&stackData()[i]) T(other.stackData()[i]);
                }
            }
        }
        else
        {
            new (&m_heapStorage) std::vector<T>(other.heapData());
        }
    }

    template <typename T, size_t N>
    inline StackVector<T, N>::StackVector(StackVector&& other) noexcept
        : m_size{ other.m_size },
          m_usingStack{ other.m_usingStack }
    {
        if (m_usingStack)
        {
            if constexpr (std::is_trivially_copyable_v<T>)
            {
                std::memcpy(m_stackStorage, other.m_stackStorage, m_size * sizeof(T));
            }
            else
            {
                for (size_type i = 0; i < m_size; ++i)
                {
                    new (&stackData()[i]) T(std::move(other.stackData()[i]));
                }
                if constexpr (!std::is_trivially_destructible_v<T>)
                {
                    for (size_type i = 0; i < m_size; ++i)
                    {
                        other.stackData()[i].~T();
                    }
                }
            }
        }
        else
        {
            new (&m_heapStorage) std::vector<T>(std::move(other.heapData()));
        }
        other.m_size = 0;
        other.m_usingStack = true;
    }

    template <typename T, size_t N>
    inline StackVector<T, N>::~StackVector() noexcept
    {
        if (m_usingStack)
        {
            if constexpr (!std::is_trivially_destructible_v<T>)
            {
                for (size_type i = 0; i < m_size; ++i)
                {
                    stackData()[i].~T();
                }
            }
        }
        else
        {
            heapData().~vector();
        }
    }

    template <typename T, size_t N>
    inline StackVector<T, N>& StackVector<T, N>::operator=(const StackVector& other)
    {
        if (this != &other)
        {
            if (m_usingStack)
            {
                if constexpr (!std::is_trivially_destructible_v<T>)
                {
                    for (size_type i = 0; i < m_size; ++i)
                    {
                        stackData()[i].~T();
                    }
                }
            }
            else
            {
                heapData().~vector();
            }

            m_size = other.m_size;
            m_usingStack = other.m_usingStack;

            if (m_usingStack)
            {
                if constexpr (std::is_trivially_copyable_v<T>)
                {
                    std::memcpy(m_stackStorage, other.m_stackStorage, m_size * sizeof(T));
                }
                else
                {
                    for (size_type i = 0; i < m_size; ++i)
                    {
                        new (&stackData()[i]) T(other.stackData()[i]);
                    }
                }
            }
            else
            {
                new (&m_heapStorage) std::vector<T>(other.heapData());
            }
        }
        return *this;
    }

    template <typename T, size_t N>
    inline StackVector<T, N>& StackVector<T, N>::operator=(StackVector&& other) noexcept
    {
        if (this != &other)
        {
            if (m_usingStack)
            {
                if constexpr (!std::is_trivially_destructible_v<T>)
                {
                    for (size_type i = 0; i < m_size; ++i)
                    {
                        stackData()[i].~T();
                    }
                }
            }
            else
            {
                heapData().~vector();
            }

            m_size = other.m_size;
            m_usingStack = other.m_usingStack;

            if (m_usingStack)
            {
                if constexpr (std::is_trivially_copyable_v<T>)
                {
                    std::memcpy(m_stackStorage, other.m_stackStorage, m_size * sizeof(T));
                }
                else
                {
                    for (size_type i = 0; i < m_size; ++i)
                    {
                        new (&stackData()[i]) T(std::move(other.stackData()[i]));
                    }
                    if constexpr (!std::is_trivially_destructible_v<T>)
                    {
                        for (size_type i = 0; i < m_size; ++i)
                        {
                            other.stackData()[i].~T();
                        }
                    }
                }
            }
            else
            {
                new (&m_heapStorage) std::vector<T>(std::move(other.heapData()));
            }
            other.m_size = 0;
            other.m_usingStack = true;
        }
        return *this;
    }

    template <typename T, size_t N>
    inline bool StackVector<T, N>::operator==(const StackVector& other) const noexcept
    {
        if (m_size != other.m_size)
        {
            return false;
        }
        return std::equal(begin(), end(), other.begin());
    }

    template <typename T, size_t N>
    inline void StackVector<T, N>::push_back(const T& value)
    {
        if (m_usingStack && m_size < N)
        {
            new (&stackData()[m_size]) T(value);
            ++m_size;
        }
        else
        {
            if (m_usingStack)
            {
                transitionToHeap();
            }
            heapData().push_back(value);
            m_size = heapData().size();
        }
    }

    template <typename T, size_t N>
    inline void StackVector<T, N>::push_back(T&& value)
    {
        if (m_usingStack && m_size < N)
        {
            new (&stackData()[m_size]) T(std::move(value));
            ++m_size;
        }
        else
        {
            if (m_usingStack)
            {
                transitionToHeap();
            }
            heapData().push_back(std::move(value));
            m_size = heapData().size();
        }
    }

    template <typename T, size_t N>
    template <typename... Args>
    inline void StackVector<T, N>::emplace_back(Args&&... args)
    {
        if (m_usingStack && m_size < N)
        {
            new (&stackData()[m_size]) T(std::forward<Args>(args)...);
            ++m_size;
        }
        else
        {
            if (m_usingStack)
            {
                transitionToHeap();
            }
            heapData().emplace_back(std::forward<Args>(args)...);
            m_size = heapData().size();
        }
    }

    template <typename T, size_t N>
    inline void StackVector<T, N>::pop_back() noexcept
    {
        if (m_usingStack)
        {
            if constexpr (!std::is_trivially_destructible_v<T>)
            {
                stackData()[m_size - 1].~T();
            }
            --m_size;
        }
        else
        {
            heapData().pop_back();
            --m_size;
        }
    }

    template <typename T, size_t N>
    inline void StackVector<T, N>::clear() noexcept
    {
        if (m_usingStack)
        {
            if constexpr (!std::is_trivially_destructible_v<T>)
            {
                for (size_type i = 0; i < m_size; ++i)
                {
                    stackData()[i].~T();
                }
            }
            m_size = 0;
        }
        else
        {
            heapData().clear();
            m_size = 0;
        }
    }

    template <typename T, size_t N>
    inline void StackVector<T, N>::resize(size_type count)
    {
        if (count < m_size)
        {
            if (m_usingStack)
            {
                if constexpr (!std::is_trivially_destructible_v<T>)
                {
                    for (size_type i = count; i < m_size; ++i)
                    {
                        stackData()[i].~T();
                    }
                }
            }
            else
            {
                heapData().resize(count);
            }
            m_size = count;
        }
        else if (count > m_size)
        {
            reserve(count);
            for (size_type i = m_size; i < count; ++i)
            {
                emplace_back();
            }
        }
    }

    template <typename T, size_t N>
    inline void StackVector<T, N>::resize(size_type count, const T& value)
    {
        if (count < m_size)
        {
            if (m_usingStack)
            {
                if constexpr (!std::is_trivially_destructible_v<T>)
                {
                    for (size_type i = count; i < m_size; ++i)
                    {
                        stackData()[i].~T();
                    }
                }
            }
            else
            {
                heapData().resize(count, value);
            }
            m_size = count;
        }
        else if (count > m_size)
        {
            reserve(count);
            for (size_type i = m_size; i < count; ++i)
            {
                push_back(value);
            }
        }
    }

    template <typename T, size_t N>
    inline void StackVector<T, N>::reserve(size_type newCapacity)
    {
        if (newCapacity > N && m_usingStack)
        {
            transitionToHeap();
            heapData().reserve(newCapacity);
        }
        else if (!m_usingStack)
        {
            heapData().reserve(newCapacity);
        }
    }

    template <typename T, size_t N>
    inline typename StackVector<T, N>::reference StackVector<T, N>::operator[](size_type pos) noexcept
    {
        return m_usingStack ? stackData()[pos] : heapData()[pos];
    }

    template <typename T, size_t N>
    inline typename StackVector<T, N>::const_reference StackVector<T, N>::operator[](size_type pos) const noexcept
    {
        return m_usingStack ? stackData()[pos] : heapData()[pos];
    }

    template <typename T, size_t N>
    inline typename StackVector<T, N>::reference StackVector<T, N>::at(size_type pos)
    {
        if (pos >= m_size)
        {
            throw std::out_of_range{ "StackVector::at: index out of range" };
        }
        return (*this)[pos];
    }

    template <typename T, size_t N>
    inline typename StackVector<T, N>::const_reference StackVector<T, N>::at(size_type pos) const
    {
        if (pos >= m_size)
        {
            throw std::out_of_range{ "StackVector::at: index out of range" };
        }
        return (*this)[pos];
    }

    template <typename T, size_t N>
    inline typename StackVector<T, N>::reference StackVector<T, N>::back() noexcept
    {
        return m_usingStack ? stackData()[m_size - 1] : heapData().back();
    }

    template <typename T, size_t N>
    inline typename StackVector<T, N>::const_reference StackVector<T, N>::back() const noexcept
    {
        return m_usingStack ? stackData()[m_size - 1] : heapData().back();
    }

    template <typename T, size_t N>
    inline typename StackVector<T, N>::reference StackVector<T, N>::front() noexcept
    {
        return m_usingStack ? stackData()[0] : heapData().front();
    }

    template <typename T, size_t N>
    inline typename StackVector<T, N>::const_reference StackVector<T, N>::front() const noexcept
    {
        return m_usingStack ? stackData()[0] : heapData().front();
    }

    template <typename T, size_t N>
    inline typename StackVector<T, N>::size_type StackVector<T, N>::size() const noexcept
    {
        return m_size;
    }

    template <typename T, size_t N>
    inline bool StackVector<T, N>::isEmpty() const noexcept
    {
        return m_size == 0;
    }

    template <typename T, size_t N>
    inline typename StackVector<T, N>::size_type StackVector<T, N>::capacity() const noexcept
    {
        return m_usingStack ? N : heapData().capacity();
    }

    template <typename T, size_t N>
    inline typename StackVector<T, N>::pointer StackVector<T, N>::data() noexcept
    {
        return m_usingStack ? stackData() : heapData().data();
    }

    template <typename T, size_t N>
    inline typename StackVector<T, N>::const_pointer StackVector<T, N>::data() const noexcept
    {
        return m_usingStack ? stackData() : heapData().data();
    }

    template <typename T, size_t N>
    inline typename StackVector<T, N>::pointer StackVector<T, N>::begin() noexcept
    {
        return data();
    }

    template <typename T, size_t N>
    inline typename StackVector<T, N>::const_pointer StackVector<T, N>::begin() const noexcept
    {
        return data();
    }

    template <typename T, size_t N>
    inline typename StackVector<T, N>::pointer StackVector<T, N>::end() noexcept
    {
        return data() + m_size;
    }

    template <typename T, size_t N>
    inline typename StackVector<T, N>::const_pointer StackVector<T, N>::end() const noexcept
    {
        return data() + m_size;
    }

    template <typename T, size_t N>
    inline typename StackVector<T, N>::const_pointer StackVector<T, N>::cbegin() const noexcept
    {
        return data();
    }

    template <typename T, size_t N>
    inline typename StackVector<T, N>::const_pointer StackVector<T, N>::cend() const noexcept
    {
        return data() + m_size;
    }

    template <typename T, size_t N>
    inline void StackVector<T, N>::transitionToHeap()
    {
        std::vector<T> vec;
        vec.reserve(N * 2);
        for (size_type i = 0; i < m_size; ++i)
        {
            vec.push_back(std::move(stackData()[i]));
        }

        if constexpr (!std::is_trivially_destructible_v<T>)
        {
            for (size_type i = 0; i < m_size; ++i)
            {
                stackData()[i].~T();
            }
        }

        new (&m_heapStorage) std::vector<T>(std::move(vec));
        m_usingStack = false;
    }

    template <typename T, size_t N>
    inline T* StackVector<T, N>::stackData() noexcept
    {
        return reinterpret_cast<T*>(m_stackStorage);
    }

    template <typename T, size_t N>
    inline const T* StackVector<T, N>::stackData() const noexcept
    {
        return reinterpret_cast<const T*>(m_stackStorage);
    }

    template <typename T, size_t N>
    inline std::vector<T>& StackVector<T, N>::heapData() noexcept
    {
        return *reinterpret_cast<std::vector<T>*>(&m_heapStorage);
    }

    template <typename T, size_t N>
    inline const std::vector<T>& StackVector<T, N>::heapData() const noexcept
    {
        return *reinterpret_cast<const std::vector<T>*>(&m_heapStorage);
    }
} // namespace dnv::vista::sdk
