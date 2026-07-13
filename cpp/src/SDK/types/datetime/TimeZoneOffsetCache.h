#pragma once

#include "dnv/vista/sdk/detail/types/datetime/Constants.h"
#include "dnv/vista/sdk/types/datetime/DateTime.h"
#include "dnv/vista/sdk/types/datetime/TimeSpan.h"

#include <atomic>
#include <chrono>
#include <ctime>
#include <mutex>

// Cross-platform time functions

/** @brief Cross-platform thread-safe time conversion functions */
#if defined(__MINGW32__) || defined(__MINGW64__)
    // MinGW uses Windows CRT gmtime/localtime (thread-safe on modern MinGW)
    /** @brief Thread-safe UTC time conversion wrapper for MinGW */
    #define DNV_VISTA_SDK_GMTIME_R(timer, result) ((*(result) = *gmtime(timer)), (result)->tm_year >= 0)
    /** @brief Thread-safe local time conversion wrapper for MinGW */
    #define DNV_VISTA_SDK_LOCALTIME_R(timer, result) ((*(result) = *localtime(timer)), (result)->tm_year >= 0)
#elif defined(_WIN32)
    // MSVC Windows uses gmtime_s and localtime_s (different parameter order than POSIX)
    /** @brief Thread-safe UTC time conversion wrapper for Windows gmtime_s */
    #define DNV_VISTA_SDK_GMTIME_R(timer, result) (gmtime_s((result), (timer)) == 0)
    /** @brief Thread-safe local time conversion wrapper for Windows localtime_s */
    #define DNV_VISTA_SDK_LOCALTIME_R(timer, result) (localtime_s((result), (timer)) == 0)
#elif defined(__GNUC__) || defined(__clang__)
    // POSIX systems use gmtime_r and localtime_r
    /** @brief Thread-safe UTC time conversion wrapper for POSIX gmtime_r */
    #define DNV_VISTA_SDK_GMTIME_R(timer, result) (gmtime_r((timer), (result)) != nullptr)
    /** @brief Thread-safe local time conversion wrapper for POSIX localtime_r */
    #define DNV_VISTA_SDK_LOCALTIME_R(timer, result) (localtime_r((timer), (result)) != nullptr)
#else
    // Fallback to non-thread-safe versions
    /** @brief Fallback UTC time conversion using non-thread-safe gmtime */
    #define DNV_VISTA_SDK_GMTIME_R(timer, result) ((*(result) = *gmtime(timer)), true)
    /** @brief Fallback local time conversion using non-thread-safe localtime */
    #define DNV_VISTA_SDK_LOCALTIME_R(timer, result) ((*(result) = *localtime(timer)), true)
#endif

namespace dnv::vista::sdk::internal
{
    /**
     * @brief Thread-safe timezone offset cache with hour-level granularity
     * @details Caches timezone offset per hour to handle DST transitions correctly.
     *          Uses atomic operations for lock-free reads in the common case.
     */
    class TimeZoneOffsetCache
    {
    public:
        TimeZoneOffsetCache() noexcept
            : m_cachedHour{ -1 },
              m_cachedOffsetSeconds{ 0 }
        {}

        /**
         * @brief Get timezone offset for given DateTime, using cache when valid
         * @param dateTime The DateTime to get timezone offset for
         * @return TimeSpan representing the timezone offset
         * @note Cache invalidates hourly to handle DST transitions correctly
         */
        TimeSpan offset(const DateTime& dateTime) noexcept
        {
            const auto year{ dateTime.year() };
            const auto month{ dateTime.month() };
            const auto day{ dateTime.day() };
            const auto hour{ dateTime.hour() };

            const std::int64_t currentHourKey{ year * 1000000LL + month * 10000 + day * 100 + hour };

            // Fast path: lock-free read
            if (m_cachedHour.load(std::memory_order_acquire) == currentHourKey)
                return TimeSpan::fromSeconds(
                    static_cast<double>(m_cachedOffsetSeconds.load(std::memory_order_acquire)));

            // Slow path: recompute under lock to avoid torn cache writes
            std::lock_guard<std::mutex> lock{ m_mutex };

            // Re-check after acquiring lock (another thread may have updated)
            if (m_cachedHour.load(std::memory_order_relaxed) == currentHourKey)
            {
                return TimeSpan::fromSeconds(
                    static_cast<double>(m_cachedOffsetSeconds.load(std::memory_order_relaxed)));
            }
            const auto newOffset{ computeOffset(dateTime) };
            m_cachedOffsetSeconds.store(newOffset, std::memory_order_relaxed);
            m_cachedHour.store(currentHourKey, std::memory_order_release);

            return TimeSpan::fromSeconds(static_cast<double>(newOffset));
        }

    private:
        std::mutex m_mutex;
        std::atomic<std::int64_t> m_cachedHour;          ///< Cached hour key (yyyymmddhh), -1 = invalid
        std::atomic<std::int64_t> m_cachedOffsetSeconds; ///< Cached offset in seconds

        /**
         * @brief Compute timezone offset for given DateTime
         * @param dateTime The DateTime to compute offset for
         * @return Offset in seconds
         */
        std::int64_t computeOffset(const DateTime& dateTime) noexcept
        {
            // Convert DateTime to Unix timestamp (seconds since 1970-01-01)
            const auto unixSeconds{ dateTime.toEpochSeconds() };
            const auto systemTimeT{ static_cast<std::time_t>(unixSeconds) };

            std::tm utcTm{};
            std::tm localTm{};

            const bool utcSuccess = DNV_VISTA_SDK_GMTIME_R(&systemTimeT, &utcTm);
            const bool localSuccess = DNV_VISTA_SDK_LOCALTIME_R(&systemTimeT, &localTm);

            if (!utcSuccess || !localSuccess)
            {
                return 0; // Fallback to UTC
            }

            const DateTime localAsUtc{ localTm.tm_year + 1900, localTm.tm_mon + 1, localTm.tm_mday,
                                       localTm.tm_hour,        localTm.tm_min,     localTm.tm_sec };

            return localAsUtc.toEpochSeconds() - unixSeconds;
        }
    };

    /**
     * @brief Get system timezone offset with caching
     * @param dateTime The DateTime to get timezone offset for
     * @return TimeSpan representing the timezone offset
     */
    inline TimeSpan systemTimezoneOffset(const DateTime& dateTime) noexcept
    {
        static TimeZoneOffsetCache cache;
        return cache.offset(dateTime);
    }
} // namespace dnv::vista::sdk::internal
