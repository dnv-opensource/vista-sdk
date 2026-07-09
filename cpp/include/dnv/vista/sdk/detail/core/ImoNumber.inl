namespace dnv::vista::sdk
{
    inline bool ImoNumber::operator==(const ImoNumber& other) const noexcept
    {
        return m_value == other.m_value;
    }

    inline constexpr bool ImoNumber::isValid(int imoNumber) noexcept
    {
        if (imoNumber < 1000000 || imoNumber > 9999999)
        {
            return false;
        }

        // IMO Resolution A.1078(28): multiply each of the first 6 digits left-to-right
        // by 7, 6, 5, 4, 3, 2 respectively. Check digit = sum % 10
        int digits[7];
        int temp = imoNumber;
        for (int i = 6; i >= 0; --i)
        {
            digits[i] = temp % 10;
            temp /= 10;
        }

        int checkSum = 0;
        for (int i = 0; i < 6; ++i)
        {
            checkSum += digits[i] * (7 - i);
        }

        return digits[6] == checkSum % 10;
    }

    inline constexpr int ImoNumber::value() const noexcept
    {
        return m_value;
    }
} // namespace dnv::vista::sdk
