#include "utils.h"

#ifdef os_is_win
#include <stringapiset.h>
#include <tchar.h>
#endif //!os_is_win

namespace utils
{

	/**
	*	@brief:
	*/
	std::string helper::wstr2str(const std::wstring& wstr)
	{
		if (wstr.empty())
		{
			return std::string("");
		}

#ifdef os_is_win
		int size_wstr		= WideCharToMultiByte(CP_ACP, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
		std::string ret_val = std::string(size_wstr, 0);
		WideCharToMultiByte(CP_ACP, 0, &wstr[0], (int)wstr.size(), &ret_val[0], size_wstr, NULL, NULL);//CP_UTF8

		return ret_val;
#else
#endif ///!os_is_win

		return std::string("");
	}

	/**
	*	@brief:
	*/
	std::wstring helper::str2wstr(const std::string& str)
	{
		if (str.empty())
		{
			return std::wstring(L"");
		}

#ifdef os_is_win
		int size_str = MultiByteToWideChar(CP_ACP, 0, &str[0], (int)str.size(), NULL, 0);
		std::wstring ret_val = std::wstring(size_str, 0);
		MultiByteToWideChar(CP_ACP, 0, &str[0], (int)str.size(), &ret_val[0], size_str);

		return ret_val;
#else
#endif ///!os_is_win

		return std::wstring(L"");
	}


#ifdef used_in_mfc
//----------------------------------------------------------------------------------------
	/**
	*	@brief:
	*/
	long helper::cs_to_l(const CString& str, const int base /*= 10*/)
	{
		// ????1
			// return _wtoi(str);

			// ????2
		USES_CONVERSION;
		// ?????????????????? ??????????????0?? ?????????????????10-10?????16-16????
		return strtol(W2A(str), 0, base);
	}

	/**
	*	@brief:
	*/
	double helper::cs_to_d(const CString& str)
	{
		return _wtof(str);
	}

	/**
	*	@brief:
	*/
	char * helper::cs_to_pchar(const CString str)
	{
		USES_CONVERSION;
		return  W2A(str);
	}

	/**
	*	@brief:
	*/
	CString helper::pchar_to_cs(const char* parr)
	{
		// ???? ???????
		if (NULL == parr || nullptr == parr)
			return CString(L"");

		return CString(parr);
	}

//----------------------------------------------------------------------------------------
#endif //used_in_mfc

	/**
	*	@brief:
	*/
	long helper::s_to_l(const std::string& str, const int base /*= 10*/)
	{
		return strtol(str.c_str(), 0, base);
	}

	/**
	*	@brief:
	*/
	double helper::s_to_fd(const std::string& str)
	{
		return atof(str.c_str());
	}


	/**
	*	@brief:
	*/
	char * helper::s_to_pchar(const std::string& str)
	{
		return (char *)str.c_str();
	}

#ifdef has_cxx_11
	/**
	*	@brief:
	*/
	int helper::xxs_to_i(const std::string& str, const int base /*= 10*/)
	{
		return std::stoi(str, 0, base);
	}

	/**
	*	@brief:
	*/
	float helper::xxs_to_f(const std::string& str)
	{
		return std::stof(str);
	}

	/**
	*	@brief:
	*/
	double helper::xxs_to_d(const std::string& str)
	{
		return std::stod(str);
	}

	/**
	*	@brief:
	*/
	long helper::xxs_to_l(const std::string& str, const int base /*= 10*/)
	{
		return std::stol(str, 0, base);
	}

	/**
	*	@brief:
	*/
	unsigned long helper::xxs_to_ul(const std::string& str, const int base /*= 10*/)
	{
		return std::stoul(str, 0, base);
	}

	/**
	*	@brief:
	*/
	long long helper::xxs_to_ll(const std::string& str, const int base /*= 10*/)
	{
		return std::stoll(str, 0, base);
	}

	/**
	*	@brief:
	*/
	unsigned long long helper::xxs_to_ull(const std::string& str, const int base /*= 10*/)
	{
		return std::stoull(str, 0, base);
	}

	/**
	*	@brief:
	*/
	long double helper::xxs_to_ld(const std::string& str)
	{
		return std::stold(str);
	}
#endif /// has_cxx_11


	/**
*	@brief:
*/
	std::string helper::get_cwd()
	{
#ifdef os_is_win
		const int arr_size_256 = 256;
		char path[arr_size_256] = { 0 };

		auto tchar2char = [](const TCHAR *ptchar_arr, char * pchar_arr)
		{
			if (NULL == ptchar_arr || nullptr == ptchar_arr)
				return;

			if (NULL == pchar_arr || nullptr == pchar_arr)
				return;

			int iLength = 0;
			//?????????   
			iLength = WideCharToMultiByte(CP_ACP, 0, ptchar_arr, -1, NULL, 0, NULL, NULL);

			//??tchar?????_char    
			WideCharToMultiByte(CP_ACP, 0, ptchar_arr, -1, pchar_arr, iLength, NULL, NULL);
		};

#ifdef UNICODE
		TCHAR tpath[arr_size_256] = { 0 };
		GetModuleFileName(NULL, tpath, arr_size_256);
		tchar2char(tpath, path);
#else
		GetModuleFileName(NULL, path, arr_size_256);
#endif ///	
		// win:path = XX\xx\xx\x\F.exe
		std::string str_ret_val(path);
		int last_pos = str_ret_val.find_last_of('\\');

		// ?????\\????????
		if (-1 == last_pos)
		{
		}
		else
		{
			// ?????????????F.exe
			str_ret_val = str_ret_val.substr(0, last_pos);
		}

		return str_ret_val;

#else
		return std::string("");
#endif //os_is_win

	} 
}/// utils





