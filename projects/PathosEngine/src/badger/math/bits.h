#pragma once

namespace badger {

	// Count leading zeros
	// https://stackoverflow.com/questions/23856596/how-to-count-leading-zeros-in-a-32-bit-unsigned-integer
	inline int32 clz(uint32 x) {
		static const char debruijn32[32] = {
			0, 31, 9, 30, 3, 8, 13, 29, 2, 5, 7, 21, 12, 24, 28, 19,
			1, 10, 4, 14, 6, 22, 25, 20, 11, 15, 23, 26, 16, 27, 17, 18
		};
		if (!x) return 32;
		x |= x >> 1;
		x |= x >> 2;
		x |= x >> 4;
		x |= x >> 8;
		x |= x >> 16;
		x++;
		return debruijn32[x * 0x076be629 >> 27];
	}
	// Count trailing zeros
	// https://stackoverflow.com/questions/45221914/how-do-you-efficiently-count-the-trailing-zero-bits-in-a-number
	inline int32 ctz(unsigned x) {
		int32 n;

		if (x == 0) return 32;
		n = 1;
		if ((x & 0x0000FFFF) == 0) { n = n + 16; x = x >> 16; }
		if ((x & 0x000000FF) == 0) { n = n + 8; x = x >> 8; }
		if ((x & 0x0000000F) == 0) { n = n + 4; x = x >> 4; }
		if ((x & 0x00000003) == 0) { n = n + 2; x = x >> 2; }
		return n - (x & 1);
	}

}
