/**
 * C++ implementation of necessary parts from:
 * https://android.googlesource.com/platform/tools/apksig
 */


#include "apksigner.hpp"
#include "AndroidBinXmlParser.hpp"
#include "ZipSections.hpp"
#include "ZipUtils.hpp"
#include "../utils.hpp"       // int_to_hex

#include <string>
#include <string_view>
#include <vector>
#include <array>
#include <algorithm>       // std::max, std::find_if
#include <utility>         // std::pair

#ifdef APK_DEBUG_BUILD
#include <iostream>
#define APK_DEBUG(x) do { std::cout << x << std::endl; } while (0)
#define APK_DEBUG_NNL(x) do { std::cout << x; } while (0)
#else
#define APK_DEBUG(x) do { } while (0)
#define APK_DEBUG_NNL(x) do { } while (0)
#endif

using namespace apksigner;

namespace {

	class ApkUtils {

	public:

		constexpr static std::string_view ANDROID_MANIFEST_ZIP_ENTRY_NAME = "AndroidManifest.xml";

		/**
		 * Returns the API Level corresponding to the provided platform codename.
		 *
		 * This method is pessimistic. It returns a value one lower than the API Level with which the
		 * platform is actually released (e.g., 23 for N which was released as API Level 24). This is
		 * because new features which first appear in an API Level are not available in the early days
		 * of that platform version's existence, when the platform only has a codename. Moreover, this
		 * method currently doesn't differentiate between initial and MR releases, meaning API Level
		 * returned for MR releases may be more than one lower than the API Level with which the
		 * platform version is actually released.
		 */
		static int getMinSdkVersionForCodename(std::string &codename) {

			char firstChar = codename.empty() ? ' ' : codename.at(0);
			// Codenames are case-sensitive. Only codenames starting with A-Z are supported for now.
			// We only look at the first letter of the codename as this is the most important letter.
			if ((firstChar >= 'A') && (firstChar <= 'Z')) {
				using CodeLevelPair = std::pair<char, int>;
				std::array<CodeLevelPair, 13> sortedCodenamesFirstCharToApiLevel = {{
					{'C', 2}, {'D', 3}, {'E', 4}, {'F', 7},
					{'G', 8}, {'H', 10}, {'I', 13}, {'J', 15},
					{'K', 18}, {'L', 20}, {'M', 22}, {'N', 23}, {'O', 25}
				}};

				if (firstChar < 'C') {
					// 'A' or 'B' -- never released to public
					return 1;
				}

				auto it = std::find_if (sortedCodenamesFirstCharToApiLevel.begin(),
				                        sortedCodenamesFirstCharToApiLevel.end(),
				                        [&](CodeLevelPair p){ return p.first == firstChar; });

				if (it != sortedCodenamesFirstCharToApiLevel.end()) {
					// Exact match
					return it->second;
				} else {
					// Find the newest older codename.
					// API Level bumped by at least 1 for every change in the first letter of codename
					auto it2 = std::find_if (sortedCodenamesFirstCharToApiLevel.begin(),
					                         sortedCodenamesFirstCharToApiLevel.end(),
					                         [&](CodeLevelPair p){ return p.first > firstChar; });

					char newestOlderCodenameFirstChar = it2->first;
					int newestOlderCodenameApiLevel = it2->second;
					return newestOlderCodenameApiLevel + static_cast<int>(firstChar - newestOlderCodenameFirstChar);
				}
			}

			throw min_sdk_version_exception(std::string("Unable to determine APK's minimum supported Android platform version")
			                                + " : Unsupported codename in " + std::string(ANDROID_MANIFEST_ZIP_ENTRY_NAME)
			                                + "'s minSdkVersion: \"" + codename + "\"");
		}


		/**
		 * Returns the lowest Android platform version (API Level) supported by an APK with the
		 * provided {@code AndroidManifest.xml}.
		 */
		static int getMinSdkVersionFromBinaryAndroidManifest(std::vector<uint8_t> &androidManifestContents) {
			// IMPLEMENTATION NOTE: Minimum supported Android platform version number is declared using
			// uses-sdk elements which are children of the top-level manifest element. uses-sdk element
			// declares the minimum supported platform version using the android:minSdkVersion attribute
			// whose default value is 1.
			// For each encountered uses-sdk element, the Android runtime checks that its minSdkVersion
			// is not higher than the runtime's API Level and rejects APKs if it is higher. Thus, the
			// effective minSdkVersion value is the maximum over the encountered minSdkVersion values.

			// If no uses-sdk elements are encountered, Android accepts the APK. We treat this
			// scenario as though the minimum supported API Level is 1.
			int result = 1;

			ByteBuffer bb{androidManifestContents};
			AndroidBinXmlParser parser(bb);
			int eventType = parser.getEventType();

			while (eventType != AndroidBinXmlParser::EVENT_END_DOCUMENT) {
				if ((eventType == AndroidBinXmlParser::EVENT_START_ELEMENT)
					&& (parser.getDepth() == 2)
					&& ("uses-sdk" == parser.getName())
					&& (parser.getNamespace().empty())) {

					// In each uses-sdk element, minSdkVersion defaults to 1
					int minSdkVersion = 1;
					for (size_t i = 0; i < parser.getAttributeCount(); ++i) {
						if (parser.getAttributeNameResourceId(i) == MIN_SDK_VERSION_ATTR_ID) {
							int valueType = parser.getAttributeValueType(i);
							switch (valueType) {
							case AndroidBinXmlParser::VALUE_TYPE_INT:
								minSdkVersion = parser.getAttributeIntValue(i);
								break;
							case AndroidBinXmlParser::VALUE_TYPE_STRING: {
								auto codename = parser.getAttributeStringValue(i);
								minSdkVersion = getMinSdkVersionForCodename(codename);
								break;
							}
							default:
								throw min_sdk_version_exception(std::string("Unable to determine APK's minimum supported Android")
								                                + ": unsupported value type in "
								                                + std::string(ANDROID_MANIFEST_ZIP_ENTRY_NAME) + "'s"
								                                + " minSdkVersion"
								                                + ". Only integer values supported.");
							}
						}
					}
					result = std::max(result, minSdkVersion);
				}
				eventType = parser.next();
			}

			return result;

		}

	private:
		constexpr static int MIN_SDK_VERSION_ATTR_ID = 0x0101020c;

	};

}

void apksigner::sign(minizip::Unzipper &unzipper, minizip::Zipper &zipper) {
	
	// Step 1. Find input APK's main ZIP sections
	try {
		//ZipSections inputZipSections = ApkUtils.findZipSections(unzipper); // TODO
	} catch (const zip_format_exception& e) {
		throw apk_format_exception(std::string("Malformed APK: not a ZIP archive.\n") + e.what());
	}

	std::vector<uint8_t> androidManifest;
	unzipper.extractEntryToMemory(std::string(ApkUtils::ANDROID_MANIFEST_ZIP_ENTRY_NAME), androidManifest);

	int minSdk = ApkUtils::getMinSdkVersionFromBinaryAndroidManifest(androidManifest);

	APK_DEBUG("Min SDK is: " + std::to_string(minSdk));
}

