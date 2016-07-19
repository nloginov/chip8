#pragma once
template <size_t TFrom, size_t TTo>
std::conditional_t<(TTo - TFrom > 1), uint16_t, uint8_t> GetOctetsRange(uint16_t word)
{
    static_assert(TFrom <= TTo, "TFrom must be not greater than TTo");
    static_assert((1 <= TFrom && TFrom <= 4) && (1 <= TTo && TTo <= 4), "Octet number must be in range from 1 to 4");

    uint16_t mask = 0x0F;
    for(size_t i = TFrom; i < TTo; ++i) {
        mask = (mask << 4) | 0x0F;
    }

    mask = mask << 4*(TFrom - 1);
    return (word & mask) >> 4*(TFrom - 1);
}

template <size_t TNumber>
uint8_t GetOctetAt(uint16_t word)
{
    return GetOctetsRange<TNumber, TNumber>(word);
}

inline uint8_t GetLastOctet(uint16_t word)
{
    return GetOctetAt<4>(word);
}