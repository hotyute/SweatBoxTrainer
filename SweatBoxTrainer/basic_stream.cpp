#include "basic_stream.h"

BasicStream::BasicStream(std::size_t preallocateSize)
	: data(nullptr), index(0), readable(0), capacity(0), mark(SIZE_MAX), bit_position(0) {

	if (preallocateSize > 0) {
		data = new char[preallocateSize];
		capacity = preallocateSize;
	}

	for (int i = 0; i < 32; ++i) {
		bit_mask_out[i] = (1 << i) - 1;
	}
}

BasicStream::~BasicStream()
{
	delete[] data;
}

int BasicStream::add_data(SOCKET clientSocket)
{
	u_long bytes_available = 0;
	const int result = ioctlsocket(clientSocket, FIONREAD, &bytes_available);

	if (result == SOCKET_ERROR || bytes_available == 0) {
		if (result == SOCKET_ERROR) {
			std::cerr << "ioctlsocket failed with error: " << WSAGetLastError() << std::endl;
		}
		return result;
	}

	// Check if we have enough capacity. If not, grow the buffer.
	if (readable + bytes_available > capacity) {
		// Grow by doubling, or by the required amount if that's larger.
		std::size_t new_capacity = std::max(capacity * 2, readable + bytes_available);
		char* new_data = new char[new_capacity];

		if (data) {
			memcpy(new_data, data, readable);
			delete[] data;
		}

		data = new_data;
		capacity = new_capacity;
	}

	// Receive new data at the end of the existing content.
	const int bytes_read = recv(clientSocket, data + readable, bytes_available, 0);

	if (bytes_read == SOCKET_ERROR) {
		std::cerr << "recv failed with error: " << WSAGetLastError() << std::endl;
		return bytes_read;
	}

	// Increase the content size by the number of bytes read.
	readable += bytes_read;

	return bytes_read;
}

std::size_t BasicStream::available() const
{
	return (index < readable) ? readable - index : 0;
}

char BasicStream::read()
{
	if (index < readable) {
		return data[index++];
	}
	else {
		throw std::out_of_range("End of stream reached");
	}
}

uint8_t BasicStream::read_unsigned_byte()
{
	if (index < readable) {
		return static_cast<uint8_t>(data[index++]);
	}
	else {
		throw std::out_of_range("End of stream reached");
	}
}

uint16_t BasicStream::read_unsigned_short()
{
	if (index + 1 < readable) {
		const uint16_t value = (static_cast<uint8_t>(data[index]) << 8) |
			static_cast<uint8_t>(data[index + 1]);
		index += 2;
		return value;
	}
	else {
		throw std::out_of_range("End of stream reached");
	}
}

uint32_t BasicStream::read3Byte()
{
	if (index + 2 < readable) {
		uint32_t value = (static_cast<uint8_t>(data[index]) << 16) |
			(static_cast<uint8_t>(data[index + 1]) << 8) |
			static_cast<uint8_t>(data[index + 2]);
		if (value > 8388607) {
			value -= 16777216;
		}
		index += 3;
		return value;
	}
	else {
		throw std::out_of_range("End of stream reached");
	}
}

uint32_t BasicStream::read_unsigned_int()
{
	if (index + 3 < readable) {
		const uint32_t value = (static_cast<uint8_t>(data[index]) << 24) |
			(static_cast<uint8_t>(data[index + 1]) << 16) |
			(static_cast<uint8_t>(data[index + 2]) << 8) |
			static_cast<uint8_t>(data[index + 3]);
		index += 4;
		return value;
	}
	else {
		throw std::out_of_range("End of stream reached");
	}
}

long long BasicStream::readQWord()
{
	if (index + 7 < readable) {
		const __int64 value = (static_cast<__int64>(static_cast<uint8_t>(data[index])) << 56) |
			(static_cast<__int64>(static_cast<uint8_t>(data[index + 1])) << 48) |
			(static_cast<__int64>(static_cast<uint8_t>(data[index + 2])) << 40) |
			(static_cast<__int64>(static_cast<uint8_t>(data[index + 3])) << 32) |
			(static_cast<__int64>(static_cast<uint8_t>(data[index + 4])) << 24) |
			(static_cast<__int64>(static_cast<uint8_t>(data[index + 5])) << 16) |
			(static_cast<__int64>(static_cast<uint8_t>(data[index + 6])) << 8) |
			static_cast<__int64>(static_cast<uint8_t>(data[index + 7]));
		index += 8;
		return value;
	}
	else {
		throw std::out_of_range("End of stream reached");
	}
}

void BasicStream::readString(char* output)
{
	int count = 0;
	byte b;
	int outputOffset = 0;
	while ((b = data[index++]) != 0 && count++ < 5000) {
		output[outputOffset++] = b;
	}
	output[outputOffset++] = '\0';//Null Terminator
}

std::string BasicStream::read_std_string()
{
	if (index < readable) {
		std::string result;
		while (index < readable && data[index] != '\0' && data[index] != 0) {
			result += data[index++];
		}
		if (index < readable) {
			index++; // Skip the null terminator
		}
		return result;
	}
	else {
		throw std::out_of_range("End of stream reached");
	}
}

const char* BasicStream::read_string()
{
	if (index < readable) {
		const char* result = data + index;
		while (index < readable && data[index] != '\0' && data[index] != 0) {
			index++;
		}
		if (index < readable) {
			index++; // Skip the null terminator
		}
		return result;
	}
	else {
		throw std::out_of_range("End of stream reached");
	}
}

void BasicStream::write_byte(uint8_t byte)
{
	ensure_capacity(1);
	data[index++] = static_cast<char>(byte);
}

void BasicStream::write_short(uint16_t value)
{
	ensure_capacity(2);
	data[index++] = static_cast<char>(value >> 8);
	data[index++] = static_cast<char>(value & 0xFF);
}

void BasicStream::write_3byte(uint32_t value)
{
	ensure_capacity(3);
	data[index++] = static_cast<char>((value >> 16) & 0xFF);
	data[index++] = static_cast<char>((value >> 8) & 0xFF);
	data[index++] = static_cast<char>(value & 0xFF);
}

void BasicStream::write_int(uint32_t value)
{
	ensure_capacity(4);
	data[index++] = static_cast<char>((value >> 24) & 0xFF);
	data[index++] = static_cast<char>((value >> 16) & 0xFF);
	data[index++] = static_cast<char>((value >> 8) & 0xFF);
	data[index++] = static_cast<char>(value & 0xFF);
}

void BasicStream::write_qword(long long value)
{
	ensure_capacity(8);
	data[index++] = static_cast<char>((value >> 56) & 0xFF);
	data[index++] = static_cast<char>((value >> 48) & 0xFF);
	data[index++] = static_cast<char>((value >> 40) & 0xFF);
	data[index++] = static_cast<char>((value >> 32) & 0xFF);
	data[index++] = static_cast<char>((value >> 24) & 0xFF);
	data[index++] = static_cast<char>((value >> 16) & 0xFF);
	data[index++] = static_cast<char>((value >> 8) & 0xFF);
	data[index++] = static_cast<char>(value & 0xFF);
}

void BasicStream::write_string(const char* s) {
	size_t len = strlen(s);
	ensure_capacity(len + 1);
	memcpy(data + index, s, len);
	index += len;
	data[index++] = 0;
}

void BasicStream::ensure_capacity(std::size_t additional_bytes)
{
	if (index + additional_bytes <= capacity) {
		return;
	}

	// Grow by doubling, or by the required amount if that's larger.
	const std::size_t new_capacity = std::max(capacity * 2, index + additional_bytes);
	const auto new_data = new char[new_capacity];
	if (data) {
		memcpy(new_data, data, readable);
		delete[] data;
	}
	data = new_data;
	capacity = new_capacity;
}

void BasicStream::create_frame(int id) {
	ensure_capacity(1);
	data[index++] = static_cast<char>(id);
}

void BasicStream::create_frame_var_size(int id)
{
	ensure_capacity(2);
	write_byte(static_cast<uint8_t>(id));
	write_byte(0); // placeholder for size byte

	if (frame_stack_ptr >= frame_stack_size - 1) {
		std::cerr << "Stack overflow" << std::endl;
	}
	else {
		frame_stack[++frame_stack_ptr] = static_cast<int>(index);
	}
}

void BasicStream::end_frame_var_size()
{
	if (frame_stack_ptr < 0) {
		std::cerr << "Stack empty (byte)" << std::endl;
	}
	else {
		const std::size_t frame_size = index - frame_stack[frame_stack_ptr];
		data[frame_stack[frame_stack_ptr] - 1] = static_cast<char>(frame_size & 0xFF);
		frame_stack_ptr--;
	}
}

void BasicStream::create_frame_var_size_word(int id)
{
	ensure_capacity(3);
	write_byte(static_cast<uint8_t>(id));
	write_short(0); // placeholder for size short

	if (frame_stack_ptr >= static_cast<int>(frame_stack_size) - 1) {
		std::cerr << "Stack overflow" << std::endl;
	}
	else {
		frame_stack[++frame_stack_ptr] = static_cast<int>(index);
	}
}

void BasicStream::end_frame_var_size_word()
{
	if (frame_stack_ptr < 0) {
		std::cerr << "Stack empty (short)" << std::endl;
	}
	else {
		const std::size_t frame_size = index - frame_stack[frame_stack_ptr];
		data[frame_stack[frame_stack_ptr] - 2] = static_cast<char>((frame_size >> 8) & 0xFF);
		data[frame_stack[frame_stack_ptr] - 1] = static_cast<char>(frame_size & 0xFF);
		frame_stack_ptr--;
	}
}

void BasicStream::init_bit_access()
{
	bit_position = static_cast<int>(index) * 8;
}

void BasicStream::write_bits(int num_bits, int value)
{
	// Make sure we have enough capacity for the bitwise operations
	ensure_capacity((bit_position + num_bits + 7) / 8 - index);
	int byte_pos = bit_position >> 3;
	int bit_offset = 8 - (bit_position & 7);
	bit_position += num_bits;

	for (; num_bits > bit_offset; bit_offset = 8) {
		data[byte_pos] &= ~bit_mask_out[bit_offset];    // mask out the desired area
		data[byte_pos++] |= (value >> (num_bits - bit_offset)) & bit_mask_out[bit_offset];
		num_bits -= bit_offset;
	}
	if (num_bits == bit_offset) {
		data[byte_pos] &= ~bit_mask_out[bit_offset];
		data[byte_pos] |= value & bit_mask_out[bit_offset];
	}
	else {
		data[byte_pos] &= ~(bit_mask_out[num_bits] << (bit_offset - num_bits));
		data[byte_pos] |= (value & bit_mask_out[num_bits]) << (bit_offset - num_bits);
	}
}

void BasicStream::finish_bit_access()
{
	index = (bit_position + 7) / 8;
}

void BasicStream::skip_bytes(std::size_t bytes_to_skip)
{
	if (index + bytes_to_skip <= readable) {
		index += bytes_to_skip;
	}
	else {
		throw std::out_of_range("Cannot skip more bytes than available");
	}
}

void BasicStream::mark_position()
{
	mark = index;
}

void BasicStream::reset()
{
	if (mark != SIZE_MAX) {
		index = mark;
		mark = SIZE_MAX;
	}
	else {
		throw std::runtime_error("No mark has been set");
	}
}

void BasicStream::delete_marked_block()
{
	if (mark != SIZE_MAX) {
		const std::size_t bytes_to_move = readable - index;
		memmove(data + mark, data + index, bytes_to_move);
		readable -= (index - mark);
		index = mark;
		mark = SIZE_MAX;
	}
	else {
		throw std::runtime_error("No mark has been set");
	}
}

void BasicStream::delete_bytes_from_mark(const std::size_t bytes_to_delete)
{
	if (mark != SIZE_MAX) {
		const std::size_t bytes_to_move = readable - index;

		if (bytes_to_delete > bytes_to_move) {
			throw std::runtime_error("Cannot delete more bytes than available from mark position");
		}

		memmove(data + mark, data + index + bytes_to_delete, bytes_to_move - bytes_to_delete);
		readable -= (index - mark);
		readable -= bytes_to_delete;
		index = mark;
		mark = SIZE_MAX;
	}
	else {
		throw std::runtime_error("No mark has been set");
	}
}

void BasicStream::delete_bytes_from_index(std::size_t bytes_to_delete)
{
	const std::size_t bytes_to_copy = readable - index;
	if (bytes_to_delete > bytes_to_copy) {
		throw std::runtime_error("Cannot delete more bytes than available from current position");
	}
	else {
		memmove(data + index, data + index + bytes_to_delete, bytes_to_copy - bytes_to_delete);
		readable -= bytes_to_delete;
	}
}

void BasicStream::clear() {
	// Don't deallocate the buffer, just reset the pointers.
	// This makes reusing the stream for writing much faster.
	readable = 0;
	index = 0;
	mark = SIZE_MAX;
	bit_position = 0;
}