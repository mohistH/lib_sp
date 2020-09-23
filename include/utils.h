#pragma once
#include "universe_def.h"
#include "config.h"

#include <string>
#include <sstream>
#include <string.h>
#include <memory>

#ifdef used_in_mfc
// MFC CString
	#include <afxstr.h>
#endif ///!used_in_mfc


// to definite an export flag
#if defined(_WIN32) || defined(_WIN64) || defined(WIN32)
//---------------------------------------------------------------------------------------------
	#ifndef _utils_api_
		#define _utils_api_	__declspec(dllexport)
	#else
		#define _utils_api_	__declspec(dllimport)
	#endif /// !_utils_api_

#elif defined(_unix_) || defined(_linux_) || defined(_unix) || defined(_linux) || defined(__unix)
//---------------------------------------------------------------------------------------------
	#ifndef _utils_api_
		#define _utils_api_	__attribute__((visibility ("default")))
	#endif /// !_utils_api_

#endif /// !defined(_WIN32) || defined(_WIN64) || defined(WIN32)




namespace utils
{

//----------------------------------------------------------------------------------------


	/**
	* @brief: its a convenient class
	*/
	class _utils_api_ helper
	{
	public:

	//----------------------------------------------------------------------------------------		
		
		/**
		*  @ brief: std::wstring -> std::string
		*/
		static std::string wstr2str(const std::wstring& wstr);
		/**
		*  @ brief: std::string -> std::wstring
		*/
		static std::wstring str2wstr(const std::string& str);
	//----------------------------------------------------------------------------------------

#ifdef used_in_mfc
	//----------------------------------------------------------------------------------------
		/** 
		*  @ brief: CString -> long 
		*/
		static long cs_to_l(const CString &str, const int base = 10);

		/**
		* @brief: CString -> double
		*/
		static double cs_to_d(const CString& str);

		/**
		* @brief: CString -> char*
		*/
		static char * cs_to_pchar(const CString str);

		/** 
		*  @ brief: long??int??BYTE -> CString
		*/
		template<typename T>
		static CString l_to_cs(const T& val, const int to_base = 10)
		{
			CString str;

			switch (base)
			{
				// ??16??????????
			case 16:
				str.Format(L"%X", val);
				break;

				// to base 10
			case 10:
				str.Format(L"%d", val);
			default:
				break;
			}

			return str;
		}

		/** 
		*  @ brief: float??double -> CString
		*  @ precision - 0-%.0f, 1-%.1f, 2-%.2f....
		*/
		template <typename T>
		static CString fd_to_cs(const T& val, unsigned int precision)
		{
			CString str;
			CString str_format;
			str_format = CString(L"%.") + str_format.Format(L"%d", precision) + CString(L"f");
			
			str.Format(str_format, val);

			return str;
		}

		/**
		* @brief: char * array -> CString
		*/
		static CString pchar_to_cs(const char* parr);
#endif /// used_in_mfc
	//----------------------------------------------------------------------------------------

		/** 
		*  @ brief: std::string -> long
		*/
		static long  s_to_l(const std::string& str, const int base = 10);

		/**
		* @brief: std::string -> double(float)
		*/
		static double s_to_fd(const std::string& str);
		/**
		* @brief: std::string -> char*
		*/
		static char * s_to_pchar(const std::string& str);

		/**
		* @brief: (int??double??float??long??short) -> std::string
		*/
		template<typename T>
		static std::string val_to_s(const T& val)
		{
			std::string str;
			std::stringstream  ss;
			ss << val;
			ss >> str;
			return str;
		}

//----------------------------------------------------------------------------------------
#ifdef has_cxx_11
		/** 
		*  @ brief: other type -> std::string
					other type??
					int??double??long??long long?? long double
					unsigned long, unsigned long long??short??WORD??DWROD
		*/
		template<typename T>
		static std::string to_str(const T& val)
		{
			return std::to_string(val);
		}

		/**
		* @brief:  std::string -> int
		* @base - what is the str's base 
		*/
		static int xxs_to_i(const std::string& str, const int base = 10);

		/**
		* @brief:  std::string -> float
		*/
		static float xxs_to_f(const std::string& str);

		/**
		* @brief: std::string -> double
		*/
		static double xxs_to_d(const std::string& str);

		/** 
		*  @ brief: std::string -> long
		*  @base - what is the str's base
		*/
		static long xxs_to_l(const std::string& str, const int base = 10);

		/** 
		*  @ brief: std::string -> unsigned long	
		*  @ base - what is the str's base
		*/
		static unsigned long xxs_to_ul(const std::string& str, const int base = 10);

		/**
		* @brief: std::string -> long long
		*  @ base - what is the str's base
		*/
		static long long xxs_to_ll(const std::string& str, const int base = 10);

		/**
		* @brief: std::string -> unsigned long long
		*  @ base - what is the str's base
		*/
		static unsigned long long xxs_to_ull(const std::string& str, const int base = 10);

		/**
		* @brief: std::string ->  long double
		*/
		static long double xxs_to_ld(const std::string& str);


		/**
		* @brief: std::string format
		*/
		template<typename ... Args>
		static std::string str_format(const std::string &format, Args ... args)
		{
			auto size_buf = std::snprintf(nullptr, 0, format.c_str(), args ...) + 1; 
			std::unique_ptr<char[]> buf(new(std::nothrow) char[size_buf]);

			if (!buf)
				return std::string("");

			std::snprintf(buf.get(), size_buf, format.c_str(), args ...);

			return std::string(buf.get(), buf.get() + size_buf - 1); 
		}


#endif /// has_cxx_11
//----------------------------------------------------------------------------------------
		/**
		* @brief: to get current work directory
		*/
		static std::string get_cwd() ;
	};



} /// utils




