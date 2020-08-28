#ifndef __VERSION_HEADER__
#define __VERSION_HEADER__

#define MAJOR_VERSION_STR "1"
#define MAJOR_VERSION_INT 1

#define SUB_VERSION_STR "0"
#define SUB_VERSION_INT 0

#define RELEASE_NUMBER_STR "0"
#define RELEASE_NUMBER_INT 1

#define BUILD_NUMBER_STR "4"
#define BUILD_NUMBER_INT 4

// Version with build number (example "1.0.3.342")
#define FULL_VERSION_STR MAJOR_VERSION_STR "." SUB_VERSION_STR "." RELEASE_NUMBER_STR "." BUILD_NUMBER_STR

// Version without build number (example "1.0.3")
#define VERSION_STR MAJOR_VERSION_STR "." SUB_VERSION_STR "." RELEASE_NUMBER_STR

#define stringOriginalFilename  "regrader.vst3"
#if PLATFORM_64
#define stringFileDescription   "Regrader plugin (64Bit)"
#else
#define stringFileDescription   "Regrader plugin"
#endif
#define stringCompanyName       "igorski.nl\0"
#define stringLegalCopyright    "� 2018 igorski.nl"
#define stringLegalTrademarks   "VST is a trademark of Steinberg Media Technologies GmbH"

#endif
