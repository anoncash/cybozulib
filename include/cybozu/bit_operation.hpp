#pragma once
/**
	@file
	@brief bit operation
*/
#include <assert.h>
#include <cybozu/inttype.hpp>

#if defined(_WIN32)
	#include <intrin.h>
#elif defined(__linux__) || defined(__CYGWIN__)
	#include <x86intrin.h>
#elif defined(__GNUC__)
	#include <emmintrin.h>
#endif

namespace cybozu {

namespace bit_op_local {

template<bool equalTo8>
struct Tag {};

// sizeof(T) < 8
template<>
struct Tag<false> {
	template<class T>
	static inline int bsf(T x)
	{
#if defined(_WIN32)
		unsigned long out;
		_BitScanForward(&out, x);
		return out;
#else
		return __builtin_ctz(x);
#endif
	}
	template<class T>
	static inline int bsr(T x)
	{
#if defined(_MSC_VER)
		unsigned long out;
		_BitScanReverse(&out, x);
		return out;
#else
		return __builtin_clz(x) ^ 0x1f;
#endif
	}
};

// sizeof(T) == 8
template<>
struct Tag<true> {
	template<class T>
	static inline int bsf(T x)
	{
#if defined(_MSC_VER) && defined(_WIN64)
		unsigned long out;
		_BitScanForward64(&out, x);
		return out;
#elif defined(__x86_64__)
		return __builtin_ctzl(x);
#else
		const uint32_t L = uint32_t(x);
		if (L) return Tag<false>::bsf(L);
		const uint32_t H = uint32_t(x >> 32);
		return Tag<false>::bsf(H) + 32;
#endif
	}
	template<class T>
	static inline int bsr(T x)
	{
#if defined(_MSC_VER) && defined(_WIN64)
		unsigned long out;
		_BitScanReverse64(&out, x);
		return out;
#elif defined(__x86_64__)
		return __builtin_clzl(x) ^ 0x3f;
#else
		const uint32_t H = uint32_t(x >> 32);
		if (H) return Tag<false>::bsr(H) + 32;
		const uint32_t L = uint32_t(x);
		return Tag<false>::bsr(L);
#endif
	}
};

} // bit_op_local

template<class T>
int bsf(T x)
{
	return bit_op_local::Tag<sizeof(T) == 8>::bsf(x);
}
template<class T>
int bsr(T x)
{
	return bit_op_local::Tag<sizeof(T) == 8>::bsr(x);
}

template<class T>
uint64_t makeBitMask64(T x)
{
	assert(x < 64);
	return (uint64_t(1) << x) - 1;
}

#if defined(_MSC_VER) || defined(__POPCNT__)
template<class T>
uint32_t popcnt(T x);

template<>
inline uint32_t popcnt<uint32_t>(uint32_t x)
{
	return (uint32_t)_mm_popcnt_u32(x);
}

template<>
inline uint32_t popcnt<uint64_t>(uint64_t x)
{
#if defined(_WIN64) || defined(__x86_64__)
	return (uint32_t)_mm_popcnt_u64(x);
#else
	return popcnt<uint32_t>(uint32_t(x)) + popcnt<uint32_t>(uint32_t(x >> 32));
#endif
}

#endif

} // cybozu
