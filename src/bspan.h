#pragma once

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

		// Constructors
		ByteSpan() : fStart(nullptr), fEnd(nullptr) {}
		ByteSpan(const unsigned char* start, const unsigned char* end) : fStart(start), fEnd(end) {}
		explicit ByteSpan(const void* data, size_t sz) :fStart((const unsigned char*)data), fEnd((const unsigned char*)fStart + sz) {}



		// Type conversions
		explicit operator bool() const { return (fEnd - fStart) > 0; };
		
		
		// Array access
		unsigned char& operator[](size_t i) { return ((unsigned char*)fStart)[i]; }
		const unsigned char& operator[](size_t i) const { return ((unsigned char*)fStart)[i]; }

		// get current value from fStart, like a 'peek' operation
		unsigned char& operator*() { static unsigned char zero = 0;  if (fStart < fEnd) return *(unsigned char*)fStart; return  zero; }
		const uint8_t& operator*() const { static unsigned char zero = 0;  if (fStart < fEnd) return *(unsigned char*)fStart; return  zero; }
		
		ByteSpan& operator+= (size_t n) { 
			if (n > size())
				n = size();
			fStart += n;

			return *this;
		}


		ByteSpan& operator++() { return operator+=(1); }			// prefix notation ++y
		ByteSpan& operator++(int i) { return operator+=(1); }       // postfix notation y++
		//ByteSpan& operator--() { return operator-=(1); }       // postfix notation y++
		//ByteSpan& operator--(int i) { return operator-=(1); }       // postfix notation y++



		// setting up for a range-based for loop
		const unsigned char* data() noexcept { return (unsigned char *)fStart; }
		const unsigned char* begin() noexcept { return fStart; }
		const unsigned char* end() noexcept { return fEnd; }
		size_t size()  const noexcept { return fEnd - fStart; }
		
		void setAll(unsigned char c) noexcept {memset((uint8_t*)fStart,c, size());}
	};



	//static inline ByteSpan chunk_from_data_size(const void* data, size_t sz) noexcept;
	static inline ByteSpan chunk_from_cstr(const char* str) noexcept;


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

	static inline void chunk_truncate(ByteSpan& dc) noexcept;
	static inline ByteSpan& chunk_skip(ByteSpan& dc, ptrdiff_t n) noexcept;
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

		//static inline ByteSpan chunk_from_data_size(const void* data, size_t sz) noexcept { return { (uint8_t*)data, (uint8_t*)data + sz }; }
		static inline ByteSpan chunk_from_cstr(const char* data) noexcept { return ByteSpan{ (uint8_t*)data, (uint8_t*)data + strlen(data) }; }



		//static inline const uint8_t* data(ByteSpan& dc)  noexcept { return dc.fStart; }


		static inline size_t chunk_size(const ByteSpan& a) noexcept { return a.size(); }
		static inline bool chunk_empty(const ByteSpan& dc)  noexcept { return dc.fEnd == dc.fStart; }
		static inline size_t copy(ByteSpan& a, const ByteSpan& b) noexcept
		{
			size_t maxBytes = a.size() < b.size() ? a.size() : b.size();
			memcpy((uint8_t*)a.fStart, b.fStart, maxBytes);
			return maxBytes;
		}

		static inline int compare(const ByteSpan& a, const ByteSpan& b) noexcept
		{
			size_t maxBytes = a.size() < b.size() ? a.size() : b.size();
			return memcmp(a.fStart, b.fStart, maxBytes);
		}

		static inline int comparen(const ByteSpan& a, const ByteSpan& b, int n) noexcept
		{
			size_t maxBytes = a.size() < b.size() ? a.size() : b.size();
			if (maxBytes > n)
				maxBytes = n;
			return memcmp(a.fStart, b.fStart, maxBytes);
		}

		static inline int comparen_cstr(const ByteSpan& a, const char* b, int n) noexcept
		{
			size_t maxBytes = a.size() < n ? a.size() : n;
			return memcmp(a.fStart, b, maxBytes);
		}

		static inline bool chunk_is_equal(const ByteSpan& a, const ByteSpan& b) noexcept
		{
			if (a.size() != b.size())
				return false;
			return memcmp(a.fStart, b.fStart, a.size()) == 0;
		}

		static inline bool chunk_is_equal_cstr(const ByteSpan& a, const char* cstr) noexcept
		{
			size_t len = strlen(cstr);
			if (a.size() != len)
				return false;
			return memcmp(a.fStart, cstr, len) == 0;
		}


		static inline void chunk_truncate(ByteSpan& dc) noexcept
		{
			dc.fEnd = dc.fStart;
		}

		static inline ByteSpan& chunk_skip(ByteSpan& dc, ptrdiff_t n) noexcept
		{
			if (n > dc.size())
				n = dc.size();
			dc.fStart += n;

			return dc;
		}

		static inline ByteSpan& chunk_skip_to_end(ByteSpan& dc) noexcept { dc.fStart = dc.fEnd; }

















	static inline bool operator==(const ByteSpan& a, const ByteSpan& b) noexcept
	{
		if (a.size() != b.size())
			return false;
		return memcmp(a.fStart, b.fStart, a.size()) == 0;
	}

	static inline bool operator==(const ByteSpan& a, const char* b) noexcept
	{
		size_t len = strlen(b);
		if (a.size() != len)
			return false;
		return memcmp(a.fStart, b, len) == 0;
	}

	static inline bool operator!=(const ByteSpan& a, const ByteSpan& b) noexcept
	{
		if (a.size() != b.size())
			return true;
		return memcmp(a.fStart, b.fStart, a.size()) != 0;
	}

	static inline bool operator<(const ByteSpan& a, const ByteSpan& b) noexcept
	{
		size_t maxBytes = a.size() < b.size() ? a.size() : b.size();
		return memcmp(a.fStart, b.fStart, maxBytes) < 0;
	}

	static inline bool operator>(const ByteSpan& a, const ByteSpan& b) noexcept
	{
		size_t maxBytes = a.size() < b.size() ? a.size() : b.size();
		return memcmp(a.fStart, b.fStart, maxBytes) > 0;
	}

	static inline bool operator<=(const ByteSpan& a, const ByteSpan& b) noexcept
	{
		size_t maxBytes = a.size() < b.size() ? a.size() : b.size();
		return memcmp(a.fStart, b.fStart, maxBytes) <= 0;
	}

	static inline bool operator>=(const ByteSpan& a, const ByteSpan& b) noexcept
	{
		size_t maxBytes = a.size() < b.size() ? a.size() : b.size();
		return memcmp(a.fStart, b.fStart, maxBytes) >= 0;
	}


}
