/**
 * C++ implementation of necessary parts from:
 * https://android.googlesource.com/platform/tools/apksig
 */

#ifndef APKSIGNER_HPP
#define APKSIGNER_HPP

#include "minizip_wrapper.hpp"   // Unzipper, Zipper
#include "swf_utils.hpp"         // bytestodec_le

namespace apksigner {

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

	/**
	 * Indicates that an error occurred while parsing a document.
	 */
	class xml_parser_exception : public std::exception {
		public:
			explicit xml_parser_exception(const std::string &message = "xml_parser_exception")
				: std::exception(), error_message(message) {}
			const char *what() const noexcept
			{
				return error_message.c_str();
			}
		private:
			std::string error_message;
	};

	class min_sdk_version_exception : public std::exception {
		public:
			explicit min_sdk_version_exception(const std::string &message = "min_sdk_version_exception")
				: std::exception(), error_message(message) {}
			const char *what() const noexcept
			{
				return error_message.c_str();
			}
		private:
			std::string error_message;
	};

	class ByteBuffer {
		std::vector<uint8_t> buffer;
		size_t pos;
	public:
		ByteBuffer() : buffer(), pos(0) {}
		ByteBuffer(const std::vector<uint8_t> &_buffer) : buffer(_buffer), pos(0) {}
		ByteBuffer(const ByteBuffer& bb, size_t start, size_t end) : buffer(bb.buffer.begin()+start, bb.buffer.begin()+end), pos(0) {}
		ByteBuffer(const ByteBuffer& bb) : buffer(bb.buffer), pos(bb.pos) {}
		ByteBuffer& operator= (const ByteBuffer& bb) {
			this->buffer = bb.buffer;
			this->pos = bb.pos;
			return *this;
		}
		void setPosition(const size_t newPosition) {
			if (newPosition > buffer.size()) throw buffer_underflow_exception("New position cannot be greater to the buffer's size.");
			pos = newPosition;
		}
		size_t getPosition() const { return pos; }
		size_t remaining() const { return buffer.size() - pos; }
		bool hasRemaining() const { return (this->remaining() > 0); }
		size_t size() const { return buffer.size(); }
		const std::vector<uint8_t>& getBuffer() { return this->buffer; }
		size_t getUnsignedInt32() {
			auto u32 = this->getUnsignedInt32(pos);
			pos += 4;
			return u32;
		}
		size_t getUnsignedInt32(size_t position) {
			if ((buffer.size()-position) < 4) throw buffer_underflow_exception("There are less than 4 bytes left to read in the buffer.");
			auto u32 = static_cast<size_t>(bytestodec_le<uint32_t>(buffer.data() + position));
			return u32;
		}
		size_t getUnsignedInt16() {
			auto u16 = this->getUnsignedInt16(pos);
			pos += 2;
			return u16;
		}
		size_t getUnsignedInt16(size_t position) {
			if ((buffer.size()-position) < 2) throw buffer_underflow_exception("There are less than 2 bytes left to read in the buffer.");
			auto u16 = static_cast<size_t>(bytestodec_le<uint16_t>(buffer.data() + position));
			return u16;
		}
		size_t getUnsignedInt8() {
			auto u8 = this->getUnsignedInt8(pos);
			pos += 1;
			return u8;
		}
		size_t getUnsignedInt8(size_t position) {
			if ((buffer.size()-position) < 1) throw buffer_underflow_exception("There is less than 1 byte left to read in the buffer.");
			auto u8 = static_cast<size_t>(bytestodec_le<uint8_t>(buffer.data() + position));
			return u8;
		}
	};

	void sign(minizip::Unzipper &, minizip::Zipper &);

}

#endif
