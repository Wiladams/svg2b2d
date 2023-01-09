#pragma once


#include "definitions.h"


#include <cstdint>
#include <cstring>
#include <iterator>	// for std::data(), std::size()

namespace svg2b2d
{
#ifdef __cplusplus
	extern "C" {
#endif

		//
		// A core type for representing a contiguous sequence of bytes
		// As of C++ 20, there is std::span, but it is not yet widely supported
		// 
		// The ByteSpan is used in everything from networking
		// to graphics bitmaps to audio buffers.
		// Having a universal representation of a chunk of data
		// allows for easy interoperability between different
		// subsystems.  
		// 
		// It also allows us to eliminate disparate implementations that
		// are used for the same purpose.
		struct ByteSpan
		{
			const uint8_t* fStart;
			const uint8_t* fEnd;

#ifdef __cplusplus

			
			ByteSpan(const uint8_t* start, const uint8_t* end) : fStart(start), fEnd(end) {}
			ByteSpan() : fStart(nullptr), fEnd(nullptr) {}

			
			INLINE uint8_t& operator[](size_t i);				// Array access
			INLINE const uint8_t& operator[](size_t i) const;	// Array access

			INLINE uint8_t& operator*();				// get current byte value
			INLINE const uint8_t& operator*() const;	// get current byte value

			INLINE ByteSpan& operator+= (size_t a);	// advance by the specified amount

			INLINE ByteSpan& operator++();				// prefix ++y
			INLINE ByteSpan& operator++(int i);		// postfix y++
			INLINE ByteSpan& operator--(int i);		// postfix y--
			
			INLINE explicit operator bool() const { return (fEnd - fStart) > 0; };
#endif
		};





		static INLINE ByteSpan make_chunk(const void* starting, const void* ending) noexcept;

		static INLINE ByteSpan chunk_from_data_size(void* data, size_t sz) noexcept;
		static INLINE ByteSpan chunk_from_cstr(const char* str) noexcept;
		
		static INLINE const uint8_t* data(ByteSpan& dc) noexcept;
		static INLINE const uint8_t* begin(ByteSpan& dc) noexcept;
		static INLINE const uint8_t* end(ByteSpan& dc) noexcept;
		//static INLINE size_t size(const ByteSpan& dc) noexcept;
		static INLINE size_t chunk_size(const ByteSpan& dc) noexcept;
		static INLINE bool chunk_empty(const ByteSpan& dc) noexcept;
		static INLINE size_t copy(ByteSpan& a, const ByteSpan& b) noexcept;
		static INLINE size_t copy_to_cstr(char *str, size_t len, const ByteSpan& a) noexcept;
		static INLINE int compare(const ByteSpan& a, const ByteSpan& b) noexcept;
		static INLINE int comparen(const ByteSpan& a, const ByteSpan& b, int n) noexcept;
		static INLINE int comparen_cstr(const ByteSpan& a, const char * b, int n) noexcept;
		static INLINE bool chunk_is_equal(const ByteSpan& a, const ByteSpan& b) noexcept;
		static INLINE bool chunk_is_equal_cstr(const ByteSpan& a, const char* s) noexcept;
		
		// Some utility functions for common operations
		static INLINE void chunk_clear(ByteSpan& dc) noexcept;
		static INLINE void chunk_truncate(ByteSpan& dc) noexcept;
		static INLINE ByteSpan& chunk_skip(ByteSpan& dc, int n) noexcept;
		static INLINE ByteSpan& chunk_skip_to_end(ByteSpan& dc) noexcept;


		
#ifdef __cplusplus
	}
#endif
	
#ifdef __cplusplus
	//
	// operators for comparison
	// operator!=;
	// operator<=;
	// operator>=;
	static INLINE bool operator==(const ByteSpan& a, const ByteSpan& b) noexcept;
	static INLINE bool operator==(const ByteSpan& a, const char* b) noexcept;
	static INLINE bool operator< (const ByteSpan& a, const ByteSpan& b) noexcept;
	static INLINE bool operator> (const ByteSpan& a, const ByteSpan& b) noexcept;
	static INLINE bool operator!=(const ByteSpan& a, const ByteSpan& b) noexcept;
	static INLINE bool operator<=(const ByteSpan& a, const ByteSpan& b) noexcept;
	static INLINE bool operator>=(const ByteSpan& a, const ByteSpan& b) noexcept;
	
#endif


	}

	
namespace svg2b2d
{

	
#ifdef __cplusplus
	extern "C" {
#endif
		
	// ByteSpan routines
	static INLINE ByteSpan make_chunk(const void* starting, const void* ending) noexcept { return { (const uint8_t*)starting, (const uint8_t*)ending }; }
	//static INLINE ByteSpan make_chunk_size(void* data, size_t sz) noexcept { return { (uint8_t*)data, (uint8_t*)data+sz }; }
	//static INLINE ByteSpan make_chunk_cstr(const char* data) noexcept { return { (uint8_t*)data, (uint8_t*)data + strlen(data) }; }
	static INLINE ByteSpan chunk_from_data_size(void* data, size_t sz) noexcept { return { (uint8_t*)data, (uint8_t*)data + sz }; }
	static INLINE ByteSpan chunk_from_cstr(const char* data) noexcept { return { (uint8_t*)data, (uint8_t*)data + strlen(data) }; }

	static INLINE const uint8_t* begin(ByteSpan& dc) noexcept { return dc.fStart; }
	static INLINE const uint8_t* end(ByteSpan& dc) noexcept { return dc.fEnd; }
	
	static INLINE const uint8_t* data(ByteSpan& dc)  noexcept { return dc.fStart; }
	//static INLINE size_t size(const ByteSpan& dc)  noexcept { return dc.fEnd - dc.fStart; }
	static INLINE size_t chunk_size(const ByteSpan& dc)  noexcept { return dc.fEnd - dc.fStart; }
	static INLINE bool chunk_empty(const ByteSpan& dc)  noexcept { return dc.fEnd == dc.fStart; }
	static INLINE size_t copy(ByteSpan& a, const ByteSpan& b) noexcept 
	{ 
		size_t maxBytes = chunk_size(a) < chunk_size(b) ? chunk_size(a) : chunk_size(b);
		memcpy((uint8_t *)a.fStart, b.fStart, maxBytes);
		return maxBytes;
	}

	static INLINE int compare(const ByteSpan& a, const ByteSpan& b) noexcept
	{
		size_t maxBytes = chunk_size(a) < chunk_size(b) ? chunk_size(a) : chunk_size(b);
		return memcmp(a.fStart, b.fStart, maxBytes);
	}

	static INLINE int comparen(const ByteSpan &a, const ByteSpan &b, int n) noexcept
	{
		size_t maxBytes = chunk_size(a) < chunk_size(b) ? chunk_size(a) : chunk_size(b);
		if (maxBytes > n)
			maxBytes = n;
		return memcmp(a.fStart, b.fStart, maxBytes);
	}
	
	static INLINE int comparen_cstr(const ByteSpan &a, const char *b, int n) noexcept
	{
		size_t maxBytes = chunk_size(a) < n ? chunk_size(a) : n;
		return memcmp(a.fStart, b, maxBytes);
	}
	
	static INLINE bool chunk_is_equal(const ByteSpan& a, const ByteSpan& b) noexcept
	{
		if (chunk_size(a) != chunk_size(b))
			return false;
		return memcmp(a.fStart, b.fStart, chunk_size(a)) == 0;
	}

	static INLINE bool chunk_is_equal_cstr(const ByteSpan &a, const char *cstr) noexcept
	{
		size_t len = strlen(cstr);
		if (chunk_size(a) != len)
			return false;
		return memcmp(a.fStart, cstr, len) == 0;
	}

	static INLINE void chunk_clear(ByteSpan& dc) noexcept
	{
		memset((uint8_t *)dc.fStart, 0, chunk_size(dc));
	}
	
	static INLINE void chunk_truncate(ByteSpan& dc) noexcept
	{
		dc.fEnd = dc.fStart;
	}
	
	static INLINE ByteSpan & chunk_skip(ByteSpan &dc, int n) noexcept
	{
		if (n > chunk_size(dc))
			n = chunk_size(dc);
		dc.fStart += n;
		
		return dc;
	}
	
	static INLINE ByteSpan& chunk_skip_to_end(ByteSpan& dc) noexcept { dc.fStart = dc.fEnd; }

	
#ifdef __cplusplus
	}
#endif
}

namespace svg2b2d
{
#ifdef __cplusplus
	
	INLINE uint8_t& ByteSpan::operator[](size_t i) { return ((uint8_t *)fStart)[i]; }
	INLINE const uint8_t& ByteSpan::operator[](size_t i) const { return ((uint8_t *)fStart)[i]; }
	
	INLINE uint8_t& ByteSpan::operator*() { static uint8_t zero = 0;  if (fStart < fEnd) return *(uint8_t*)fStart; return  zero; }
	INLINE const uint8_t& ByteSpan::operator*() const { static uint8_t zero = 0;  if (fStart < fEnd) return *(uint8_t*)fStart; return  zero; }

	INLINE ByteSpan& ByteSpan::operator++() { return chunk_skip(*this, 1); }			// prefix notation ++y
	INLINE ByteSpan& ByteSpan::operator++(int i) { return chunk_skip(*this, 1); }       // postfix notation y++
	INLINE ByteSpan& ByteSpan::operator--(int i) { return chunk_skip(*this, -1); }       // postfix notation y++

	INLINE ByteSpan& ByteSpan::operator+= (size_t n) { return chunk_skip(*this, n); }

	//INLINE explicit ByteSpan::operator bool() const { return (fEnd - fStart) > 0; }
	
	static INLINE bool operator==(const ByteSpan& a, const ByteSpan& b) noexcept
	{
		if (chunk_size(a) != chunk_size(b))
			return false;
		return memcmp(a.fStart, b.fStart, chunk_size(a)) == 0;
	}
	
	static INLINE bool operator==(const ByteSpan& a, const char* b) noexcept
	{
		size_t len = strlen(b);
		if (chunk_size(a) != len)
			return false;
		return memcmp(a.fStart, b, len) == 0;
	}

	static INLINE bool operator!=(const ByteSpan& a, const ByteSpan& b) noexcept
	{
		if (chunk_size(a) != chunk_size(b))
			return true;
		return memcmp(a.fStart, b.fStart, chunk_size(a)) != 0;
	}
	
	static INLINE bool operator<(const ByteSpan &a, const ByteSpan&b) noexcept
	{
		size_t maxBytes = chunk_size(a) < chunk_size(b) ? chunk_size(a) : chunk_size(b);
		return memcmp(a.fStart, b.fStart, maxBytes) < 0;
	}

	static INLINE bool operator>(const ByteSpan& a, const ByteSpan& b) noexcept
	{
		size_t maxBytes = chunk_size(a) < chunk_size(b) ? chunk_size(a) : chunk_size(b);
		return memcmp(a.fStart, b.fStart, maxBytes) > 0;
	}
	
	static INLINE bool operator<=(const ByteSpan &a, const ByteSpan &b) noexcept
	{
		size_t maxBytes = chunk_size(a) < chunk_size(b) ? chunk_size(a) : chunk_size(b);
		return memcmp(a.fStart, b.fStart, maxBytes) <= 0;
	}
	
	static INLINE bool operator>=(const ByteSpan& a, const ByteSpan& b) noexcept
	{
		size_t maxBytes = chunk_size(a) < chunk_size(b) ? chunk_size(a) : chunk_size(b);
		return memcmp(a.fStart, b.fStart, maxBytes) >= 0;
	}
	
#endif
}

