#pragma once

#include <bitset>
#include <cstdint>

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

		explicit charset(const char achar) noexcept { addChar(achar); }
		charset(const char* chars) noexcept { addChars(chars); }


		// add a single character to the set
		charset& addChar(const char achar) noexcept
		{
			bits.set(achar);
			return *this;
		}

		charset& addChars(const char* chars) noexcept
		{
			size_t len = strlen(chars);
			for (size_t i = 0; i < len; i++)
				bits.set(chars[i]);

			return *this;
		}

		charset& operator+=(const char achar) noexcept { return addChar(achar); }
		charset& operator+=(const char* chars) noexcept { return addChars(chars); }

		charset operator+(const char achar) const noexcept
		{
			charset result(*this);
			return result.addChar(achar);
		}

		charset operator+(const char* chars) const noexcept
		{
			charset result(*this);
			return result.addChars(chars);
		}

		// This one makes it look like an array
		bool operator [](const uint8_t idx) const noexcept { return bits[idx]; }

		// This way makes it look like a function
		bool operator ()(const uint8_t idx) const noexcept { return bits[idx]; }

		bool contains(const uint8_t idx) const noexcept { return bits[idx]; }

	};

}
