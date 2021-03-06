/* ParallelFlash. Library - for filesystem-like access to SPI Serial Flash memory
 * https://github.com/FrankBoesing/ParallelFlash
 * Copyright (C) 2015,2016, Paul Stoffregen, F. Boesing
 *
 * Development of this library was funded by PJRC.COM, LLC by sales of Teensy.
 * Please support PJRC's efforts to develop open source software by purchasing
 * Teensy or other genuine PJRC products.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice, development funding notice, and this permission
 * notice shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef BiParFlash_h_
#define BiParFlash_h_

#include <Arduino.h>

class BiParFlashFile;

class BiParFlashChip
{
public:
	static bool begin();
	static uint32_t capacity(const uint8_t *id);
	static uint32_t blockSize();
	static void sleep();
	static void wakeup();
	static void readID(uint8_t *buf);
	//static void readSerialNumber(uint8_t *buf);
	static uint16_t order(uint32_t addr, uint32_t len);
	static void read(uint32_t addr, void *buf, uint32_t len);
	static bool ready();
	static void wait();
	static void write(uint32_t addr, const void *buf, uint32_t len);
	static void eraseAll();
	static void eraseBlock(uint32_t addr);

	static BiParFlashFile open(const char *filename);
	static bool create(const char *filename, uint32_t length, uint32_t align = 0);
	static bool createErasable(const char *filename, uint32_t length) {
		return create(filename, length, blockSize());
	}
	static bool exists(const char *filename);
	static bool remove(const char *filename);
	static bool remove(BiParFlashFile &file);
	static void opendir() { dirindex = 0; }
	static bool readdir(char *filename, uint32_t strsize, uint32_t &filesize);
private:
	static uint16_t dirindex; // current position for readdir()
	static uint8_t flags;	// chip features
	static uint8_t busy;	// 0 = ready
				// 1 = suspendable program operation
				// 2 = suspendable erase operation
				// 3 = busy for realz!!
	static void enterQPI();
	static void exitQPI();
	static void writeByteBoth(const uint8_t val);
	static void writeByteF0(const uint8_t val);
	static void writeByteF1(const uint8_t val);
	//static void writeBytes(const uint8_t * buf, const int len);
	//inline static void write16(const uint16_t val) __attribute__((always_inline));//not used
	inline static void write32Both(const uint32_t val) __attribute__((always_inline));
	inline static void write32F0(const uint32_t val) __attribute__((always_inline));
	inline static void write32F1(const uint32_t val) __attribute__((always_inline));
	static uint16_t readByteBoth(void);
	static void readBytes( uint8_t * const buf, const int len);
};

extern BiParFlashChip BiParFlash;


class BiParFlashFile
{
public:
	BiParFlashFile() : address(0) {
	}
	operator bool() {
		if (address > 0) return true;
		return false;
	}
	uint32_t read(void *buf, uint32_t rdlen) {
		if (offset + rdlen > length) {
			if (offset >= length) return 0;
			rdlen = length - offset;
		}
		BiParFlash.read(address + offset, buf, rdlen);
		offset += rdlen;
		return rdlen;
	}
	uint32_t write(const void *buf, uint32_t wrlen) {
		if (offset + wrlen > length) {
			if (offset >= length) return 0;
			wrlen = length - offset;
		}
		BiParFlash.write(address + offset, buf, wrlen);
		offset += wrlen;
		return wrlen;
	}
	void seek(uint32_t n) {
		offset = n;
	}
	uint32_t position() {
		return offset;
	}
	uint32_t size() {
		return length;
	}
	uint32_t available() {
		if (offset >= length) return 0;
		return length - offset;
	}
	void erase();
	void flush() {
	}
	void close() {
	}
	uint32_t getFlashAddress() {
		return address;
	}
protected:
	friend class BiParFlashChip;
	uint32_t address;  // where this file's data begins in the Flash, or zero
	uint32_t length;   // total length of the data in the Flash chip
	uint32_t offset; // current read/write offset in the file
	uint16_t dirindex;
};


#endif
