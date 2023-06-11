#ifndef BYTEBUFFER_HPP
#define BYTEBUFFER_HPP

#include "swf_utils.hpp"  // bytestodec_le

#include <cstddef>        // std::size_t
#include <cstdint>        // std::uint8_t
#include <exception>
#include <string>
#include <vector>

namespace apksigner {
	
class buffer_overflow_exception : public std::exception {
	public:
		explicit buffer_overflow_exception(const std::string &message = "buffer_overflow_exception")
			: std::exception(), error_message(message) {}
		const char *what() const noexcept
		{
			return error_message.c_str();
		}
	private:
		std::string error_message;
};

class buffer_underflow_exception : public std::exception {
	public:
		explicit buffer_underflow_exception(const std::string &message = "buffer_underflow_exception")
			: std::exception(), error_message(message) {}
		const char *what() const noexcept
		{
			return error_message.c_str();
		}
	private:
		std::string error_message;
};
	
class ByteBuffer {
	std::vector<std::uint8_t> buffer;
	std::size_t pos;
public:
	ByteBuffer() : buffer(), pos(0) {}
	ByteBuffer(std::size_t size) : buffer(size, 0), pos(0) {}
	ByteBuffer(const std::vector<std::uint8_t> &_buffer) : buffer(_buffer), pos(0) {}
	ByteBuffer(const ByteBuffer& bb, std::size_t start, std::size_t end)
		: buffer(bb.buffer.begin()+start, bb.buffer.begin()+end), pos(0) {}
	ByteBuffer(const ByteBuffer& bb) : buffer(bb.buffer), pos(bb.pos) {}
	ByteBuffer& operator= (const ByteBuffer& bb) {
		this->buffer = bb.buffer;
		this->pos = bb.pos;
		return *this;
	}
	void setPosition(const std::size_t newPosition) {
		if (newPosition > buffer.size()) throw buffer_underflow_exception("New position cannot be greater to the buffer's size.");
		pos = newPosition;
	}
	std::size_t getPosition() const { return pos; }
	std::size_t remaining() const { return buffer.size() - pos; }
	bool hasRemaining() const { return (this->remaining() > 0); }
	std::size_t getSize() const { return buffer.size(); }
	const std::vector<std::uint8_t>& getBuffer() { return this->buffer; }
	std::uint32_t getUnsignedInt32() {
		auto u32 = this->getUnsignedInt32(pos);
		pos += 4;
		return u32;
	}
	std::uint32_t getUnsignedInt32(std::size_t position) {
		if ((buffer.size()-position) < 4) throw buffer_underflow_exception("There are less than 4 bytes left to read in the buffer.");
		auto u32 = bytestodec_le<std::uint32_t>(buffer.data() + position);
		return u32;
	}
	std::uint16_t getUnsignedInt16() {
		auto u16 = this->getUnsignedInt16(pos);
		pos += 2;
		return u16;
	}
	std::uint16_t getUnsignedInt16(std::size_t position) {
		if ((buffer.size()-position) < 2) throw buffer_underflow_exception("There are less than 2 bytes left to read in the buffer.");
		auto u16 = bytestodec_le<std::uint16_t>(buffer.data() + position);
		return u16;
	}
	std::uint8_t getUnsignedInt8() {
		auto u8 = this->getUnsignedInt8(pos);
		pos += 1;
		return u8;
	}
	std::uint8_t getUnsignedInt8(std::size_t position) {
		if ((buffer.size()-position) < 1) throw buffer_underflow_exception("There is less than 1 byte left to read in the buffer.");
		auto u8 = bytestodec_le<std::uint8_t>(buffer.data() + position);
		return u8;
	}
	ByteBuffer& putUnsignedInt32(std::uint32_t value) {
		auto arr = dectobytes_le(value);
		this->put(arr);
		pos+=4;
		return *this;
	}
	ByteBuffer& putUnsignedInt32(std::size_t position, std::uint32_t value) {
		auto arr = dectobytes_le(value);
		this->put(arr);
		return *this;
	}
	ByteBuffer& putUnsignedInt16(std::uint16_t value) {
		auto arr = dectobytes_le(value);
		this->put(arr);
		pos+=2;
		return *this;
	}
	ByteBuffer& putUnsignedInt16(std::size_t position, std::uint16_t value) {
		auto arr = dectobytes_le(value);
		this->put(arr);
		return *this;
	}
	ByteBuffer& putUnsignedInt8(std::uint8_t value) {
		this->buffer[this->pos] = value;
		++pos;
		return *this;
	}
	ByteBuffer& putUnsignedInt8(std::size_t position, std::uint8_t value) {
		this->buffer[position] = value;
		return *this;
	}
	ByteBuffer& put(ByteBuffer& src) {
		if (src.remaining() > this->remaining()) {
			throw buffer_overflow_exception("There are more bytes remaining in the source buffer than in this buffer.");
		}
		while (src.hasRemaining()) {
			this->putUnsignedInt8(src.getUnsignedInt8());
		}
	}
	ByteBuffer& put(uint8_t* src, std::size_t offset, std::size_t length) {
		for (std::size_t i = offset; i < offset + length; i++) {
			this->putUnsignedInt8(src[i]);
		}
		return *this;
	}
private:
	
	template<class container>
	void put(const container& arr) {
		for (std::size_t i = 0; i < arr.size(); ++i) {
			this->buffer[this->pos + i] = arr[i];
		}
	}
};

}  // namespace apksigner

#endif
