#pragma once

#include <iostream>
#include <cstdint>
#include <winsock2.h>
#include <algorithm> // Required for std::max

class BasicStream {
public:
	BasicStream(std::size_t preallocateSize = 0);

	~BasicStream();

	char* data;
	std::size_t index;
	std::size_t readable; // Represents the number of valid bytes in the buffer
	std::size_t capacity;  // Represents the allocated size of the buffer

	int add_data(SOCKET clientSocket);

	std::size_t available() const;

	char read();

	uint8_t read_unsigned_byte();

	uint16_t read_unsigned_short();

	uint32_t read3Byte();

	uint32_t read_unsigned_int();

	__int64 readQWord();

	void readString(char* output);

	std::string read_std_string();

	const char* read_string();

	void write_byte(uint8_t byte);

	void write_short(uint16_t value);
	void write_3byte(uint32_t value);

	void write_int(uint32_t value);

	void write_qword(__int64 value);
	void write_string(const char* s);

	void ensure_capacity(std::size_t additional_bytes);
	void create_frame(int id);

	void create_frame_var_size(int id);

	void end_frame_var_size();

	void create_frame_var_size_word(int id);

	void end_frame_var_size_word();

	void init_bit_access();

	void write_bits(int num_bits, int value);

	void finish_bit_access();


	void skip_bytes(std::size_t bytes_to_skip);


	void mark_position();

	void reset();

	void delete_marked_block();


	void delete_bytes_from_mark(const std::size_t bytes_to_delete);


	void delete_bytes_from_index(std::size_t bytes_to_delete);
	void clear();

	char* get_data() const { return data; }

	std::size_t get_index() const { return index; }

private:
	std::size_t mark;
	int bit_position;
	int bit_mask_out[32]{};
	static constexpr int frame_stack_size = 100;
	int frame_stack[frame_stack_size] = { 0 };
	int frame_stack_ptr = -1;
};