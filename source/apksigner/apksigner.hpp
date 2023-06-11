/**
 * C++ implementation of necessary parts from:
 * https://android.googlesource.com/platform/tools/apksig
 *
 * Overview: https://source.android.com/docs/security/features/apksigning
 */

#ifndef APKSIGNER_HPP
#define APKSIGNER_HPP

#include "../minizip_wrapper.hpp"   // Unzipper, Zipper

namespace apksigner {

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
	
	void sign(minizip::Unzipper &, minizip::Zipper &);

}  // namespace apksigner

#endif
