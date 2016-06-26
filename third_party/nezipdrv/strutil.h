#pragma once
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <functional>
#include <stdarg.h>

typedef std::vector<std::string> StringVector;

class StrUtil
{
public:

	

	/** Removes any whitespace characters, be it standard space or
	TABs and so on.
	@remarks
	The user may specify wether they want to trim only the
	beginning or the end of the std::string ( the default action is
	to trim both).
	*/
	static void trim( std::string& str, bool left = true, bool right = true )
	{
		static const std::string delims = " \t\r";
		if(right)
			str.erase(str.find_last_not_of(delims)+1);
		if(left)
			str.erase(0, str.find_first_not_of(delims));
	}

    //去掉所有空格=
	static void trimAllSpace(std::string &str)
	{
		std::string::iterator destEnd=std::remove_if(str.begin(),str.end(),isspace);
		str.resize(destEnd-str.begin());
	}

    //去除所有特定字符=
	static void trimAll(std::string &str,char ch)
	{
		std::string::iterator destEnd=std::remove_if(str.begin(),str.end(),std::bind1st(std::equal_to<char>(),ch));
		str.resize(destEnd-str.begin());
	}

	/** Returns a std::stringVector that contains all the substd::strings delimited
	by the characters in the passed <code>delims</code> argument.
	@param
	delims A list of delimiter characters to split by
	@param
	maxSplits The maximum number of splits to perform (0 for unlimited splits). If this
	parameters is > 0, the splitting process will stop after this many splits, left to right.
	*/
	static StringVector split( const std::string& str, const std::string& delims = "\t\n ", unsigned int maxSplits = 0)
	{
		StringVector ret;
		unsigned int numSplits = 0;

		// Use STL methods
		size_t start, pos;
		start = 0;
		do
		{
			pos = str.find_first_of(delims, start);
			if (pos == start)
			{
				// Do nothing
				start = pos + 1;
			}
			else if (pos == std::string::npos || (maxSplits && numSplits == maxSplits))
			{
				// Copy the rest of the std::string
				ret.push_back( str.substr(start) );
				break;
			}
			else
			{
				// Copy up to delimiter
				ret.push_back( str.substr(start, pos - start) );
				start = pos + 1;
			}
			// parse up to next real data
			start = str.find_first_not_of(delims, start);
			++numSplits;

		} while (pos != std::string::npos);
		return ret;
	}

	/** Upper-cases all the characters in the std::string.
	*/
	static void toLowerCase( std::string& str )
	{
		std::transform(
			str.begin(),
			str.end(),
			str.begin(),
			tolower);
	}

	/** Lower-cases all the characters in the std::string.
	*/
	static void toUpperCase( std::string& str )
	{
		std::transform(
			str.begin(),
			str.end(),
			str.begin(),
			toupper);
	}

	/** Converts the contents of the std::string to a float.
	@remarks
	Assumes the only contents of the std::string are a valid parsable float. Defaults to  a
	value of 0.0 if conversion is not possible.
	*/
	static float toFloat( const std::string& str )
	{
		return (float)atof(str.c_str());
	}

	static double toDouble( const std::string& str )
	{
		return atof(str.c_str());
	}

	/** Returns whether the std::string begins with the pattern passed in.
	@param pattern The pattern to compare with.
	@param lowerCase If true, the end of the std::string will be lower cased before
	comparison, pattern should also be in lower case.
	*/
	static bool startsWith(const std::string& str, const std::string& pattern, bool lowerCase = true)
	{
		size_t thisLen = str.length();
		size_t patternLen = pattern.length();
		if (thisLen < patternLen || patternLen == 0)
			return false;

		std::string startOfThis = str.substr(0, patternLen);
		if (lowerCase)
			toLowerCase(startOfThis);

		return (startOfThis == pattern);
	}

	/** Returns whether the std::string ends with the pattern passed in.
	@param pattern The pattern to compare with.
	@param lowerCase If true, the end of the std::string will be lower cased before
	comparison, pattern should also be in lower case.
	*/
	static bool endsWith(const std::string& str, const std::string& pattern, bool lowerCase = true)
	{
		size_t thisLen = str.length();
		size_t patternLen = pattern.length();
		if (thisLen < patternLen || patternLen == 0)
			return false;

		std::string endOfThis = str.substr(thisLen - patternLen, patternLen);
		if (lowerCase)
			toLowerCase(endOfThis);

		return (endOfThis == pattern);
	}

	/** Method for standardising paths - use forward slashes only, end with slash.
	*/
	static std::string standardisePath( const std::string &init)
	{
		std::string path = init;

		std::replace( path.begin(), path.end(), '\\', '/' );
		if( path[path.length() - 1] != '/' )
			path += '/';

		return path;
	}

	/** Method for splitting a fully qualified filename into the base name
	and path.
	@remarks
	Path is standardised as in standardisePath
	*/
	static void splitFilename(const std::string& qualifiedName,std::string& outBasename, std::string& outPath)
	{
		std::string path = qualifiedName;
		// Replace \ with / first
		std::replace( path.begin(), path.end(), '\\', '/' );
		// split based on final /
		size_t i = path.find_last_of('/');

		if (i == std::string::npos)
		{
			outPath = "";
			outBasename = qualifiedName;
		}
		else
		{
			outBasename = path.substr(i+1, path.size() - i - 1);
			outPath = path.substr(0, i+1);
		}
	}

	/** Simple pattern-matching routine allowing a wildcard pattern.
	@param str std::string to test
	@param pattern Pattern to match against; can include simple '*' wildcards
	@param caseSensitive Whether the match is case sensitive or not
	*/
	static bool match(const std::string& str, const std::string& pattern, bool caseSensitive = true)
	{
		std::string tmpStr = str;
		std::string tmpPattern = pattern;
		if (!caseSensitive)
		{
			toLowerCase(tmpStr);
			toLowerCase(tmpPattern);
		}

		std::string::const_iterator strIt = tmpStr.begin();
		std::string::const_iterator patIt = tmpPattern.begin();
		std::string::const_iterator lastWildCardIt = tmpPattern.end();
		while (strIt != tmpStr.end() && patIt != tmpPattern.end())
		{
			if (*patIt == '*')
			{
				lastWildCardIt = patIt;
				// Skip over looking for next character
				++patIt;
				if (patIt == tmpPattern.end())
				{
					// Skip right to the end since * matches the entire rest of the string
					strIt = tmpStr.end();
				}
				else
				{
					// scan until we find next pattern character
					while(strIt != tmpStr.end() && *strIt != *patIt)
						++strIt;
				}
			}
			else
			{
				if (*patIt != *strIt)
				{
					if (lastWildCardIt != tmpPattern.end())
					{
						// The last wildcard can match this incorrect sequence
						// rewind pattern to wildcard and keep searching
						patIt = lastWildCardIt;
						lastWildCardIt = tmpPattern.end();
					}
					else
					{
						// no wildwards left
						return false;
					}
				}
				else
				{
					++patIt;
					++strIt;
				}
			}

		}
		// If we reached the end of both the pattern and the string, we succeeded
		if (patIt == tmpPattern.end() && strIt == tmpStr.end())
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	/// Constant blank std::string, useful for returning by ref where local does not exist
	static const std::string BLANK()
	{
		static const std::string temp = std::string("");
		return temp;
	}

	//地球人都知道，恶心的std::string是没有CString的Format这个函数的，所以我们自己造
	static std::string printf(const char *pszFormat, ...)
	{
		va_list argptr;
		va_start(argptr, pszFormat);
		std::string result=printf(pszFormat,argptr);
		va_end(argptr);
		return result;
	}

	//地球人都知道，恶心的std::string是没有CString的Format这个函数的，所以我们自己造
	static std::string printf2(const char *pszFormat, ...)
	{
		va_list argptr;
		va_start(argptr, pszFormat);
		std::string result=printf2(pszFormat,argptr);
		va_end(argptr);
		return result;
	}

	//地球人都知道，恶心的std::string是没有CString的Format这个函数的，所以我们自己造
	static std::string printf2(const char *pszFormat,va_list argptr)
	{
		int         size   = 1024;
		char*       buffer = new char[size];

		while (1)
		{
			int n = _vsnprintf(buffer, size, pszFormat, argptr);

			// If that worked, return a string.
			if (n > -1 && n < size)
			{
				std::string s(buffer);
				delete [] buffer;
				return s;
			}

			if (n > -1)     size  = n+1; // ISO/IEC 9899:1999
			else            size *= 2;   // twice the old size

			delete [] buffer;
			buffer = new char[size];
		}
	}

	//地球人都知道，恶心的std::string是没有CString的Format这个函数的，所以我们自己造
	static std::string printf(const char* pszFormat, va_list argptr)
	{
		int size = 1024;
		int len=0;
		std::string ret;
		for ( ;; )
		{
			ret.resize(size + 1,0);
			char *buf=(char *)ret.c_str();   
			if ( !buf )
			{
				return BLANK();
			}

			va_list argptrcopy;
			VaCopy(argptrcopy, argptr);

			len = _vsnprintf(buf, size, pszFormat, argptrcopy);
			va_end(argptrcopy);

			if ( len >= 0 && len <= size )
			{
				// ok, there was enough space
				break;
			}
			size *= 2;
		}
		ret.resize(len);
		return ret;
	}

	//取得右边的N个字符
	static std::string right(const std::string &src,size_t nCount)
	{
		if(nCount>src.length())
			return BLANK();
		return src.substr(src.length()-nCount,nCount);
	}

	//取左边的N个字符
	static std::string left(const std::string &src,size_t nCount)
	{
		return src.substr(0,nCount);
	}

	size_t charCount(const std::string &src,char ch)
	{
		size_t result=0;
		for(size_t i=0;i<src.length();i++)
		{
			if(src[i]==ch)result++;
		}
		return result;
	}

protected:
	static inline void VaCopy(va_list &dest,va_list &src)
	{
		dest=src;
	}
};


class StrConvert
{
public:
	/** Converts a float to a String. */
	static std::string toString(float val, unsigned short precision = 6,
		unsigned short width = 0, char fill = ' ',
		std::ios::fmtflags flags = std::ios::fmtflags(0) )
	{
		std::stringstream stream;
		stream.precision(precision);
		stream.width(width);
		stream.fill(fill);
		if (flags)
			stream.setf(flags);
		stream << val;
		return stream.str();
	}

	/** Converts a double to a String. */
	static std::string toString(double val, unsigned short precision = 6,
		unsigned short width = 0, char fill = ' ',
		std::ios::fmtflags flags = std::ios::fmtflags(0) )
	{
		std::stringstream stream;
		stream.precision(precision);
		stream.width(width);
		stream.fill(fill);
		if (flags)
			stream.setf(flags);
		stream << val;
		return stream.str();
	}

	/** Converts an int to a String. */
	static std::string toString(int val, unsigned short width = 0,
		char fill = ' ',
		std::ios::fmtflags flags = std::ios::fmtflags(0) )
	{
		std::stringstream stream;
		stream.width(width);
		stream.fill(fill);
		if (flags)
			stream.setf(flags);
		stream << val;
		return stream.str();
	}

	/** Converts a size_t to a String. */
	static std::string toString(size_t val,
		unsigned short width = 0, char fill = ' ',
		std::ios::fmtflags flags = std::ios::fmtflags(0) )
	{
		std::stringstream stream;
		stream.width(width);
		stream.fill(fill);
		if (flags)
			stream.setf(flags);
		stream << val;
		return stream.str();
	}

	/** Converts an unsigned long to a String. */
	static std::string toString(unsigned long val,
		unsigned short width = 0, char fill = ' ',
		std::ios::fmtflags flags = std::ios::fmtflags(0) )
	{
		std::stringstream stream;
		stream.width(width);
		stream.fill(fill);
		if (flags)
			stream.setf(flags);
		stream << val;
		return stream.str();
	}

	/** Converts a long to a String. */
	static std::string toString(long val,
		unsigned short width = 0, char fill = ' ',
		std::ios::fmtflags flags = std::ios::fmtflags(0) )
	{
		std::stringstream stream;
		stream.width(width);
		stream.fill(fill);
		if (flags)
			stream.setf(flags);
		stream << val;
		return stream.str();
	}
	/** Converts a boolean to a String.
	@param yesNo If set to true, result is 'yes' or 'no' instead of 'true' or 'false'
	*/
	static std::string toString(bool val, bool yesNo = false)
	{
		if (val)
		{
			if (yesNo)
			{
				return "yes";
			}
			else
			{
				return "true";
			}
		}
		else
		{
			if (yesNo)
			{
				return "no";
			}
			else
			{
				return "false";
			}
		}
	}

	/** Converts a StringVector to a string.
	@remarks
	Strings must not contain spaces since space is used as a delimeter in
	the output.
	*/
	static std::string toString(const StringVector& val)
	{
		std::stringstream stream;
		StringVector::const_iterator i, iend, ibegin;
		ibegin = val.begin();
		iend = val.end();
		for (i = ibegin; i != iend; ++i)
		{
			if (i != ibegin)
				stream << " ";

			stream << *i;
		}
		return stream.str();
	}

	/** Converts a std::string to a float.
	@returns
	0.0 if the value could not be parsed, otherwise the float version of the String.
	*/
	static float parsefloat(const std::string& val)
	{
		return (float)atof(val.c_str());
	}

	/** Converts a std::string to a double.
	@returns
	0.0 if the value could not be parsed, otherwise the float version of the String.
	*/
	static double parseDouble(const std::string& val)
	{
		return atof(val.c_str());
	}

	/** Converts a std::string to a whole number.
	@returns
	0.0 if the value could not be parsed, otherwise the numeric version of the String.
	*/
	static int parseInt(const std::string& val)
	{
		return atoi(val.c_str());
	}

	/** Converts a std::string to a whole number.
	@returns
	0.0 if the value could not be parsed, otherwise the numeric version of the String.
	*/
	static unsigned int parseUnsignedInt(const std::string& val)
	{
		return static_cast<unsigned int>(strtoul(val.c_str(), 0, 10));
	}

	/** Converts a std::string to a whole number.
	@returns
	0.0 if the value could not be parsed, otherwise the numeric version of the String.
	*/
	static long parseLong(const std::string& val)
	{
		return strtol(val.c_str(), 0, 10);
	}

	/** Converts a std::string to a whole number.
	@returns
	0.0 if the value could not be parsed, otherwise the numeric version of the String.
	*/
	static unsigned long parseUnsignedLong(const std::string& val)
	{
		return strtoul(val.c_str(), 0, 10);
	}

	/** Converts a std::string to a boolean.
	@remarks
	Accepts 'true' or 'false' as input.
	*/
	static bool parseBool(const std::string& val)
	{
		return (val == "true" || val == "yes");
	}


	/** Pareses a StringVector from a string.
	@remarks
	Strings must not contain spaces since space is used as a delimeter in
	the output.
	*/
	static StringVector parseStringVector(const std::string& val)
	{
		return StrUtil::split(val);
	}
};
