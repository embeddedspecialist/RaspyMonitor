#ifndef VERSION_H
#define VERSION_H

namespace AutoVersion{
	
	//Date Version Types
	static const char DATE[] = "29";
	static const char MONTH[] = "05";
	static const char YEAR[] = "2014";
	static const double UBUNTU_VERSION_STYLE = 8.10;
	
	//Software Status
	static const char STATUS[] = "Release";
	static const char STATUS_SHORT[] = "r";
	
	//Standard Version Type
	static const long MAJOR = 1;
	static const long MINOR = 0;
	static const long BUILD = 17;
	static const long REVISION = 121;
	
	//Miscellaneous Version Types
	static const long BUILDS_COUNT = 69;
	#define RC_FILEVERSION 1,0,17,121
	#define RC_FILEVERSION_STRING "1, 0, 17, 121\0"

	static const char FULLVERSION_STRING[] = "1.7.0";
	
	//These values are to keep track of your versioning state, don't modify them.
	static const long BUILD_HISTORY = 17;
	

}
#endif //VERSION_h
