#pragma once


//#include "definitions.h"


#include <cstdint>
#include <cstring>
#include <iterator>	// for std::data(), std::size()



namespace svg2b2d {


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
		const unsigned char* fStart;
		const unsigned char* fEnd;




		ByteSpan(const uint8_t* start, const uint8_t* end) : fStart(start), fEnd(end) {}
		ByteSpan() : fStart(nullptr), fEnd(nullptr) {}


		inline uint8_t& operator[](size_t i);				// Array access
		inline const uint8_t& operator[](size_t i) const;	// Array access

		inline uint8_t& operator*();				// get current byte value
		inline const uint8_t& operator*() const;	// get current byte value

		inline ByteSpan& operator+= (size_t a);	// advance by the specified amount

		inline ByteSpan& operator++();				// prefix ++y
		inline ByteSpan& operator++(int i);		// postfix y++
		inline ByteSpan& operator--(int i);		// postfix y--

		inline explicit operator bool() const { return (fEnd - fStart) > 0; };

	};





	static inline ByteSpan make_chunk(const void* starting, const void* ending) noexcept;

	static inline ByteSpan chunk_from_data_size(const void* data, size_t sz) noexcept;
	static inline ByteSpan chunk_from_cstr(const char* str) noexcept;

	static inline const uint8_t* data(ByteSpan& dc) noexcept;
	static inline const uint8_t* begin(ByteSpan& dc) noexcept;
	static inline const uint8_t* end(ByteSpan& dc) noexcept;
	//static inline size_t size(const ByteSpan& dc) noexcept;
	static inline size_t chunk_size(const ByteSpan& dc) noexcept;
	static inline bool chunk_empty(const ByteSpan& dc) noexcept;
	static inline size_t copy(ByteSpan& a, const ByteSpan& b) noexcept;
	static inline size_t copy_to_cstr(char* str, size_t len, const ByteSpan& a) noexcept;
	static inline int compare(const ByteSpan& a, const ByteSpan& b) noexcept;
	static inline int comparen(const ByteSpan& a, const ByteSpan& b, int n) noexcept;
	static inline int comparen_cstr(const ByteSpan& a, const char* b, int n) noexcept;
	static inline bool chunk_is_equal(const ByteSpan& a, const ByteSpan& b) noexcept;
	static inline bool chunk_is_equal_cstr(const ByteSpan& a, const char* s) noexcept;

	// Some utility functions for common operations
	static inline void chunk_clear(ByteSpan& dc) noexcept;
	static inline void chunk_truncate(ByteSpan& dc) noexcept;
	static inline ByteSpan& chunk_skip(ByteSpan& dc, int n) noexcept;
	static inline ByteSpan& chunk_skip_to_end(ByteSpan& dc) noexcept;






	//
	// operators for comparison
	// operator!=;
	// operator<=;
	// operator>=;
	static inline bool operator==(const ByteSpan& a, const ByteSpan& b) noexcept;
	static inline bool operator==(const ByteSpan& a, const char* b) noexcept;
	static inline bool operator< (const ByteSpan& a, const ByteSpan& b) noexcept;
	static inline bool operator> (const ByteSpan& a, const ByteSpan& b) noexcept;
	static inline bool operator!=(const ByteSpan& a, const ByteSpan& b) noexcept;
	static inline bool operator<=(const ByteSpan& a, const ByteSpan& b) noexcept;
	static inline bool operator>=(const ByteSpan& a, const ByteSpan& b) noexcept;




		// ByteSpan routines
		static inline ByteSpan make_chunk(const void* starting, const void* ending) noexcept { return { (const uint8_t*)starting, (const uint8_t*)ending }; }

		static inline ByteSpan chunk_from_data_size(const void* data, size_t sz) noexcept { return { (uint8_t*)data, (uint8_t*)data + sz }; }
		static inline ByteSpan chunk_from_cstr(const char* data) noexcept { return { (uint8_t*)data, (uint8_t*)data + strlen(data) }; }

		static inline const uint8_t* begin(ByteSpan& dc) noexcept { return dc.fStart; }
		static inline const uint8_t* end(ByteSpan& dc) noexcept { return dc.fEnd; }

		static inline const uint8_t* data(ByteSpan& dc)  noexcept { return dc.fStart; }
		//static inline size_t size(const ByteSpan& dc)  noexcept { return dc.fEnd - dc.fStart; }
		static inline size_t chunk_size(const ByteSpan& dc)  noexcept { return dc.fEnd - dc.fStart; }
		static inline bool chunk_empty(const ByteSpan& dc)  noexcept { return dc.fEnd == dc.fStart; }
		static inline size_t copy(ByteSpan& a, const ByteSpan& b) noexcept
		{
			size_t maxBytes = chunk_size(a) < chunk_size(b) ? chunk_size(a) : chunk_size(b);
			memcpy((uint8_t*)a.fStart, b.fStart, maxBytes);
			return maxBytes;
		}

		static inline int compare(const ByteSpan& a, const ByteSpan& b) noexcept
		{
			size_t maxBytes = chunk_size(a) < chunk_size(b) ? chunk_size(a) : chunk_size(b);
			return memcmp(a.fStart, b.fStart, maxBytes);
		}

		static inline int comparen(const ByteSpan& a, const ByteSpan& b, int n) noexcept
		{
			size_t maxBytes = chunk_size(a) < chunk_size(b) ? chunk_size(a) : chunk_size(b);
			if (maxBytes > n)
				maxBytes = n;
			return memcmp(a.fStart, b.fStart, maxBytes);
		}

		static inline int comparen_cstr(const ByteSpan& a, const char* b, int n) noexcept
		{
			size_t maxBytes = chunk_size(a) < n ? chunk_size(a) : n;
			return memcmp(a.fStart, b, maxBytes);
		}

		static inline bool chunk_is_equal(const ByteSpan& a, const ByteSpan& b) noexcept
		{
			if (chunk_size(a) != chunk_size(b))
				return false;
			return memcmp(a.fStart, b.fStart, chunk_size(a)) == 0;
		}

		static inline bool chunk_is_equal_cstr(const ByteSpan& a, const char* cstr) noexcept
		{
			size_t len = strlen(cstr);
			if (chunk_size(a) != len)
				return false;
			return memcmp(a.fStart, cstr, len) == 0;
		}

		static inline void chunk_clear(ByteSpan& dc) noexcept
		{
			memset((uint8_t*)dc.fStart, 0, chunk_size(dc));
		}

		static inline void chunk_truncate(ByteSpan& dc) noexcept
		{
			dc.fEnd = dc.fStart;
		}

		static inline ByteSpan& chunk_skip(ByteSpan& dc, int n) noexcept
		{
			if (n > chunk_size(dc))
				n = chunk_size(dc);
			dc.fStart += n;

			return dc;
		}

		static inline ByteSpan& chunk_skip_to_end(ByteSpan& dc) noexcept { dc.fStart = dc.fEnd; }








	inline uint8_t& ByteSpan::operator[](size_t i) { return ((uint8_t*)fStart)[i]; }
	inline const uint8_t& ByteSpan::operator[](size_t i) const { return ((uint8_t*)fStart)[i]; }

	inline uint8_t& ByteSpan::operator*() { static uint8_t zero = 0;  if (fStart < fEnd) return *(uint8_t*)fStart; return  zero; }
	inline const uint8_t& ByteSpan::operator*() const { static uint8_t zero = 0;  if (fStart < fEnd) return *(uint8_t*)fStart; return  zero; }

	inline ByteSpan& ByteSpan::operator++() { return chunk_skip(*this, 1); }			// prefix notation ++y
	inline ByteSpan& ByteSpan::operator++(int i) { return chunk_skip(*this, 1); }       // postfix notation y++
	inline ByteSpan& ByteSpan::operator--(int i) { return chunk_skip(*this, -1); }       // postfix notation y++

	inline ByteSpan& ByteSpan::operator+= (size_t n) { return chunk_skip(*this, n); }

	//inline explicit ByteSpan::operator bool() const { return (fEnd - fStart) > 0; }

	static inline bool operator==(const ByteSpan& a, const ByteSpan& b) noexcept
	{
		if (chunk_size(a) != chunk_size(b))
			return false;
		return memcmp(a.fStart, b.fStart, chunk_size(a)) == 0;
	}

	static inline bool operator==(const ByteSpan& a, const char* b) noexcept
	{
		size_t len = strlen(b);
		if (chunk_size(a) != len)
			return false;
		return memcmp(a.fStart, b, len) == 0;
	}

	static inline bool operator!=(const ByteSpan& a, const ByteSpan& b) noexcept
	{
		if (chunk_size(a) != chunk_size(b))
			return true;
		return memcmp(a.fStart, b.fStart, chunk_size(a)) != 0;
	}

	static inline bool operator<(const ByteSpan& a, const ByteSpan& b) noexcept
	{
		size_t maxBytes = chunk_size(a) < chunk_size(b) ? chunk_size(a) : chunk_size(b);
		return memcmp(a.fStart, b.fStart, maxBytes) < 0;
	}

	static inline bool operator>(const ByteSpan& a, const ByteSpan& b) noexcept
	{
		size_t maxBytes = chunk_size(a) < chunk_size(b) ? chunk_size(a) : chunk_size(b);
		return memcmp(a.fStart, b.fStart, maxBytes) > 0;
	}

	static inline bool operator<=(const ByteSpan& a, const ByteSpan& b) noexcept
	{
		size_t maxBytes = chunk_size(a) < chunk_size(b) ? chunk_size(a) : chunk_size(b);
		return memcmp(a.fStart, b.fStart, maxBytes) <= 0;
	}

	static inline bool operator>=(const ByteSpan& a, const ByteSpan& b) noexcept
	{
		size_t maxBytes = chunk_size(a) < chunk_size(b) ? chunk_size(a) : chunk_size(b);
		return memcmp(a.fStart, b.fStart, maxBytes) >= 0;
	}


}
