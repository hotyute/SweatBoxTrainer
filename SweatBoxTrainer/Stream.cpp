#include "Stream.h"

#include <iostream>

#define _WINSOCK2API_
#include <windows.h> //for everything extra
#include <stdlib.h> //for NULL (conversations s2i s2l s2ul s2d) rand srand system
#include <stdio.h>  //for printf()


Stream::Stream() {
	buffer = new char[5000];
	memset(buffer, 0, 5000);
	capacity = 5000;

	currentOffset = 0;
	markedReaderIndex = 0;
	for (int i = 0; i < 32; i++)
		bitMaskOut[i] = (1 << i) - 1;
	frameStackPtr = -1;
	memset(frameStack, 0, frameStackSize * sizeof(int));
}

Stream::Stream(int newSize) {
	buffer = new char[newSize];
	memset(buffer, 0, newSize);
	capacity = newSize;

	currentOffset = 0;
	markedReaderIndex = 0;
	for (int i = 0; i < 32; i++)
		bitMaskOut[i] = (1 << i) - 1;
	frameStackPtr = -1;
	memset(frameStack, 0, frameStackSize * sizeof(int));
}

Stream::~Stream() {
	delete[] buffer;
}

int Stream::remaining() {
	return currentOffset < writeIndex ? writeIndex - currentOffset : 0;
}

bool Stream::markReaderIndex()
{
	markedReaderIndex = currentOffset;
	return false;
}

bool Stream::resetReaderIndex()
{
	currentOffset = markedReaderIndex;
	return false;
}

bool Stream::deleteReaderBlock() {
	if (currentOffset == 0)
	{
		return false;
	}
	if (writeIndex != currentOffset)
	{
		memcpy(buffer, buffer + currentOffset, writeIndex - currentOffset);
		writeIndex -= currentOffset;
		adjustMarkers(currentOffset);
		currentOffset = 0;
	}
	else
	{
		adjustMarkers(currentOffset);
		writeIndex = currentOffset = 0;
	}
	return false;
}

void Stream::adjustMarkers(int decrement) {
	if (markedReaderIndex <= decrement) {
		markedReaderIndex = 0;
		if (markedWriterIndex <= decrement) {
			markedWriterIndex = 0;
		}
		else {
			markedWriterIndex -= decrement;
		}
	}
	else {
		markedReaderIndex -= decrement;
		markedWriterIndex -= decrement;
	}
}

bool Stream::clearBuf()
{
	markedReaderIndex = 0;
	ZeroMemory(buffer, capacity);
	currentOffset = writeIndex = 0;
	return false;
}

void Stream::createFrame(int id) {
	buffer[writeIndex++] = (unsigned char)id;
}

void Stream::createFrameVarSize(int id) { // creates a variable sized
											// frame
	buffer[writeIndex++] = (unsigned char)id;
	buffer[writeIndex++] = 0; // placeholder for size byte
	if (frameStackPtr >= frameStackSize - 1) {
		printf("Stack overflow\n");
	}
	else
		frameStack[++frameStackPtr] = writeIndex;
}

void Stream::createFrameVarSizeWord(int id) { // creates a variable sized
												// frame
	buffer[writeIndex++] = (unsigned char)id;
	writeWord(0); // placeholder for size word
	if (frameStackPtr >= frameStackSize - 1) {
		printf("Stack overflow\n");
	}
	else
		frameStack[++frameStackPtr] = writeIndex;
}

void Stream::endFrameVarSize() {// ends a variable sized frame
	if (frameStackPtr < 0)
		printf("Stack empty (byte)\n");
	else
		writeFrameSize(writeIndex - frameStack[frameStackPtr--]);
}

void Stream::endFrameVarSizeWord() { // ends a variable sized frame
	if (frameStackPtr < 0)
		printf("Stack empty (short)\n");
	else
		writeFrameSizeWord(writeIndex - frameStack[frameStackPtr--]);
}


//Write types
void Stream::writeByte(int i) {
	buffer[writeIndex++] = (char)i; //umm (byte)?
}

void Stream::writeByteA(int i) {
	buffer[writeIndex++] = (char)(i + 128);
}

void Stream::writeByteC(int i) {
	buffer[writeIndex++] = (char)(-i);
}

void Stream::writeBytes(char abyte0[], int i, int j) {
	for (int k = j; k < j + i; k++)
		buffer[writeIndex++] = abyte0[k];

}

void Stream::writeByteS(int i) {
	buffer[writeIndex++] = (char)(128 - i);
}

void Stream::writeBytes_reverse(char abyte0[], int i, int j) {
	for (int k = (j + i) - 1; k >= j; k--)
		buffer[writeIndex++] = abyte0[k];

}

void Stream::writeBytes_reverseA(char abyte0[], int i, int j) {
	for (int k = (j + i) - 1; k >= j; k--)
		buffer[writeIndex++] = (char)(abyte0[k] + 128);

}

void Stream::write3Byte(int i) {
	buffer[writeIndex++] = (char)(i >> 16);
	buffer[writeIndex++] = (char)(i >> 8);
	buffer[writeIndex++] = (char)i;
}

void Stream::writeDWord(int i) {
	buffer[writeIndex++] = (char)(i >> 24);
	buffer[writeIndex++] = (char)(i >> 16);
	buffer[writeIndex++] = (char)(i >> 8);
	buffer[writeIndex++] = (char)i;
}

void Stream::writeDWord_v1(int i) {
	buffer[writeIndex++] = (char)(i >> 8);
	buffer[writeIndex++] = (char)i;
	buffer[writeIndex++] = (char)(i >> 24);
	buffer[writeIndex++] = (char)(i >> 16);
}

void Stream::writeDWord_v2(int i) {
	buffer[writeIndex++] = (char)(i >> 16);
	buffer[writeIndex++] = (char)(i >> 24);
	buffer[writeIndex++] = (char)i;
	buffer[writeIndex++] = (char)(i >> 8);
}

void Stream::writeDWordBigEndian(int i) {
	buffer[writeIndex++] = (char)i;
	buffer[writeIndex++] = (char)(i >> 8);
	buffer[writeIndex++] = (char)(i >> 16);
	buffer[writeIndex++] = (char)(i >> 24);
}

void Stream::writeFrameSize(int i) {
	buffer[writeIndex - i - 1] = (char)i;
}

void Stream::writeFrameSizeWord(int i) {
	buffer[writeIndex - i - 2] = (char)(i >> 8);
	buffer[writeIndex - i - 1] = (char)i;
}

void Stream::writeQWord(unsigned __int64 l) {
	buffer[writeIndex++] = (char)(unsigned int)(l >> 56);
	buffer[writeIndex++] = (char)(unsigned int)(l >> 48);
	buffer[writeIndex++] = (char)(unsigned int)(l >> 40);
	buffer[writeIndex++] = (char)(unsigned int)(l >> 32);
	buffer[writeIndex++] = (char)(unsigned int)(l >> 24);
	buffer[writeIndex++] = (char)(unsigned int)(l >> 16);
	buffer[writeIndex++] = (char)(unsigned int)(l >> 8);
	buffer[writeIndex++] = (char)(unsigned int)l;
}

void Stream::writeString(char* s) {
	memcpy(buffer + writeIndex, s, strlen(s));
	writeIndex += strlen(s);
	buffer[writeIndex++] = 0;
}

void Stream::writeWord(int i) {
	buffer[writeIndex++] = (char)(i >> 8);
	buffer[writeIndex++] = (char)i;
}

void Stream::writeWordA(int i) {
	buffer[writeIndex++] = (char)(i >> 8);
	buffer[writeIndex++] = (char)(i + 128);
}

void Stream::writeWordBigEndian(int i) {
	buffer[writeIndex++] = (char)i;
	buffer[writeIndex++] = (char)(i >> 8);
}
void Stream::writeWordBigEndian_dup(int i) {
	buffer[writeIndex++] = (char)i;
	buffer[writeIndex++] = (char)(i >> 8);
}

void Stream::writeWordBigEndianA(int i) {
	buffer[writeIndex++] = (char)(i + 128);
	buffer[writeIndex++] = (char)(i >> 8);
}

//bit editing
void Stream::initBitAccess() {
	bitPosition = writeIndex * 8;
}

void Stream::writeBits(int numBits, int value) {
	int bytePos = bitPosition >> 3;
	int bitOffset = 8 - (bitPosition & 7);
	bitPosition += numBits;
	for (; numBits > bitOffset; bitOffset = 8) {
		buffer[bytePos] &= ~bitMaskOut[bitOffset];		// mask out the desired area
		buffer[bytePos++] |= (value >> (numBits - bitOffset)) & bitMaskOut[bitOffset];
		numBits -= bitOffset;
	}
	if (numBits == bitOffset) {
		buffer[bytePos] &= ~bitMaskOut[bitOffset];
		buffer[bytePos] |= value & bitMaskOut[bitOffset];
	}
	else {
		buffer[bytePos] &= ~(bitMaskOut[numBits] << (bitOffset - numBits));
		buffer[bytePos] |= (value & bitMaskOut[numBits]) << (bitOffset - numBits);
	}
}

void Stream::finishBitAccess() {
	writeIndex = (bitPosition + 7) / 8;
}

//read types

void Stream::readBytes(char abyte0[], int i, int j) {
	for (int k = j; k < j + i; k++)
		abyte0[k] = buffer[currentOffset++];
}

void Stream::readBytes_reverse(char abyte0[], int i, int j) {
	for (int k = (j + i) - 1; k >= j; k--) {
		abyte0[k] = buffer[currentOffset++];

	}
}

void Stream::readBytes_reverseA(char abyte0[], int i, int j) {
	for (int k = (j + i) - 1; k >= j; k--)
		abyte0[k] = (char)(buffer[currentOffset++] - 128);

}

unsigned int Stream::readDWord() {
	currentOffset += 4;
	return ((buffer[currentOffset - 4] & 0xff) << 24)
		+ ((buffer[currentOffset - 3] & 0xff) << 16)
		+ ((buffer[currentOffset - 2] & 0xff) << 8)
		+ (buffer[currentOffset - 1] & 0xff);
}

int Stream::readDWord_v1() {
	currentOffset += 4;
	return ((buffer[currentOffset - 2] & 0xff) << 24)
		+ ((buffer[currentOffset - 1] & 0xff) << 16)
		+ ((buffer[currentOffset - 4] & 0xff) << 8)
		+ (buffer[currentOffset - 3] & 0xff);
}

int Stream::readDWord_v2() {
	currentOffset += 4;
	return ((buffer[currentOffset - 3] & 0xff) << 24)
		+ ((buffer[currentOffset - 4] & 0xff) << 16)
		+ ((buffer[currentOffset - 1] & 0xff) << 8)
		+ (buffer[currentOffset - 2] & 0xff);
}

unsigned  __int64 Stream::readQWord() {
	unsigned int dw1 = readDWord();
	unsigned int dw2 = readDWord();
	return (((__int64)dw1) << 32) | (__int64)dw2;
}

char Stream::readSignedByte() {
	return buffer[currentOffset++];
}

char Stream::readSignedByteA() {
	return (char)(buffer[currentOffset++] - 128);
}

char Stream::readSignedByteC() {
	return (char)(-buffer[currentOffset++]);
}

char Stream::readSignedByteS() {
	return (char)(128 - buffer[currentOffset++]);
}

int Stream::readSignedWord() {
	currentOffset += 2;
	int i = ((buffer[currentOffset - 2] & 0xff) << 8)
		+ (buffer[currentOffset - 1] & 0xff);
	if (i > 32767) {
		i -= 0x10000;
	}
	return i;
}
int Stream::readSignedWordA() {
	currentOffset += 2;
	int i = ((buffer[currentOffset - 2] & 0xff) << 8)
		+ (buffer[currentOffset - 1] - 128 & 0xff);
	if (i > 32767) {
		i -= 0x10000;
	}
	return i;
}

int Stream::readSignedWordBigEndian() {
	currentOffset += 2;
	int i = ((buffer[currentOffset - 1] & 0xff) << 8)
		+ (buffer[currentOffset - 2] & 0xff);
	if (i > 32767)
		i -= 0x10000;
	return i;
}

int Stream::readSignedWordBigEndianA() {
	currentOffset += 2;
	int i = ((buffer[currentOffset - 1] & 0xff) << 8)
		+ (buffer[currentOffset - 2] - 128 & 0xff);
	if (i > 32767)
		i -= 0x10000;
	return i;
}

void Stream::readString(char* output) {
	int count = 0;
	byte b;
	int outputOffset = 0;
	while ((b = buffer[currentOffset++]) != 0 && count++ < 5000) {
		output[outputOffset++] = b;
	}
	output[outputOffset++] = '\0';//Null Terminator
}

unsigned char Stream::readUnsignedByte() {
	return buffer[currentOffset++] & 0xff;
}

unsigned char Stream::readUnsignedByteA() {
	return buffer[currentOffset++] - 128 & 0xff;
}

unsigned char Stream::readUnsignedByteC() {
	return -buffer[currentOffset++] & 0xff;
}

unsigned char Stream::readUnsignedByteS() {
	return 128 - buffer[currentOffset++] & 0xff;
}

int Stream::readUnsignedWord() {
	currentOffset += 2;
	return ((buffer[currentOffset - 2] & 0xff) << 8)
		+ (buffer[currentOffset - 1] & 0xff);
}

int Stream::read3Byte() {
	currentOffset += 3;
	int i = (((buffer[currentOffset - 3] & 0xff) << 16)
		+ ((buffer[currentOffset - 2] & 0xff) << 8)
		+ (0xff & buffer[currentOffset - 1]));
	if (i > 8388607) {
		i -= 16777216;
	}
	return i;
}

int Stream::readUnsignedWordA() {
	currentOffset += 2;
	return ((buffer[currentOffset - 2] & 0xff) << 8)
		+ (buffer[currentOffset - 1] - 128 & 0xff);
}

int Stream::readUnsignedWordBigEndian() {
	currentOffset += 2;
	return ((buffer[currentOffset - 1] & 0xff) << 8)
		+ (buffer[currentOffset - 2] & 0xff);
}

int Stream::readUnsignedWordBigEndianA() {
	currentOffset += 2;
	return ((buffer[currentOffset - 1] & 0xff) << 8)
		+ (buffer[currentOffset - 2] - 128 & 0xff);
}

int Stream::peek(int position)
{
	return (buffer[currentOffset + position] & 0xff);
}
