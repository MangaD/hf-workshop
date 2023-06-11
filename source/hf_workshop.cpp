/**
 * HF Workshop - Main class
 */

#include <string>
#include <vector>
#include <array>
#include <algorithm> // vector find, search, sort
#include <cstdint>   // uint8_t, uint32_t
#include <stdexcept> // std::exception
#include <map>       // std::map
#include <utility>    // std::pair

#include <json.hpp>

#include "minizip_wrapper.hpp"

#include <rlutil/rlutil.h>

#include "amf0.hpp"
#include "amf3.hpp"
#include "zlib_wrapper.hpp"

#include "swf.hpp"
#include "utils.hpp"
#include "swf_utils.hpp" // concatVectorWithContainer

// Because of libintl conflict with json, these headers must come after <json.hpp> and "amf0.hpp"
#include "io_wrapper.hpp"
#include "hf_workshop.hpp"

using namespace std;
using namespace swf;
using namespace l18n;
using namespace hf_workshop;

hfw::hfw() : unsaved(false), io(), swf(nullptr), stages_ids(),
             data_ids() {
	rlutil::setConsoleTitle(this->io.getText("HF Workshop"));
	this->printHeader();

	try {
		this->readFile();
		this->fillDataIDs();
		this->showMenuMain();
	} catch (const exception &e) {
		printf_error(io.getText("Error: %s\n"), e.what());
	}
}

void hfw::readFile() {
	string filename;
	bool fileNameError;
	do {
		fileNameError = false;
		try {
			readLine(filename, io.getText("File path: "));
			removequotes(trim(filename));

			vector<uint8_t> buffer;
			readBinaryFile(filename, buffer);

			swf = make_unique<SWF>(buffer);
		} catch (const exception &e) {
			printf_error(io.getText("Error: %s\n"), e.what());
			fileNameError = true;
		}
	} while (fileNameError);
}

void hfw::fillDataIDs() {
	vector<Tag *> tv = swf->getTagsOfType(SWF::tagId("DefineBinaryData"));

	for (auto &t : tv) {
		if (startsWith(t->symbolName, "Data.Global_story0") ||
		    t->symbolName.find("storylist") != std::string::npos) {
			stages_ids.push_back(t->id);
		} else if (endsWith(t->symbolName, "Spt") || endsWith(t->symbolName, "Lmi") ||
		           endsWith(t->symbolName, "_globalDat") || endsWith(t->symbolName, "_bg0Bgi")) {
			data_ids.push_back(t->id);
		}
	}
}

void hfw::showMenuMain() {

	while (true) {

		string choice;

		vector<string> options = {io.getText("Help"), io.getText("Edit stages"),
			io.getText("Edit sounds"), io.getText("Edit images"),
			io.getText("Edit data"), io.getText("Export SWF"),
			io.getText("Export EXE"), io.getText("Exit")};

		printOptions(options);

		readLine(choice);

		if (choice == "0") {
			string choice3;
			if (unsaved) {
				/// TRANSLATORS: Please don't change the y and n options.
				readLine(choice3, io.getText("You have unsaved changes. Are you sure you want to exit without exporting SWF/EXE? [y/n] (default=n): "));
				while (!(choice3 == "y" || choice3 == "Y" || choice3 == "n" || choice3 == "N" || choice3 == "")) {
					printf_error(io.getText("Invalid option.\n"));
					readLine(choice3);
				}
				if (choice3 == "n" || choice3 == "N" || choice3 == "") {
					continue;
				}
			}
			printf_normal(io.getText("Good bye. o/\n"));
			exit(0);
		} else if (choice == "1") {
			printHelp();
		} else if (choice == "2") {
			showMenuStages();
		} else if (choice == "3") {
			showMenuSounds();
		} else if (choice == "4") {
			showMenuImages();
		} else if (choice == "5") {
			showMenuData();
		} else if (choice == "6") {
			exportSwf();
		} else if (choice == "7") {
			exportExe();
		} else {
			printf_normal(io.getText("Invalid option.\n"));
		}
	}
}

void hfw::showMenuStages() {

	string choice2;
	while (true) {

		vector<string> options = {io.getText("List stage files"), io.getText("Export stage file(s)"),
			io.getText("Replace stage file"), io.getText("Back")};

		printOptions(options);

		readLine(choice2);
		if (choice2 == "0") {
			break;
		} else if (choice2 == "1") {
			listTagsWithIds(stages_ids);
		} else if (choice2 == "2") {
			printf_colored(rlutil::YELLOW, io.getText("Which stage(s) do you wish to export?\n"));
			string ids;
			/// TRANSLATORS: Please don't change the 'all' option
			readLine(ids, io.getText("Type 'all' or comma separated IDs: "));
			trim(ids);
			if (equalsIgnoreCase(ids, "all")) {
				int count = exportStages(stages_ids);
				printf_colored(rlutil::YELLOW, io.getText("Exported %d stage file(s).\n"), count);
			} else {
				vector<size_t> ids_v = getVectorOfNumbers<size_t>(ids);

				if (ids_v.empty()) {
					printf_error(io.getText("Invalid input.\n"));
					continue;
				}

				int count = exportStages(ids_v);
				printf_colored(rlutil::YELLOW, io.getText("Exported %d stage file(s).\n"), count);
			}
		} else if (choice2 == "3") {
			// Get id to replace
			printf_colored(rlutil::YELLOW, io.getText("Which stage file do you wish to replace?\n"));
			string id_s;
			readLine(id_s, io.getText("Stage ID: "));
			trim(id_s);
			size_t id, end;

			try {
				id = stoul(id_s, &end);
				if (end < id_s.length()) {
					printf_error(io.getText("Invalid number.\n"));
					continue;
				}
			} catch (const logic_error &lr) {
				printf_error(io.getText("Invalid number.\n"));
				continue;
			}

			// Get stage filename
			printf_colored(rlutil::YELLOW, io.getText("Which stage file do you wish to replace with? (.xml)\n"));
			string stage_s;
			readLine(stage_s, io.getText("File path: "));
			removequotes(trim(stage_s));

			replaceStage(id, stage_s);
		} else {
			printf_error(io.getText("Invalid option.\n"));
		}
	}
}

void hfw::showMenuImages() {

	string choice2;
	while (true) {

		vector<string> options = {io.getText("List images"), io.getText("Export image(s)"),
			io.getText("Replace image"), io.getText("Back")};

		printOptions(options);

		readLine(choice2);
		if (choice2 == "0") {
			break;
		} else if (choice2 == "1") {
			listTagsOfType(SWF::tagId("DefineBitsLossless"));
			listTagsOfType(SWF::tagId("DefineBitsLossless2"));
		} else if (choice2 == "2") {
			printf_colored(rlutil::YELLOW, io.getText("Which image(s) do you wish to export?\n"));
			string ids;
			readLine(ids, io.getText("Type 'all' or comma separated IDs: "));
			trim(ids);
			if (equalsIgnoreCase(ids, "all")) {
				vector<size_t> v{};
				exportImages(v);
			} else {
				vector<size_t> ids_v = getVectorOfNumbers<size_t>(ids);

				if (ids_v.empty()) {
					printf_error(io.getText("Invalid input.\n"));
					continue;
				}

				int count = exportImages(ids_v);
				printf_colored(rlutil::YELLOW, io.getText("Exported %d image(s).\n"), count);
			}
		} else if (choice2 == "3") {
			// Get id to replace
			printf_colored(rlutil::YELLOW, io.getText("Which image do you wish to replace?\n"));
			string id_s;
			readLine(id_s, io.getText("Image ID: "));
			trim(id_s);
			size_t id, end;

			try {
				id = stoul(id_s, &end);
				if (end < id_s.length()) {
					printf_error(io.getText("Invalid number.\n"));
					continue;
				}
			} catch (const logic_error &lr) {
				printf_error(io.getText("Invalid number.\n"));
				continue;
			}

			// Get mp3 filename
			printf_colored(rlutil::YELLOW, io.getText("Which image file do you wish to replace with?\n"));
			string img_s;
			readLine(img_s, io.getText("File path: "));
			removequotes(trim(img_s));

			replaceImage(id, img_s);
		} else {
			printf_error(io.getText("Invalid option.\n"));
		}
	}
}

void hfw::showMenuSounds() {

	string choice2;
	while (true) {

		vector<string> options = {io.getText("List sounds"), io.getText("Export sound(s)"),
			io.getText("Replace sound"), io.getText("Back")};

		printOptions(options);

		readLine(choice2);
		if (choice2 == "0") {
			break;
		} else if (choice2 == "1") {
			listTagsOfType(SWF::tagId("DefineSound"));
		} else if (choice2 == "2") {
			printf_colored(rlutil::YELLOW, io.getText("Which sound(s) do you wish to export?\n"));
			string ids;
			readLine(ids, io.getText("Type 'all' or comma separated IDs: "));
			trim(ids);
			if (equalsIgnoreCase(ids, "all")) {
				vector<size_t> v{};
				exportSounds(v);
			} else {
				vector<size_t> ids_v = getVectorOfNumbers<size_t>(ids);

				if (ids_v.empty()) {
					printf_error(io.getText("Invalid input.\n"));
					continue;
				}

				int count = exportSounds(ids_v);
				printf_colored(rlutil::YELLOW, io.getText("Exported %d sound(s).\n"), count);
			}
		} else if (choice2 == "3") {
			// Get id to replace
			printf_colored(rlutil::YELLOW, io.getText("Which MP3 sound do you wish to replace?\n"));
			string id_s;
			readLine(id_s, io.getText("Sound ID: "));
			trim(id_s);
			size_t id, end;

			try {
				id = stoul(id_s, &end);
				if (end < id_s.length()) {
					printf_error(io.getText("Invalid number.\n"));
					continue;
				}
			} catch (const logic_error &lr) {
				printf_error(io.getText("Invalid number.\n"));
				continue;
			}

			// Get mp3 filename
			printf_colored(rlutil::YELLOW, io.getText("Which MP3 file do you wish to replace with?\n"));
			string mp3_s;
			readLine(mp3_s, io.getText("File path: "));
			removequotes(trim(mp3_s));

			replaceSound(id, mp3_s);
		} else {
			printf_error(io.getText("Invalid option.\n"));
		}
	}
}

void hfw::showMenuData() {

	string choice2;
	while (true) {

		vector<string> options = {io.getText("List data files"), io.getText("Export data file(s)"),
			io.getText("Replace data file"), io.getText("Back")};

		printOptions(options);

		readLine(choice2);
		if (choice2 == "0") {
			break;
		} else if (choice2 == "1") {
			listTagsWithIds(data_ids);
		} else if (choice2 == "2") {
			printf_colored(rlutil::YELLOW, io.getText("Which data file(s) do you wish to export?\n"));
			string ids;
			readLine(ids, io.getText("Type 'all' or comma separated IDs: "));
			trim(ids);
			if (equalsIgnoreCase(ids, "all")) {
				vector<size_t> v{};
				exportData(v);
			} else {
				vector<size_t> ids_v = getVectorOfNumbers<size_t>(ids);

				if (ids_v.empty()) {
					printf_error(io.getText("Invalid input.\n"));
					continue;
				}

				int count = exportData(ids_v);
				printf_colored(rlutil::YELLOW, io.getText("Exported %d data file(s).\n"), count);
			}
		} else if (choice2 == "3") {
			// Get id to replace
			printf_colored(rlutil::YELLOW, io.getText("Which data file do you wish to replace?\n"));
			string id_s;
			readLine(id_s, io.getText("Data ID: "));
			trim(id_s);
			size_t id, end;

			try {
				id = stoul(id_s, &end);
				if (end < id_s.length()) {
					printf_error(io.getText("Invalid number.\n"));
					continue;
				}
			} catch (const logic_error &lr) {
				printf_error(io.getText("Invalid number.\n"));
				continue;
			}

			// Get mp3 filename
			printf_colored(rlutil::YELLOW, io.getText("Which data file do you wish to replace with? (.zip)\n"));
			string chr_s;
			readLine(chr_s, io.getText("File path: "));
			removequotes(trim(chr_s));

			replaceData(id, chr_s);
		} else {
			printf_error(io.getText("Invalid option.\n"));
		}
	}
}

void hfw::listTagsWithIds(vector<size_t> ids) {

	vector<pair<size_t, string>> tags;
	for (auto id : ids) {
		auto tag = static_cast<Tag_DefineBinaryData *>(swf->getTagWithId(id));
		if (tag == nullptr) {
			printf_error(io.getText("File with ID=%zu not found inside the game.\n"), id);
			continue;
		}
		tags.emplace_back(tag->id, tag->symbolName);
	}
	// Sort by name
	sort(tags.begin(), tags.end(), [](pair<size_t, string> const &p1,
	                                  pair<size_t, string> const &p2) { return p1.second < p2.second; });
	for (auto p : tags) {
		printf_colored(rlutil::LIGHTRED, "%zu", p.first);
		printf_normal(": %s\n", p.second.c_str());
	}
}

void hfw::listTagsOfType(int id) {

	vector<Tag *> tv = swf->getTagsOfType(id);

	// Sort by name
	sort(tv.begin(), tv.end(), [](Tag * const &t1, Tag * const &t2) { return t1->symbolName < t2->symbolName; });

	for (auto t : tv) {
		printf_colored(rlutil::LIGHTRED, "%zu", t->id);
		printf_normal(": %s\n", t->symbolName.c_str());
	}
}

int hfw::exportImages(vector<size_t> &ids) {

	int count = 0;
	vector<Tag *> tv = swf->getTagsOfType(SWF::tagId("DefineBitsLossless"));
	vector<Tag *> tv1 = swf->getTagsOfType(SWF::tagId("DefineBitsLossless2"));
	tv.insert(tv.end(), tv1.begin(), tv1.end());

	// Check if ids exist
	for (auto i : ids) {
		bool found = false;
		for (auto &t : tv) {
			auto dbl = static_cast<Tag_DefineBitsLossless *>(t);
			if (dbl->id == i) {
				found = true;
				break;
			}
		}
		if (!found) {
			printf_error(io.getText("No image with ID=%zu.\n"), i);
		}
	}

	for (auto &t : tv) {
		auto dbl = static_cast<Tag_DefineBitsLossless *>(t);
		auto found = find(ids.begin(), ids.end(), dbl->id);
		if (ids.empty() || found != ids.end()) {
			string name = dbl->symbolName;
			if (name == "") {
				name = to_string(dbl->id);
			} else {
				name = to_string(dbl->id) + " - " + name;
			}
			name += ".png";

			printf_normal(io.getText("Exporting: %s\n"), name.c_str());

			vector<uint8_t> imageData;
			try {
				imageData = swf->exportImage(dbl->id);
			} catch (const swf_exception &se) {
				printf_error(io.getText("Error for image with ID=%zu: %s\n"), dbl->id, se.what());
				continue;
			}

			writeBinaryFile(name, imageData);

			++count;
		}
	}
	return count;
}

int hfw::exportStages(vector<size_t> &ids) {

	int count = 0;

	Tag_DefineBinaryData * t;
	string name;
	for (auto id : ids) {

		if (find(stages_ids.begin(), stages_ids.end(), id) == stages_ids.end()) {
			printf_error(io.getText("No stage file with ID=%zu.\n"), id);
			continue;
		}

		t = static_cast<Tag_DefineBinaryData *>(swf->getTagWithId(id));
		if (t == nullptr) {
			printf_error(io.getText("Stage file with ID=%zu not found inside the game.\n"), id);
			continue;
		}
		name = to_string(t->id) + " - " + t->symbolName;
		name += ".xml";

		printf_normal(io.getText("Exporting: %s\n"), name.c_str());

		vector<uint8_t> xmlData;
		try {
			xmlData = swf->exportBinary(id);
		} catch (const swf_exception &se) {
			printf_error(io.getText("Error for stage file with ID=%zu: %s\n"), id, se.what());
			continue;
		}
		writeBinaryFile(name, xmlData);

		++count;
	}
	return count;
}

int hfw::exportSounds(vector<size_t> &ids) {

	int count = 0;
	vector<Tag *> tv = swf->getTagsOfType(SWF::tagId("DefineSound"));

	// Check if ids exist
	for (auto i : ids) {
		bool found = false;
		for (auto &t : tv) {
			auto ds = static_cast<Tag_DefineSound *>(t);
			if (ds->id == i) {
				found = true;
				break;
			}
		}
		if (!found) {
			printf_error(io.getText("No sound with ID=%zu.\n"), i);
		}
	}

	// Export
	for (auto &t : tv) {
		auto ds = static_cast<Tag_DefineSound *>(t);
		auto found = find(ids.begin(), ids.end(), ds->id);
		if (ids.empty() || found != ids.end()) {

			if (ds->soundFormat != 2) {
				printf_error(io.getText("Sound with ID=%zu is not an MP3 file.\n"), ds->id);
				continue;
			}

			string name = ds->symbolName;
			if (name == "") {
				name = to_string(ds->id);
			} else {
				name = to_string(ds->id) + " - " + name;
			}
			name += ".mp3";

			printf_normal(io.getText("Exporting: %s\n"), name.c_str());

			vector<uint8_t> mp3Data = swf->exportMp3(ds->id);

			writeBinaryFile(name, mp3Data);

			++count;
		}
	}
	return count;
}


int hfw::exportData(vector<size_t> &ids) {

	int count = 0;

	if (ids.empty()) {
		concatVectorWithContainer(ids, data_ids);
	}

	string name;
	for (auto id : ids) {

		if (find(data_ids.begin(), data_ids.end(), id) == data_ids.end()) {
			printf_error(io.getText("No data file with ID=%zu.\n"), id);
			continue;
		}

		auto t = static_cast<Tag_DefineBinaryData *>(swf->getTagWithId(id));
		if (t == nullptr) {
			printf_error(io.getText("Data file with ID=%zu not found inside the game.\n"), id);
			continue;
		}
		name = to_string(t->id) + " - " + t->symbolName;

		printf_normal(io.getText("Exporting: %s\n"), name.c_str());

		vector<uint8_t> data;
		try {
			data = swf->exportBinary(t->id);
			/**
			 * An AMF3 byte array that holds compressed data
			 */
			size_t pos = 0;
			AMF3 amf{data.data(), pos};

			if (amf.object->type != AMF3::BYTE_ARRAY_MARKER) {
				printf_error(io.getText("Error for data file with ID=%zu: Not an AMF3 ByteArray.\n"), t->id);
				goto CONTINUE_FOR_EACH_ID;
			}

			auto ba = static_cast<AMF3_BYTEARRAY *>(amf.object.get());
			data = zlib::zlib_decompress(ba->binaryData);

			//XXX Only for test
			/*
			writeBinaryFile(name + ".out", data);
			*/

			pos = 0;

			/**
			 * The decompressed byte array starts with an UTF string.
			 * An UTF string is prefixed by 2 bytes in big-endian specifying its length.
			 *
			 * https://help.adobe.com/en_US/FlashPlatform/reference/actionscript/3/flash/utils/ByteArray.html#readUTF()
			 */
			uint16_t strLen = bytestodec_be<uint16_t>(data.data()+pos);;
			pos += 2;
			string fileType;

			for(size_t i = pos; i < strLen+pos; ++i) {
				fileType += data[i];
			}
			pos += strLen;

			string comment = "Created with HF Workshop.";

			if (fileType == "limbInfo") { // HF v0.7 and less

				// Create zip file
				minizip::Zipper zipper(name + ".zip", comment);

				/**
				 * After the file type comes the number of LimbPic objects.
				 * A LimbPic object is serialized in AMF0 format and stored
				 * in an AMF3 byte array.
				 */
				int32_t numLP = static_cast<int32_t>(bytestodec_be<uint32_t>(data.data() + pos));
				pos += 4;

				for (int32_t i = 0; i < numLP; ++i) {

					AMF3 a{ data.data(), pos };
					if (a.object->type != AMF3::BYTE_ARRAY_MARKER) {
						printf_error(io.getText("Error for data file with ID=%zu: 'LimbPic' is not inside an AMF3 ByteArray.\n"), t->id);
						goto CONTINUE_FOR_EACH_ID;
					}
					auto ba2 = static_cast<AMF3_BYTEARRAY *>(a.object.get());

					// Parse AMF0
					AMF0 limbPic{ ba2->binaryData.data(), ba2->binaryData.size()};
					string json = limbPic.exportToJSON();

					// Add JSON to zip
					zipper.add("LimbPic_" + to_string(i) + ".json", "", Z_BEST_COMPRESSION, json);
				}

				for (int32_t i = 0; i < numLP; ++i) {
					AMF3 a{ data.data(), pos };

					if (a.object->type == AMF3::BYTE_ARRAY_MARKER) {
						auto ba2 = static_cast<AMF3_BYTEARRAY *>(a.object.get());
						zipper.add(to_string(i) + ".png", "", Z_BEST_COMPRESSION, ba2->binaryData);
					}
					// else 0x01 for null because not all limbPics have 'embeded' == true and 'disabled' == false

				}

				int32_t numLimb = static_cast<int32_t>(bytestodec_be<uint32_t>(data.data()+pos));
				pos += 4;

				for (int32_t i = 0; i < numLimb; ++i) {
					AMF3 a{ data.data(), pos };
					if (a.object->type != AMF3::BYTE_ARRAY_MARKER) {
						printf_error(io.getText("Error for data file with ID=%zu: 'Limb' is not inside an AMF3 ByteArray.\n"), t->id);
						goto CONTINUE_FOR_EACH_ID;
					}
					auto ba2 = static_cast<AMF3_BYTEARRAY *>(a.object.get());

					// Parse AMF0
					AMF0 limb{ ba2->binaryData.data(), ba2->binaryData.size() };
					string json = limb.exportToJSON();

					// Add JSON to zip
					zipper.add("Limb_" + to_string(i) + ".json", "", Z_BEST_COMPRESSION, json);
				}
			} else if (fileType == "Spt") { // HF v0.7 and less

				minizip::Zipper zipper(name + ".zip", comment);

				// Spt
				AMF3 a{ data.data(), pos };

				if (a.object->type != AMF3::BYTE_ARRAY_MARKER) {
					printf_error(io.getText("Error for data file with ID=%zu: 'Spt' is not inside an AMF3 ByteArray.\n"), t->id);
					goto CONTINUE_FOR_EACH_ID;
				}
				auto ba2 = static_cast<AMF3_BYTEARRAY *>(a.object.get());

				// Parse AMF0
				AMF0 spt{ ba2->binaryData.data(), ba2->binaryData.size() };
				string json = spt.exportToJSON();

				// Add JSON to zip
				zipper.add("Spt.json", "", Z_BEST_COMPRESSION, json);

			} else if (fileType == "Bg") { // HF v0.7 and less

				minizip::Zipper zipper(name + ".zip", comment);

				// BgInfoFile
				AMF3 a{ data.data(), pos };

				if (a.object->type != AMF3::BYTE_ARRAY_MARKER) {
					printf_error(io.getText("Error for data file with ID=%zu: 'Bg' is not inside an AMF3 ByteArray.\n"), t->id);
					goto CONTINUE_FOR_EACH_ID;
				}
				auto ba2 = static_cast<AMF3_BYTEARRAY *>(a.object.get());

				// Parse AMF0
				AMF0 bg{ ba2->binaryData.data(), ba2->binaryData.size() };
				string json = bg.exportToJSON();

				// Add JSON to zip
				zipper.add("BgInfoFile.json", "", Z_BEST_COMPRESSION, json);

			} else if (fileType == "gdat") { // HF v0.7 and less

				minizip::Zipper zipper(name + ".zip", comment);

				int32_t numA = static_cast<int32_t>(bytestodec_be<uint32_t>(data.data()+pos));
				pos += 4;

				for (int32_t i = 0; i < numA; ++i) {
					AMF3 a{ data.data(), pos };
					if (a.object->type != AMF3::BYTE_ARRAY_MARKER) {
						printf_error(io.getText("Error for data file with ID=%zu: 'Attack' is not inside an AMF3 ByteArray.\n"), t->id);
						goto CONTINUE_FOR_EACH_ID;
					}
					auto ba2 = static_cast<AMF3_BYTEARRAY *>(a.object.get());

					// Parse AMF0
					AMF0 attack{ ba2->binaryData.data(), ba2->binaryData.size() };
					string json = attack.exportToJSON();

					// Add JSON to zip
					zipper.add("Attack_" + to_string(i) + ".json", "", Z_BEST_COMPRESSION, json);
				}

				int32_t numPt = static_cast<int32_t>(bytestodec_be<uint32_t>(data.data()+pos));
				pos += 4;

				for (int32_t i = 0; i < numPt; ++i) {
					AMF3 a{ data.data(), pos };
					if (a.object->type != AMF3::BYTE_ARRAY_MARKER) {
						printf_error(io.getText("Error for data file with ID=%zu: 'PtWithName' is not inside an AMF3 ByteArray.\n"), t->id);
						goto CONTINUE_FOR_EACH_ID;
					}
					auto ba2 = static_cast<AMF3_BYTEARRAY *>(a.object.get());

					// Parse AMF0
					AMF0 ptWithName{ ba2->binaryData.data(), ba2->binaryData.size() };
					string json = ptWithName.exportToJSON();

					// Add JSON to zip
					zipper.add("PtWithName_" + to_string(i) + ".json", "", Z_BEST_COMPRESSION, json);
				}

			} else if (fileType == "limbInfoO") { // HFX

				// Create zip file
				minizip::Zipper zipper(name + ".zip", comment);

				/**
				 * After the file type comes the number of LimbPic objects.
				 * A LimbPic object in HFX is serialized in AMF3 format.
				 */
				int32_t numLP = static_cast<int32_t>(bytestodec_be<uint32_t>(data.data()+pos));
				pos += 4;

				// LimbPic objects (JSON)
				for (int32_t i = 0; i < numLP; ++i) {
					string json = (AMF3{ data.data(), pos }).exportToJSON();
					zipper.add("LimbPic_" + to_string(i) + ".json", "", Z_BEST_COMPRESSION, json);
				}

				// LimbPic pictures (PNG)
				for (int32_t i = 0; i < numLP; ++i) {
					AMF3 a{ data.data(), pos };

					if (a.object->type == AMF3::BYTE_ARRAY_MARKER) {
						auto png = static_cast<AMF3_BYTEARRAY *>(a.object.get());
						zipper.add(to_string(i) + ".png", "", Z_BEST_COMPRESSION, png->binaryData);
					}
					// else 0x01 for null because not all limbPics have 'embeded' == true and 'disabled' == false
				}

				int32_t numLimb = static_cast<int32_t>(bytestodec_be<uint32_t>(data.data()+pos));
				pos += 4;

				// Limb objects (JSON)
				for (int32_t i = 0; i < numLimb; ++i) {
					string json = (AMF3{ data.data(), pos }).exportToJSON();
					zipper.add("Limb_" + to_string(i) + ".json", "", Z_BEST_COMPRESSION, json);
				}
			} else if (fileType == "SptO") { // HFX

				minizip::Zipper zipper(name + ".zip", comment);

				// Spt
				string json = (AMF3{ data.data(), pos }).exportToJSON();
				zipper.add("Spt.json", "", Z_BEST_COMPRESSION, json);

			} else if (fileType == "BgO") { // HFX

				minizip::Zipper zipper(name + ".zip", comment);
				string json = (AMF3{ data.data(), pos }).exportToJSON();
				zipper.add("BgInfoFile.json", "", Z_BEST_COMPRESSION, json);

			} else if (fileType == "gdatO") { // HFX

				minizip::Zipper zipper(name + ".zip", comment);

				int32_t numA = static_cast<int32_t>(bytestodec_be<uint32_t>(data.data()+pos));
				pos += 4;

				for (int32_t i = 0; i < numA; ++i) {
					string json = (AMF3{ data.data(), pos }).exportToJSON();
					zipper.add("Attack_" + to_string(i) + ".json", "", Z_BEST_COMPRESSION, json);
				}

				int32_t numPt = static_cast<int32_t>(bytestodec_be<uint32_t>(data.data()+pos));
				pos += 4;

				for (int32_t i = 0; i < numPt; ++i) {
					string json = (AMF3{ data.data(), pos }).exportToJSON();
					zipper.add("PtWithName_" + to_string(i) + ".json", "", Z_BEST_COMPRESSION, json);
				}

			} else {
				printf_error(io.getText("Error for data file with ID=%zu: File type unrecognized.\n"), t->id);
				continue;
			}

		} catch (const exception &e) {
			printf_error(io.getText("Error for data file with ID=%zu: %s\n"), t->id, e.what());
			continue;
		}

		++count;

		CONTINUE_FOR_EACH_ID:;
	} // for each id
	return count;
}


/**
 * Changed 'unsaved' to true in case of success
 */
void hfw::replaceStage(const size_t id, const string &stageFileName) {

	Tag_DefineBinaryData * t;
	string name;

	if (find(stages_ids.begin(), stages_ids.end(), id) == stages_ids.end()) {
		printf_error(io.getText("No stage file with ID=%zu.\n"), id);
		return;
	}

	t = static_cast<Tag_DefineBinaryData *>(swf->getTagWithId(id));
	if (t == nullptr) {
		printf_error(io.getText("Stage file with ID=%zu not found inside the game.\n"), id);
		return;
	}

	name = t->symbolName;
	name += ".xml";

	// Get new stage file data
	vector<uint8_t> stageBuf;
	try {
		readBinaryFile(stageFileName, stageBuf);
	} catch (exception &e) {
		printf_error("%s\n", e.what());
		return;
	}

	printf_colored(rlutil::YELLOW, io.getText("Replacing '%s' with file '%s'...\n"), name.c_str(), stageFileName.c_str());

	try {
		swf->replaceBinary(stageBuf, id);
		unsaved = true;
	} catch (const swf_exception &se) {
		printf_error(se.what());
		putchar('\n');
		return;
	}
}

/**
 * Changed 'unsaved' to true in case of success
 */
void hfw::replaceImage(const size_t id, const string &imgFileName) {

	// Get name of file we're replacing
	string name;
	vector<Tag *> tv = swf->getTagsOfType(SWF::tagId("DefineBitsLossless"));
	vector<Tag *> tv1 = swf->getTagsOfType(SWF::tagId("DefineBitsLossless2"));
	tv.insert(tv.end(), tv1.begin(), tv1.end());

	for (auto &t : tv) {
		auto dbl = static_cast<Tag_DefineBitsLossless *>(t);
		if (id == dbl->id) {

			name = dbl->symbolName;
			if (name == "") {
				name = to_string(dbl->id);
			}

			name += ".png";

			break;
		}
	}

	if (name.empty()) {
		printf_error(io.getText("No image with ID=%zu.\n"), id);
		return;
	}

	// Get new image file data
	vector<uint8_t> imgBuf;
	try {
		readBinaryFile(imgFileName, imgBuf);
	} catch (exception &e) {
		printf_error("%s\n", e.what());
		return;
	}

	printf_colored(rlutil::YELLOW, io.getText("Replacing '%s' with file '%s'...\n"), name.c_str(), imgFileName.c_str());

	try {
		swf->replaceImg(imgBuf, id);
		unsaved = true;
	} catch (const swf_exception &se) {
		printf_error(se.what());
		return;
	}
}

/**
 * Changed 'unsaved' to true in case of success
 */
void hfw::replaceSound(const size_t id, const string &mp3FileName) {

	// Get name of file we're replacing
	string name;
	vector<Tag *> tv = swf->getTagsOfType(SWF::tagId("DefineSound"));

	for (auto &t : tv) {
		auto ds = static_cast<Tag_DefineSound *>(t);
		if (id == ds->id) {

			name = ds->symbolName;
			if (name == "") {
				name = to_string(ds->id);
			}

			if (ds->soundFormat != 2) {
				printf_error(io.getText("Sound with ID=%zu is not an MP3 file.\n"), id);
				return;
			}

			name += ".mp3";

			break;
		}
	}

	if (name.empty()) {
		printf_error(io.getText("No sound with ID=%zu.\n"), id);
		return;
	}

	// Get new mp3 file data
	vector<uint8_t> mp3Buf;
	try {
		readBinaryFile(mp3FileName, mp3Buf);
	} catch (exception &e) {
		printf_error("%s\n", e.what());
		return;
	}

	printf_colored(rlutil::YELLOW, io.getText("Replacing '%s' with file '%s'...\n"), name.c_str(), mp3FileName.c_str());

	try {
		swf->replaceMp3(mp3Buf, id);
		unsaved = true;
	} catch (const swf_exception &se) {
		printf_error(se.what());
		putchar('\n');
		return;
	}
}

/**
 * Changed 'unsaved' to true in case of success
 */
void hfw::replaceData(const size_t id, const string &dataFileName) {

	if (find(data_ids.begin(), data_ids.end(), id) == data_ids.end()) {
		printf_error(io.getText("No data file with ID=%zu.\n"), id);
		return;
	}

	Tag_DefineBinaryData * t = static_cast<Tag_DefineBinaryData *>(swf->getTagWithId(id));
	if (t == nullptr) {
		printf_error(io.getText("Data file with ID=%zu not found inside the game.\n"), id);
		return;
	}

	string tagName = t->symbolName;

	/*string zipFilename = getFilenameFromPath(dataFileName);

	size_t startPos = zipFilename.find(" - ") + 3;
	size_t compLen = zipFilename.find_last_of(".") - startPos;
	if (startPos == string::npos || compLen == string::npos ||
	    zipFilename.compare(0, startPos-3, to_string(id)) != 0 || // compare id
	    zipFilename.compare(startPos, compLen, tagName) != 0) // compare name
	{

		printf_error(io.getText("Zip's filename must have the format \"X - DataName\","
			" where X is the ID of the file being replaced and 'DataName' is its name.\n"));
		return;
	}*/

	// Get new data file
	try {
		//vector<uint8_t> zipBuf;
		//readBinaryFile(dataFileName, zipBuf);

		//minizip::Unzipper unzipper(zipBuf);
		minizip::Unzipper unzipper(dataFileName);
		std::vector<minizip::ZipEntry> entries = unzipper.getEntries();

		printf_colored(rlutil::YELLOW, io.getText("Replacing '%s' with contents of file '%s'...\n"),
		               tagName.c_str(), dataFileName.c_str());

		vector<uint8_t> data;

		if (endsWith(tagName, "Lmi")) {

			string fileType = "limbInfo";
			AMF0::writeStringWithLenPrefixU16(data, fileType);

			map<int, string> limbPics;
			map<int, vector<uint8_t>> pngs;
			map<int, string> limbs;

			for (auto &ze : entries) {

				// check name type
				vector<uint8_t> unzipped_entry;
				unzipper.extractEntryToMemory(ze.name, unzipped_entry);

				// extract number from filename
				int zeId = -1;
				try {
					zeId = extractIntFromStr(ze.name);
				}
				catch(invalid_argument &) {}
				catch(out_of_range &) {}

				if (zeId >= 0 && startsWith(ze.name, "LimbPic_") && endsWith(ze.name, ".json")) {
					string s = {unzipped_entry.begin(), unzipped_entry.end()};
					limbPics.emplace(zeId, s);
				} else if (zeId >= 0 && endsWith(ze.name, ".png")) {
					pngs.emplace(zeId, unzipped_entry);
				} else if (zeId >= 0 && startsWith(ze.name, "Limb_") && endsWith(ze.name, ".json")) {
					string s = {unzipped_entry.begin(), unzipped_entry.end()};
					limbs.emplace(zeId, s);
				} else {
					printf_error(io.getText("Please do not change the format of the file names. "
					            "They must start with \"LimbPic_\" or \"Limb_\" in case of "
					            ".json files and be numbered accordingly to their order.\n"));
					return;
				}
			}

			// Num of LimbPics
			array<uint8_t, 4> numLPBytes = dectobytes_be<uint32_t>(static_cast<uint32_t>(limbPics.size()));
			concatVectorWithContainer(data, numLPBytes);

			// LimbPics
			int count = 0;
			vector<bool> embeddedLPs;
			for (auto &ze : limbPics) {
				vector<uint8_t> amf0 = AMF0::fromJSON(ze.second);

				nlohmann::json obj = nlohmann::json::parse(ze.second);
				bool disabled = obj["Data.LimbPic"]["disabled"];
				bool embedded = obj["Data.LimbPic"]["embeded"];
				if (disabled == embedded) {
					/// TRANSLATORS: 'embeded' is an intentional typo
					printf_colored(rlutil::YELLOW, io.getText("WARNING: LimbPic with ID=%d has \"disabled\" and "
						"\"embeded\" with the same value. This should not happen.\n"), count);
				}
				embeddedLPs.emplace_back(embedded);

				data.emplace_back(AMF3::BYTE_ARRAY_MARKER);
				concatVectorWithContainer(data, AMF3::U29BAToVector(amf0.size()));
				concatVectorWithContainer(data, amf0);

				++count;
			}

			// PNGs
			count = 0;
			for(auto b : embeddedLPs) {
				if (b) {
					try {
						data.emplace_back(AMF3::BYTE_ARRAY_MARKER);
						concatVectorWithContainer(data, AMF3::U29BAToVector(pngs.at(count).size()));
						concatVectorWithContainer(data, pngs.at(count));
					} catch (const out_of_range& oor) {
						/// TRANSLATORS: 'embeded' is an intentional typo
						printf_colored(rlutil::YELLOW, io.getText("WARNING: LimbPic with ID=%d has \"embeded\" set "
							"to \"true\", but there is no PNG file with that ID. If you wish to disable it, "
							"change \"disabled\" to \"true\" and \"embeded\" to \"false\".\n"), count);
						data.emplace_back(0x01);
					}
				} else {
					data.emplace_back(0x01);
				}
				++count;
			}

			// Num of Limbs
			array<uint8_t, 4> numLimbs_Bytes = dectobytes_be<uint32_t>(static_cast<uint32_t>(limbs.size()));
			concatVectorWithContainer(data, numLimbs_Bytes);

			// Limbs
			for (auto &ze : limbs) {
				vector<uint8_t> amf0 = AMF0::fromJSON(ze.second);

				data.emplace_back(AMF3::BYTE_ARRAY_MARKER);
				concatVectorWithContainer(data, AMF3::U29BAToVector(amf0.size()));
				concatVectorWithContainer(data, amf0);
			}

		} else if (endsWith(tagName, "Spt")) {

			if (entries[0].name != "Spt.json") {
				printf_error(io.getText("Please do not change the format of the file name. "
				            "It should be \"Spt.json\".\n"));
				return;
			}

			string fileType = "Spt";
			AMF0::writeStringWithLenPrefixU16(data, fileType);

			vector<uint8_t> unzipped_entry;
			unzipper.extractEntryToMemory(entries[0].name, unzipped_entry);

			string s = {unzipped_entry.begin(), unzipped_entry.end()};
			vector<uint8_t> amf0 = AMF0::fromJSON(s);

			data.emplace_back(AMF3::BYTE_ARRAY_MARKER);
			concatVectorWithContainer(data, AMF3::U29BAToVector(amf0.size()));
			concatVectorWithContainer(data, amf0);

		} else if (endsWith(tagName, "Bgi")) {

			if (entries[0].name != "BgInfoFile.json") {
				printf_error(io.getText("Please do not change the format of the file name. "
				            "It should be \"BgInfoFile.json\".\n"));
				return;
			}

			string fileType = "Bg";
			AMF0::writeStringWithLenPrefixU16(data, fileType);

			vector<uint8_t> unzipped_entry;
			unzipper.extractEntryToMemory(entries[0].name, unzipped_entry);

			string s = {unzipped_entry.begin(), unzipped_entry.end()};
			vector<uint8_t> amf0 = AMF0::fromJSON(s);

			data.emplace_back(AMF3::BYTE_ARRAY_MARKER);
			concatVectorWithContainer(data, AMF3::U29BAToVector(amf0.size()));
			concatVectorWithContainer(data, amf0);

		} else if (endsWith(tagName, "Dat")) {

			string fileType = "gdat";
			AMF0::writeStringWithLenPrefixU16(data, fileType);

			map<int, string> attacks;
			map<int, string> ptwnames;

			for (auto &ze : entries) {

				// check name type
				vector<uint8_t> unzipped_entry;
				unzipper.extractEntryToMemory(ze.name, unzipped_entry);

				// extract number from filename
				int zeId = -1;
				try {
					zeId = extractIntFromStr(ze.name);
				}
				catch(invalid_argument &) {}
				catch(out_of_range &) {}

				if (zeId >= 0 && startsWith(ze.name, "Attack_") && endsWith(ze.name, ".json")) {
					string s = {unzipped_entry.begin(), unzipped_entry.end()};
					attacks.emplace(zeId, s);
				} else if (zeId >= 0 && startsWith(ze.name, "PtWithName_") && endsWith(ze.name, ".json")) {
					string s = {unzipped_entry.begin(), unzipped_entry.end()};
					ptwnames.emplace(zeId, s);
				} else {
					printf_error(io.getText("Please do not change the format of the file names. "
					            "They must start with \"Attack_\" or \"PtWithName_\" in case of "
					            ".json files and be numbered accordingly to their order.\n"));
					return;
				}
			}

			// Num of Attacks
			array<uint8_t, 4> numAttBytes = dectobytes_be<uint32_t>(static_cast<uint32_t>(attacks.size()));
			concatVectorWithContainer(data, numAttBytes);

			// Attacks
			for (auto &ze : attacks) {
				vector<uint8_t> amf0 = AMF0::fromJSON(ze.second);

				data.emplace_back(AMF3::BYTE_ARRAY_MARKER);
				concatVectorWithContainer(data, AMF3::U29BAToVector(amf0.size()));
				concatVectorWithContainer(data, amf0);
			}

			// Num of PtWithNames
			array<uint8_t, 4> numPtBytes = dectobytes_be<uint32_t>(static_cast<uint32_t>(ptwnames.size()));
			concatVectorWithContainer(data, numPtBytes);

			// PtWithNames
			for (auto &ze : ptwnames) {
				vector<uint8_t> amf0 = AMF0::fromJSON(ze.second);

				data.emplace_back(AMF3::BYTE_ARRAY_MARKER);
				concatVectorWithContainer(data, AMF3::U29BAToVector(amf0.size()));
				concatVectorWithContainer(data, amf0);
			}

		} else {
			printf_error(io.getText("Unrecognized data file extension.\n"));
			return;
		}

		//XXX Only for test
		/*
		writeBinaryFile(tagName + ".out", data);
		*/

		vector<uint8_t> compressed = zlib::zlib_compress(data, Z_BEST_COMPRESSION);

		vector<uint8_t> LMI;
		LMI.emplace_back(AMF3::BYTE_ARRAY_MARKER);
		concatVectorWithContainer(LMI, AMF3::U29BAToVector(compressed.size()));
		concatVectorWithContainer(LMI, compressed);

		try {
			swf->replaceBinary(LMI, id);
			unsaved = true;
		} catch (const swf_exception &se) {
			printf_error(se.what());
			putchar('\n');
			return;
		}

	} catch (const exception &e) {
		printf_error("%s\n", e.what());
		putchar('\n');
		return;
	}
}

/**
 * Changes 'unsaved' to false in case of success.
 */
void hfw::exportSwf() {

	printf_colored(rlutil::YELLOW, io.getText("Compression: \n"));

	string choice2;
	printf_normal(io.getText("[0] Uncompressed\n[1] *zlib (faster, bigger)\n[2] LZMA (slower, smaller)\n"));
	readLine(choice2, io.getText("Pick option (default=1): "));
	while (!(choice2 == "0" || choice2 == "1" || choice2 == "2" || choice2 == "")) {
		printf_error(io.getText("Invalid option.\n"));
		readLine(choice2);
	}
	if (choice2 == "") {
		choice2 = "1";
	}

	string outName;
	readLine(outName, io.getText("Path to output file (default=HF_out.swf): "));
	if (outName == "") {
		outName = "HF_out.swf";
	}

	printf_normal(io.getText("Generating SWF... Please wait.\n"));

	try {
		vector<uint8_t> bytes = swf->exportSwf(stoi(choice2));
		writeBinaryFile(outName, bytes);
		unsaved = false;
	} catch (exception &e) {
		printf_error("%s\n", e.what());
		putchar('\n');
	}
}

/**
 * Changes 'unsaved' to false in case of success.
 */
void hfw::exportExe() {

	printf_colored(rlutil::YELLOW, io.getText("Compression: \n"));

	string choice2;
	printf_normal(io.getText("[0] Uncompressed\n[1] *zlib (faster, bigger)\n[2] LZMA (slower, smaller)\n"));
	readLine(choice2, io.getText("Pick option (default=1): "));
	while (!(choice2 == "0" || choice2 == "1" || choice2 == "2" || choice2 == "")) {
		printf_error(io.getText("Invalid option.\n"));
		readLine(choice2);
	}
	if (choice2 == "") {
		choice2 = "1";
	}

	string outName;
	/// TRANSLATORS: Don't change default executable name
	readLine(outName, io.getText("Path to output file (default=HF_out.exe): "));
	removequotes(trim(outName));
	if (outName == "") {
		outName = "HF_out.exe";
	}

	string choice3;
	if (swf->hasProjector()) {
		/// TRANSLATORS: Don't change y/n options
		readLine(choice3, io.getText("You have already loaded a Flash Player SA into memory.\nWould you like to use it? [y/n] (default=y): "));
		while (!(choice3 == "y" || choice3 == "Y" || choice3 == "n" || choice3 == "N" || choice3 == "")) {
			printf_error(io.getText("Invalid option.\n"));
			readLine(choice3);
		}
	}

	bool windows = false;
	vector<uint8_t> proj;
	if (!swf->hasProjector() || choice3 == "n" || choice3 == "N") {
		string projectorName;
		readLine(projectorName, io.getText("Path to Adobe Flash Player Projector (default=SA.exe): "));
		removequotes(trim(projectorName));
		if (projectorName == "") {
			projectorName = "SA.exe";
		}
		// Check if Projector file exists
		try {
			readBinaryFile(projectorName, proj);
		} catch (exception &e) {
			printf_error("%s\n", e.what());
			return;
		}

		windows = isPEfile(proj);
	} else {
		windows = swf->isProjectorWindows();
	}

	/// TRANSLATORS: Generating EXE / ELF, leave %s as it is.
	printf_normal(io.getText("Generating %s... Please wait.\n"), (windows ? "EXE" : "ELF"));

	try {
		vector<uint8_t> bytes = swf->exportExe(proj, stoi(choice2));
		writeBinaryFile(outName, bytes);
		unsaved = false;
	} catch (exception &e) {
		printf_error("%s\n", e.what());
		putchar('\n');
	}
}

void hfw::printHeader() {
	print2Colors(io.getText(
	                     R"(  _   _ _____  __        _____  ____  _  ______  _   _  ___  ____  )""\n"
	                     R"( | | | |  ___| \ \      / / _ \|  _ \| |/ / ___|| | | |/ _ \|  _ \ )""\n"
	                     R"( | |_| | |_     \ \ /\ / / | | | |_) | ' /\___ \| |_| | | | | |_) |)""\n"
	                     R"( |  _  |  _|     \ V  V /| |_| |  _ <| . \ ___) |  _  | |_| |  __/ )""\n"
	                     R"( |_| |_|_|        \_/\_/  \___/|_| \_\_|\_\____/|_| |_|\___/|_|    )""\n\n"),
	                     15, rlutil::LIGHTRED, rlutil::YELLOW);

	printf_colored(rlutil::YELLOW, io.getText("Hello there, hero!\n"
		"Which file do you wish to modify?\n\n"));
}

void hfw::printHelp() {
	putchar('\n');
	printf_colored(rlutil::YELLOW, io.getText(
	                                       R"(  _  _ ___ _    ___ )""\n"
		                                   R"( | || | __| |  | _ \)""\n"
		                                   R"( | __ | _|| |__|  _/)""\n"
		                                   R"( |_||_|___|____|_|  )""\n\n"));

	/// TRANSLATORS: Help section
	printf_colored(rlutil::LIGHTRED, io.getText("IMPORTANT: "));
	/// TRANSLATORS: Help section
	printf_normal(io.getText("Hero Fighter is property of Marti Wong. This game has suffered a lot at the hands of crackers, "
		"who sought to unlock all the characters in the game for free, thus making the game unsustainable. "
		"HF Workshop is meant to modify Hero Fighter in a ETHICAL manner in order to bring new wonders to the game. "
		"If you wish to see new Hero Fighter versions released in the future, by no means you'll attempt to modify this game "
		"in a way that hurts Marti's income or the game's reputation and its users. Keep in mind that sounds and images "
		"that you find on the internet might be copyright protected in which case you should"));
	/// TRANSLATORS: Help section
	printf_colored(rlutil::LIGHTRED, io.getText(" NOT "));
	/// TRANSLATORS: Help section
	printf_normal(io.getText("use them. Thank you.\n\n"));

	/// TRANSLATORS: Help section
	printf_normal(io.getText("This program allows you to list, export and replace images, sounds and stages. It is recommended that "
		"you keep the "));
	/// TRANSLATORS: Help section
	printf_colored(rlutil::LIGHTRED, io.getText("images resized and sounds compressed"));
	/// TRANSLATORS: Help section
	printf_normal(io.getText(" as much as possible so that the final file will not be too large.\n\n"));

	/// TRANSLATORS: Help section
	printf_colored(rlutil::LIGHTRED, io.getText("Exporting HF: "));
	/// TRANSLATORS: Help section
	printf_normal(io.getText("You may export the modified Hero Fighter in SWF or EXE formats. When exporting, you'll be given the option "
		"to compress the SWF file using Zlib or LZMA algorithms. It is highly recommended that you use one of these "
		"because a smaller file size will be easier to transfer on the internet. Zlib compression is the most common, whilst "
		"LZMA is more recent and won't work on Flash Player versions below 11.\n\n"));

	/// TRANSLATORS: Help section
	printf_normal(io.getText("If you're using a Flash Player projector "
	     "(i.e. an EXE file) it is recommended that you compress it using UPX.\n\n"));

	/// TRANSLATORS: Help section
	printf_colored(rlutil::LIGHTRED, io.getText("Replacing Images: "));
	/// TRANSLATORS: Help section
	printf_normal(io.getText("Only PNG files are supported. It is recommended that you resize and compress your PNG files. "
		"Note that images get slightly changed during the process of extracting and replacing due to premultiplied "
		"alpha.\n\n"));

	/// TRANSLATORS: Help section
	printf_colored(rlutil::LIGHTRED, io.getText("Replacing MP3: "));
	/// TRANSLATORS: Help section
	printf_normal(io.getText("Only MP3 files with "));
	/// TRANSLATORS: Help section
	printf_colored(rlutil::LIGHTRED, io.getText("sample rates"));
	/// TRANSLATORS: Help section
	printf_normal(io.getText(" of 5512 Hz, 11025 Hz, 22050 Hz and 44100 Hz are allowed (note: sample rates and bit rates are not "
		 "the same thing). This program will tell you if the MP3 file that you're trying to replace with doesn't "
		 "meet the requirements. In this case, you may find a converter online for your MP3 file. It is also "
		 "recommended that you find an MP3 compressor to reduce the size of your MP3 files.\n\n"));

	/// TRANSLATORS: Help section
	printf_normal(io.getText("For tutorials and discussion visit "));
	printf_colored(rlutil::YELLOW, "https://hf-empire.com/forum\n\n");
}

void hfw::printOptions(const vector<string> &options) {
	printf_colored(rlutil::YELLOW, io.getText("What do you wish to do?\n"));
	for (size_t i = 0; i < options.size(); ++i) {
		printf_colored(rlutil::LIGHTRED, (to_string((i == options.size() - 1 ? 0 : i + 1)) + ". ").c_str());
		printf_normal(options[i].c_str());
		putchar('\n');
	}
}
