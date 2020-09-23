#pragma once
#include "serial_port/serial_port_interface.h"
#include <map>
#include <tuple>

#ifdef has_cxx_11



namespace lib_sp
{
	using map_type = std::map<int, int>;
	struct sp_prop_map_
	{
		map_type		_map_baud;
		map_type		_map_db;
		map_type		_map_sb;
		map_type		_map_fctrl;
		map_type		_map_parity;

		sp_prop_map_()
		{
			_map_baud		.clear();
			_map_db			.clear();
			_map_sb			.clear();
			_map_fctrl		.clear();
			_map_parity		.clear();		
		}
	};

	using sp_map = sp_prop_map_;


	/**
	* @brief: to check params of serial port
	*/
	class sp_params_check
	{
	public:

		sp_params_check();

		/**
		* @brief: 
		*/
		static sp_params_check get_instance();

		//----------------------------------------------------------------------------------------

		/**
		* @brief:  to check params 
		*/
		int is_right(const sp_prop& spp);

		//template<typename T, typename Key>
		//static bool is_found(T t, Key& key)
		//{
		//	auto get_t = std::move(t);
		//	std::tuple_element<0, decltype(get_t)>::type 

		//	return (get_t.end() == it) ? false : true;
		//}


#ifdef os_is_linux
		/**
		* @brief:  to get baud on linux
		*/
		int get_baud(lib_sp::baud_rate & key);
#endif //!os_is_linux		


	private:
		void init_map();

		int map_find(map_type& spp_map, int key);


	private:
		using map_type	= std::map<int, int>;
		map_type		_map_baud;
		map_type		_map_db;
		map_type		_map_sb;
		map_type		_map_fctrl;
		map_type		_map_parity;

	};



}


#endif // !has_cxx_11
