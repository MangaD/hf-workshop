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
		l18n::localization io;
		std::unique_ptr<swf::SWF> swf;

		/**
		 * v0.7:
		 * 
		 * 234 - Data.Global_storylist_xml
		 * 219 - Data.Global_story01_xml
		 * 398 - Data.Global_story02_xml
		 * 226 - Data.Global_story03_xml
		 * 299 - Data.Global_story04_xml
		 * 305 - Data.Global_story05_xml
		 * 454 - Data.Global_story06_xml
		 * 170 - Data.Global_story07_xml
		 */
		std::vector<size_t> stages_ids;

		/**
		 * v0.7:
		 * 
		 * 136: Data.Global_sinanSpt, 140: Data.Global_lucas2Spt, 141: Data.Global_sawsLmi, 142: Data.Global_globalDat,
		 * 144: Data.Global_jasonLmi, 149: Data.Global_horseMonsterSpt, 150: Data.Global_bg0Bgi,
		 * 156: Data.Global_sawsSpt, 163: Data.Global_giggsSpt, 167: Data.Global_easonLmi, 175: Data.Global_z_sickle01Spt,
		 * 179: Data.Global_z_dagger01Spt, 180: Data.Global_iczzyLmi, 181: Data.Global_globalLmi, 182: Data.Global_z_swordsman01Spt,
		 * 185: Data.Global_itemSpt, 191: Data.Global_z_wrestlerLmi, 195: Data.Global_z_skullLmi, 197: Data.Global_taylorSpt,
		 * 198: Data.Global_jasonSpt, 201: Data.Global_yagaSpt, 203: Data.Global_horseSpt, 205: Data.Global_z_infantry01Lmi,
		 * 214: Data.Global_horseTriceratopsSpt, 220: Data.Global_sinanLmi, 223: Data.Global_drewLmi, 224: Data.Global_model0Spt,
		 * 225: Data.Global_livermoreSpt, 229: Data.Global_drewSpt, 230: Data.Global_z_icemanSpt, 233: Data.Global_vivianSpt,
		 * 237: Data.Global_heaterLmi, 250: Data.Global_z_bandit01Spt, 277: Data.Global_tittoSpt, 278: Data.Global_z_woman01Spt,
		 * 279: Data.Global_z_wrestlerSpt, 280: Data.Global_tittoLmi, 290: Data.Global_vivianLmi, 294: Data.Global_leoLmi,
		 * 295: Data.Global_shawnSpt, 296: Data.Global_jennySpt, 297: Data.Global_lucas2Lmi, 311: Data.Global_eason0Spt,
		 * 314: Data.Global_z_bandit01Lmi, 315: Data.Global_gordonSpt, 316: Data.Global_rayeSpt, 318: Data.Global_easonSpt,
		 * 326: Data.Global_z_axemanSpt, 330: Data.Global_leggeSpt, 332: Data.Global_giggsLmi, 335: Data.Global_gordonLmi,
		 * 336: Data.Global_jennyLmi, 338: Data.Global_leoSpt, 340: Data.Global_z_infantry01Spt, 341: Data.Global_kidsSpt
		 * 347: Data.Global_bgLmi, 348: Data.Global_model0Lmi, 363: Data.Global_globalSpt, 369: Data.Global_heaterSpt
		 * 371: Data.Global_lucasLmi, 380: Data.Global_z_archer01Spt, 385: Data.Global_z_villager01Spt, 390: Data.Global_animalLmi
		 * 394: Data.Global_z_villager01Lmi, 396: Data.Global_rayeLmi, 397: Data.Global_itemLmi, 411: Data.Global_z_skullSpt
		 * 415: Data.Global_livermoreLmi, 421: Data.Global_shawnLmi, 425: Data.Global_leggeLmi, 433: Data.Global_z_sorcerer01Spt
		 * 441: Data.Global_iczzySpt, 442: Data.Global_lucasSpt, 444: Data.Global_z_icemanLmi, 445: Data.Global_z_woman01Lmi
		 * 448: Data.Global_yagaLmi, 465: Data.Global_taylorLmi, 468: Data.Global_z_cavalry01Spt
		 */
		std::vector<size_t> data_ids;
	};

} // hf_workshop

#endif // HF_WORKSHOP_HPP
