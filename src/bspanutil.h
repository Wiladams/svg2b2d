#pragma once


//	Herein you will find various objects and functions which are
//	useful while scanning, tokenizing, parsing streams of text.

#include "bspan.h"
#include <bitset>


namespace svg2b2d {
	// Represent a set of characters as a bitset
	//
	// Typical usage:
	//   charset whitespaceChars("\t\n\f\r ");
	//
	//	 // skipping over whitespace
	//   while (whitespaceChars[c])
	//		c = nextChar();
	//
	//
	//  This is better than simply using the old classic isspace() and other functions
	//  as you can define your own sets, depending on your needs:
	//
	//  charset delimeterChars("()<>[]{}/%");
	//
	//  Of course, there will surely be a built-in way of doing this in C/C++ 
	//  and it will no doubt be tied to particular version of the compiler.  Use that
	//  if it suits your needs.  Meanwhile, at least you can see how such a thing can
	//  be implemented.


	struct charset {
		std::bitset<256> bits;

		explicit charset(const char achar)
		{
			addChar(achar);
		}

		charset(const char* chars)
		{
			addChars(chars);
		}

		charset& addChar(const char achar)
		{
			bits.set(achar);
			return *this;
		}

		charset& addChars(const char* chars)
		{
			size_t len = strlen(chars);
			for (size_t i = 0; i < len; i++)
				bits.set(chars[i]);

			return *this;
		}

		charset& operator+=(const char achar)
		{
			return addChar(achar);
		}

		charset& operator+=(const char* chars)
		{
			return addChars(chars);
		}

		charset operator+(const char achar) const
		{
			charset result(*this);
			return result.addChar(achar);
		}

		charset operator+(const char* chars) const
		{
			charset result(*this);
			return result.addChars(chars);
		}

		// This one makes us look like an array
		bool operator [](const size_t idx) const
		{
			return bits[idx];
		}

		// This way makes us look like a function
		bool operator ()(const size_t idx) const
		{
			return bits[idx];
		}

		bool contains(const uint8_t idx) const
		{
			return bits[idx];
		}
		
	};
	

	static INLINE charset charset_create_from_cstr(const char achar);
	static INLINE charset charset_create_from_char(const char* chars);
	static INLINE bool charset_contains(const charset& a, const uint8_t idx);
	static INLINE void charset_add_char(charset& a, const char b);
	static INLINE void charset_add_cstr(charset& a, const char* b);

}

namespace svg2b2d
{
	static charset wspChars(" \r\n\t\f\v");		// a set of typical whitespace chars
	

		
		static INLINE size_t copy_to_cstr(char* str, size_t len, const ByteSpan& a) noexcept;
		static INLINE ByteSpan chunk_ltrim(const ByteSpan& a, const charset& skippable) noexcept;
		static INLINE ByteSpan chunk_rtrim(const ByteSpan& a, const charset& skippable) noexcept;
		static INLINE ByteSpan chunk_trim(const ByteSpan& a, const charset& skippable) noexcept;
		static INLINE ByteSpan chunk_skip_wsp(const ByteSpan& a) noexcept;
		
		static INLINE ByteSpan chunk_subchunk(const ByteSpan& a, const size_t start, const size_t sz) noexcept;
		static INLINE bool chunk_starts_with(const ByteSpan& a, const ByteSpan& b) noexcept;
		static INLINE bool chunk_starts_with_char(const ByteSpan& a, const uint8_t b) noexcept;
		static INLINE bool chunk_starts_with_cstr(const ByteSpan& a, const char* b) noexcept;
		
		static INLINE bool chunk_ends_with(const ByteSpan& a, const ByteSpan& b) noexcept;
		static INLINE bool chunk_ends_with_char(const ByteSpan& a, const uint8_t b) noexcept;
		static INLINE bool chunk_ends_with_cstr(const ByteSpan& a, const char* b) noexcept;
		
		static INLINE ByteSpan chunk_token(ByteSpan& a, const charset& delims) noexcept;
		static INLINE ByteSpan chunk_find_char(const ByteSpan& a, char c) noexcept;

		// Number Conversions
		static INLINE double chunk_to_double(ByteSpan& inChunk) noexcept;
		
}



namespace svg2b2d
{


		
		static INLINE size_t copy_to_cstr(char* str, size_t len, const ByteSpan& a) noexcept
		{
			size_t maxBytes = chunk_size(a) < len ? chunk_size(a) : len;
			memcpy(str, a.fStart, maxBytes);
			str[maxBytes] = 0;

			return maxBytes;
		}
		
		// Trim the left side of skippable characters
		static INLINE ByteSpan chunk_ltrim(const ByteSpan& a, const charset& skippable) noexcept
		{
			const uint8_t* start = a.fStart;
			const uint8_t* end = a.fEnd;
			while (start < end && skippable(*start))
				++start;
			return { start, end };
		}

		// trim the right side of skippable characters
		static INLINE ByteSpan chunk_rtrim(const ByteSpan& a, const charset& skippable) noexcept
		{
			const uint8_t* start = a.fStart;
			const uint8_t* end = a.fEnd;
			while (start < end && skippable(*(end - 1)))
				--end;

			return { start, end };
		}

		// trim the left and right side of skippable characters
		static INLINE ByteSpan chunk_trim(const ByteSpan& a, const charset& skippable) noexcept
		{
			const uint8_t* start = a.fStart;
			const uint8_t* end = a.fEnd;
			while (start < end && skippable(*start))
				++start;
			while (start < end && skippable(*(end - 1)))
				--end;
			return { start, end };
		}
		
		static INLINE ByteSpan chunk_skip_wsp(const ByteSpan& a) noexcept
		{
			const uint8_t* start = a.fStart;
			const uint8_t* end = a.fEnd;
			while (start < end && wspChars(*start))
				++start;
			return { start, end };
		}
		
		static INLINE ByteSpan chunk_subchunk(const ByteSpan& a, const size_t startAt, const size_t sz) noexcept
		{
			const uint8_t* start = a.fStart;
			const uint8_t* end = a.fEnd;
			if (startAt < chunk_size(a))
			{
				start += startAt;
				if (start + sz < end)
					end = start + sz;
				else
					end = a.fEnd;
			} else
			{
				start = end;
			}
			return { start, end };
		}
		
		static INLINE bool chunk_starts_with(const ByteSpan& a, const ByteSpan& b) noexcept
		{
			return chunk_is_equal(chunk_subchunk(a, 0, chunk_size(b)), b);
		}
		
		static INLINE bool chunk_starts_with_char(const ByteSpan& a, const uint8_t b) noexcept
		{
			return chunk_size(a) > 0 && a.fStart[0] == b;
		}

		static INLINE bool chunk_starts_with_cstr(const ByteSpan& a, const char* b) noexcept
		{
			return chunk_starts_with(a, chunk_from_cstr(b));
		}
		
		static INLINE bool chunk_ends_with(const ByteSpan& a, const ByteSpan& b) noexcept
		{
			return chunk_is_equal(chunk_subchunk(a, chunk_size(a)- chunk_size(b), chunk_size(b)), b);
		}
		
		static INLINE bool chunk_ends_with_char(const ByteSpan& a, const uint8_t b) noexcept
		{
			return chunk_size(a) > 0 && a.fEnd[-1] == b;
		}
		
		static INLINE bool chunk_ends_with_cstr(const ByteSpan& a, const char* b) noexcept
		{
			return chunk_ends_with(a, chunk_from_cstr(b));
		}
		
		// Given an input chunk
		// spit it into two chunks, 
		// Returns - the first chunk before delimeters
		// a - adjusted to reflect the rest of the input after delims
		// If delimeter NOT found
		// returns the entire input chunk
		// and 'a' is set to an empty chunk
		static INLINE ByteSpan chunk_token(ByteSpan& a, const charset& delims) noexcept
		{
			const uint8_t* start = a.fStart;
			const uint8_t* end = a.fEnd;
			const uint8_t* tokenEnd = start;
			while (tokenEnd < end && !delims(*tokenEnd))
				++tokenEnd;

			if (delims(*tokenEnd))
			{
				a.fStart = tokenEnd + 1;
			}
			else {
				a.fStart = tokenEnd;
			}

			return { start, tokenEnd };
		}
		

		
		// Given an input chunk
		// find the first instance of a specified character
		// return the chunk preceding the found character
		// or or the whole chunk of the character is not found
		static INLINE ByteSpan chunk_find_char(const ByteSpan& a, char c) noexcept
		{
			const uint8_t* start = a.fStart;
			const uint8_t* end = a.fEnd;
			while (start < end && *start != c)
				++start;

			return { start, end };
		}
		
		// Take a chunk containing a series of digits and turn
		// it into a 64-bit unsigned integer
		// Stop processing when the first non-digit is seen, 
		// or the end of the chunk
		static INLINE uint64_t chunk_to_u64(ByteSpan& s)
		{
			static charset digitChars("0123456789");
			
			uint64_t v = 0;
			
			while (s && digitChars(*s))
			{
				v = v * 10 + (uint64_t)(*s - '0');
				s++;
			}
			
			return v;
		}
		
		static INLINE int64_t chunk_to_i64(ByteSpan& s)
		{
			static charset digitChars("0123456789");

			int64_t v = 0;

			bool negative = false;
			if (s && *s == '-')
			{
				negative = true;
				s++;
			}

			while (s && digitChars(*s))
			{
				v = v * 10 + (int64_t)(*s - '0');
				s++;
			}

			if (negative)
				v = -v;

			return v;
		}
		
		// parse floating point number
		// includes sign, exponent, and decimal point
		// The input chunk is altered, with the fStart pointer moved to the end of the number
		static INLINE double chunk_to_double(ByteSpan& s) noexcept
		{
			static charset digitChars("0123456789");
			
			double sign = 1.0;

			double res = 0.0;
			long long intPart = 0, fracPart = 0;
			bool hasIntPart = false;
			bool hasFracPart = false;

			// Parse optional sign
			if (*s == '+') {
				s++;
			}
			else if (*s == '-') {
				sign = -1;
				s++;
			}
			
			// Parse integer part
			if (digitChars[*s]) {

				intPart = chunk_to_u64(s);

				res = (double)intPart;
				hasIntPart = true;
			}

			// Parse fractional part.
			if (*s == '.') {
				s++; // Skip '.'
				auto sentinel = s.fStart;
				
				if (digitChars(*s)) {
					fracPart = chunk_to_u64(s);	
					auto ending = s.fStart;
					
					res += (double)fracPart / pow(10.0, (double)(ending - sentinel));
					hasFracPart = true;
				}
			}

			// A valid number should have integer or fractional part.
			if (!hasIntPart && !hasFracPart)
				return 0.0;


			// Parse optional exponent
			if (*s == 'e' || *s == 'E') {
				long long expPart = 0;
				s++; // skip 'E'
				
				double expSign = 1.0;
				if (*s == '+') {
					s++;
				}
				else if (*s == '-') {
					expSign = -1.0;
					s++;
				}
				
				if (digitChars[*s]) {
					expPart = chunk_to_u64(s);
					res *= pow(10.0, expSign * (double)expPart);
				}
			}

			return res * sign;
		}
		
		
}


namespace svg2b2d {
	void writeChunk(const ByteSpan& chunk)
	{
		ByteSpan s = chunk;

		printf("||");
		while (s && *s) {
			printf("%c", *s);
			s++;
		}
		printf("||");
	}

	void printChunk(const ByteSpan& chunk)
	{
		if (chunk)
		{
			writeChunk(chunk);
			printf("\n");
		}
		else
			printf("BLANK==CHUNK\n");

	}
}


namespace svg2b2d {

	// simple type parsing
	static INLINE int64_t toInteger(const ByteSpan& inChunk);
	static INLINE double toNumber(const ByteSpan& inChunk);
	static INLINE std::string toString(const ByteSpan& inChunk);


	// Utility, for viewbox, points, etc
	static INLINE double nextNumber(ByteSpan& inChunk, const charset& delims);


	// return a number next in a list of numbers
	static INLINE double nextNumber(ByteSpan& inChunk, const charset& delims)
	{
		// First, trim the front of whitespace
		inChunk = chunk_ltrim(inChunk, wspChars);

		// now go for the next number separated by delimiters
		ByteSpan numChunk;
		numChunk = chunk_token(inChunk, delims);
		double anum = chunk_to_double(numChunk);

		return anum;
	}

	static INLINE int64_t toInteger(const ByteSpan& inChunk)
	{
		ByteSpan s = inChunk;
		return chunk_to_i64(s);
	}

	// toNumber
	// a floating point number
	static INLINE double toNumber(const ByteSpan& inChunk)
	{
		ByteSpan s = inChunk;
		return chunk_to_double(s);
	}

	static INLINE std::string toString(const ByteSpan& inChunk)
	{
		return std::string(inChunk.fStart, inChunk.fEnd);
	}

}


/*
	
	static INLINE void skipOverCharset(ByteSpan& dc, const charset& cs)
	{
		while (dc && cs.contains(*dc))
			dc++;
	}

	static INLINE void skipUntilCharset(ByteSpan& dc, const charset& cs)
	{
		while (dc && !cs.contains(*dc))
			++dc;
	}
	
	    // Turn a chunk into a vector of chunks, splitting on the delimiters
    // BUGBUG - should consider the option of empty chunks, especially at the boundaries
    static INLINE std::vector<svg2b2d::ByteSpan> chunk_split(const svg2b2d::ByteSpan& inChunk, const charset& delims, bool wantEmpties = false) noexcept
    {
        std::vector<ByteSpan> result;
        ByteSpan s = inChunk;
        while (s)
        {
            ByteSpan token = chunk_token(s, delims);
            //if (size(token) > 0)
            result.push_back(token);
        }

        return result;
    }
	
*/