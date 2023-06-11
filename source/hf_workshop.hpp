/**
 * HF Workshop - Main class
 */

#ifndef HF_WORKSHOP_HPP
#define HF_WORKSHOP_HPP

#include <string>
#include <vector>
#include <array>
#include <memory>  // std::std::unique_ptr

#include "swf.hpp"
#include "io_wrapper.hpp"

namespace hf_workshop {

	class hfw_exception : public std::exception {
		public:
			explicit hfw_exception(const std::string &message = "hfw_exception")
				: std::exception(), error_message(message) {}
			const char *what() const noexcept
			{
				return error_message.c_str();
			}
		private:
			std::string error_message;
	};

	class hfw {
	public:
		hfw();

		void printHeader();
		void printHelp();
		void printOptions(const std::vector<std::string> &);

		void readFile();
		void fillDataIDs();
		void showMenuMain();
		void showMenuStages();
		void showMenuImages();
		void showMenuSounds();
		void showMenuData();

		void listTagsWithIds(std::vector<size_t> ids);
		void listTagsOfType(int id);

		int exportStages(std::vector<size_t> &ids);
		int exportImages(std::vector<size_t> &ids);
		int exportSounds(std::vector<size_t> &ids);
		int exportData(std::vector<size_t> &ids);

		void replaceStage(const size_t id, const std::string &stageFileName);
		void replaceImage(const size_t id, const std::string &imgFileName);
		void replaceSound(const size_t id, const std::string &mp3FileName);
		void replaceData(const size_t id, const std::string &dataFileName);

		void exportSwf();
		void exportExe();

	private:
		bool unsaved;
		bool isHFX;
		l18n::localization io;
		std::unique_ptr<swf::SWF> swf;

		std::vector<size_t> stages_ids;
		std::vector<size_t> data_ids;
	};

} // hf_workshop

#endif // HF_WORKSHOP_HPP
