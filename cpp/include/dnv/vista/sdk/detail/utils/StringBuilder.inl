#pragma once

namespace dnv::vista::sdk
{
    inline StringBuilder::StringBuilder() noexcept
        : m_heapBuffer{ nullptr },
          m_buffer{ m_stackBuffer },
          m_size{ 0 },
          m_capacity{ STACK_BUFFER_SIZE }
    {}

    inline StringBuilder::StringBuilder(size_t initialCapacity)
        : m_heapBuffer{ nullptr },
          m_buffer{ m_stackBuffer },
          m_size{ 0 },
          m_capacity{ STACK_BUFFER_SIZE }
    {
        if (initialCapacity > STACK_BUFFER_SIZE)
        {
            m_heapBuffer = std::make_unique<char[]>(initialCapacity);
            m_buffer = m_heapBuffer.get();
            m_capacity = initialCapacity;
        }
    }

    inline StringBuilder::StringBuilder(const StringBuilder& other)
        : m_heapBuffer{ nullptr },
          m_buffer{ m_stackBuffer },
          m_size{ other.m_size },
          m_capacity{ other.m_capacity }
    {
        if (other.m_heapBuffer)
        {
            m_heapBuffer = std::make_unique<char[]>(m_capacity);
            m_buffer = m_heapBuffer.get();
        }
        else
        {
            m_capacity = STACK_BUFFER_SIZE;
        }
        std::memcpy(m_buffer, other.m_buffer, m_size);
    }

    inline StringBuilder::StringBuilder(StringBuilder&& other) noexcept
        : m_heapBuffer{ std::move(other.m_heapBuffer) },
          m_buffer{ nullptr },
          m_size{ other.m_size },
          m_capacity{ other.m_capacity }
    {
        if (m_heapBuffer)
        {
            m_buffer = m_heapBuffer.get();
        }
        else
        {
            m_buffer = m_stackBuffer;
            std::memcpy(m_stackBuffer, other.m_stackBuffer, m_size);
        }

        other.m_buffer = other.m_stackBuffer;
        other.m_size = 0;
        other.m_capacity = STACK_BUFFER_SIZE;
    }

    inline StringBuilder& StringBuilder::operator=(const StringBuilder& other)
    {
        if (this != &other)
        {
            if (other.m_heapBuffer)
            {
                if (!m_heapBuffer || m_capacity < other.m_capacity)
                {
                    m_heapBuffer = std::make_unique<char[]>(other.m_capacity);
                    m_buffer = m_heapBuffer.get();
                    m_capacity = other.m_capacity;
                }
            }
            else if (m_heapBuffer)
            {
                m_heapBuffer.reset();
                m_buffer = m_stackBuffer;
                m_capacity = STACK_BUFFER_SIZE;
            }
            std::memcpy(m_buffer, other.m_buffer, other.m_size);
            m_size = other.m_size;
        }
        return *this;
    }

    inline StringBuilder& StringBuilder::operator=(StringBuilder&& other) noexcept
    {
        if (this != &other)
        {
            m_heapBuffer = std::move(other.m_heapBuffer);
            m_size = other.m_size;
            m_capacity = other.m_capacity;

            if (m_heapBuffer)
            {
                m_buffer = m_heapBuffer.get();
            }
            else
            {
                std::memcpy(m_stackBuffer, other.m_stackBuffer, m_size);
                m_buffer = m_stackBuffer;
            }

            other.m_buffer = other.m_stackBuffer;
            other.m_size = 0;
            other.m_capacity = STACK_BUFFER_SIZE;
        }
        return *this;
    }

    inline StringBuilder& StringBuilder::operator+=(std::string_view str)
    {
        return append(str);
    }

    inline StringBuilder& StringBuilder::operator+=(char c)
    {
        return append(c);
    }

    inline size_t StringBuilder::size() const noexcept
    {
        return m_size;
    }

    inline size_t StringBuilder::capacity() const noexcept
    {
        return m_capacity;
    }

    inline bool StringBuilder::isEmpty() const noexcept
    {
        return m_size == 0;
    }

    inline const char* StringBuilder::data() const noexcept
    {
        return m_buffer;
    }

    inline std::string_view StringBuilder::view() const noexcept
    {
        return std::string_view{ m_buffer, m_size };
    }

    inline std::string StringBuilder::toString() const
    {
        return std::string{ m_buffer, m_size };
    }

    inline char& StringBuilder::back() noexcept
    {
        return m_buffer[m_size - 1];
    }

    inline char StringBuilder::back() const noexcept
    {
        return m_buffer[m_size - 1];
    }

    inline void StringBuilder::pop_back() noexcept
    {
        --m_size;
    }

    inline void StringBuilder::ensureCapacity(size_t needed)
    {
        if (needed <= m_capacity)
        {
            return;
        }

        constexpr size_t growthThreshold = 8192;
        size_t newCapacity;
        if (m_capacity < growthThreshold)
        {
            newCapacity = std::max(needed, m_capacity * 2);
        }
        else
        {
            newCapacity = std::max(needed, m_capacity + (m_capacity / 2));
        }

        auto newBuffer = std::make_unique<char[]>(newCapacity);
        std::memcpy(newBuffer.get(), m_buffer, m_size);
        m_heapBuffer = std::move(newBuffer);
        m_buffer = m_heapBuffer.get();
        m_capacity = newCapacity;
    }

    inline StringBuilder& StringBuilder::reserve(size_t minCapacity)
    {
        ensureCapacity(minCapacity);
        return *this;
    }

    inline StringBuilder& StringBuilder::append(std::string_view str)
    {
        if (str.empty())
        {
            return *this;
        }

        const size_t newSize = m_size + str.size();
        if (newSize > m_capacity)
        {
            ensureCapacity(newSize);
        }

        std::memcpy(m_buffer + m_size, str.data(), str.size());
        m_size = newSize;
        return *this;
    }

    inline StringBuilder& StringBuilder::append(char c)
    {
        const size_t newSize = m_size + 1;
        if (newSize > m_capacity)
        {
            ensureCapacity(newSize);
        }

        m_buffer[m_size] = c;
        m_size = newSize;
        return *this;
    }

    inline StringBuilder& StringBuilder::append(std::int64_t value)
    {
        ensureCapacity(m_size + MAX_NUMERIC_CHARS);

        auto [ptr, ec] = std::to_chars(m_buffer + m_size, m_buffer + m_capacity, value);
        if (ec == std::errc{})
        {
            m_size = static_cast<size_t>(ptr - m_buffer);
        }
        return *this;
    }

    inline StringBuilder& StringBuilder::append(std::uint64_t value)
    {
        ensureCapacity(m_size + MAX_NUMERIC_CHARS);

        auto [ptr, ec] = std::to_chars(m_buffer + m_size, m_buffer + m_capacity, value);
        if (ec == std::errc{})
        {
            m_size = static_cast<size_t>(ptr - m_buffer);
        }
        return *this;
    }

    inline StringBuilder& StringBuilder::append(double value)
    {
        ensureCapacity(m_size + MAX_NUMERIC_CHARS);

        auto [ptr, ec] = std::to_chars(m_buffer + m_size, m_buffer + m_capacity, value);
        if (ec == std::errc{})
        {
            m_size = static_cast<size_t>(ptr - m_buffer);
        }
        return *this;
    }

    inline StringBuilder& StringBuilder::appendRepeated(char c, size_t count)
    {
        if (count == 0)
        {
            return *this;
        }

        const size_t newSize = m_size + count;
        if (newSize > m_capacity)
        {
            ensureCapacity(newSize);
        }

        std::memset(m_buffer + m_size, c, count);
        m_size = newSize;
        return *this;
    }

    inline void StringBuilder::clear(bool toStack) noexcept
    {
        if (toStack && m_heapBuffer)
        {
            m_heapBuffer.reset();
            m_buffer = m_stackBuffer;
            m_capacity = STACK_BUFFER_SIZE;
        }
        m_size = 0;
    }
} // namespace dnv::vista::sdk
