#ifndef __VERSION_HEADER__
#define __VERSION_HEADER__

#define INT_TO_STR(x) #x

#define MAJOR_VERSION_INT PLUGIN_MAJOR_VERSION
#define MAJOR_VERSION_STR INT_TO_STR(MAJOR_VERSION_INT)

#define SUB_VERSION_INT PLUGIN_MINOR_VERSION
#define SUB_VERSION_STR INT_TO_STR(SUB_VERSION_INT)

#define RELEASE_NUMBER_INT PLUGIN_RELEASE_NUMBER
#define RELEASE_NUMBER_STR INT_TO_STR(RELEASE_NUMBER_INT)

#define BUILD_NUMBER_INT PLUGIN_BUILD_NUMBER
#define BUILD_NUMBER_STR INT_TO_STR(PLUGIN_BUILD_NUMBER)

// Version with build number (example "1.0.3.342")
#define FULL_VERSION_STR MAJOR_VERSION_STR "." SUB_VERSION_STR "." RELEASE_NUMBER_STR "." BUILD_NUMBER_STR

// Version without build number (example "1.0.3")
#define VERSION_STR MAJOR_VERSION_STR "." SUB_VERSION_STR "." RELEASE_NUMBER_STR

#define stringOriginalFilename  "regrader.vst3"
#define stringFileDescription   "Regrader plugin"
#define stringCompanyName       "igorski.nl\0"
#define stringLegalCopyright    "igorski.nl 2018-2020"
#define stringLegalTrademarks   "VST is a trademark of Steinberg Media Technologies GmbH"

#endif
