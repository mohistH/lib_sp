#include "serial_port/serial_port_params.h"

#ifdef has_cxx_11


namespace lib_sp
{

	/**
	*	@brief:
	*/
	sp_params_check::sp_params_check()
	{
		init_map();
	}



	/**
	*	@brief:
	*/
	sp_params_check sp_params_check::get_instance()
	{
		static sp_params_check instance;

		return instance;
	}

	/**
	*	@brief:
	*/
	int sp_params_check::is_right(const sp_prop& spp)
	{
		int ret_val = 0;


		// 1. name
		if (0 == spp._name.length())
		{
			ret_val = -1;
			return ret_val;
		}

		int tmp_spp_item = 0;



		// 2. baud
		if (0 != map_find(_map_baud, spp._baud_rate))
		{
			return ret_val;
		}

		// 3. stop bits
		if (0 != map_find(_map_sb, spp._stop_bits))
		{
			ret_val = -3;
			return ret_val;
		}

		// 4. data bits
		if (0 != map_find(_map_db, spp._data_bits))
		{
			ret_val = -4;
			return ret_val;
		}

		// 5. flow control
		if (0 != map_find(_map_fctrl, spp._flow_ctl))
		{
			ret_val = -5;
			return ret_val;
		}

		// 6. parity
		if (0 != map_find(_map_parity, spp._parity))
		{
			ret_val = -6;
			return ret_val;
		}

		return 0;
	}

#ifdef os_is_linux
	/**
	*	@brief: to get baud
	*/
	int sp_params_check::get_baud(lib_sp::baud_rate & key)
	{
		int ret_val = 9600;//B9600;
		std::map<int, int>::iterator it_bd = _map_baud.find(key);
		if (_map_baud.end() != it_bd)
		{
			ret_val = it_bd->second;
		}

		return ret_val;
	}

	
#endif //!os_is_linux	


	/**
	*	@brief:
	*/
	void sp_params_check::init_map()
	{

		auto insert_map = [](map_type& _map, int&& key, int&&val)
		{
			_map.insert(std::pair<int, int>(key, val));
		};

		// 1. insert baud
//----------------------------------------------------------------------------------------
		insert_map(_map_baud, lib_sp::baud_rate_110,	lib_sp::baud_rate_110	);
		insert_map(_map_baud, lib_sp::baud_rate_300,	lib_sp::baud_rate_300	);
		insert_map(_map_baud, lib_sp::baud_rate_600,	lib_sp::baud_rate_600	);
		insert_map(_map_baud, lib_sp::baud_rate_1200,	lib_sp::baud_rate_1200	);
		insert_map(_map_baud, lib_sp::baud_rate_2400,	lib_sp::baud_rate_2400	);
		insert_map(_map_baud, lib_sp::baud_rate_4800,	lib_sp::baud_rate_4800	);
		insert_map(_map_baud, lib_sp::baud_rate_9600,	lib_sp::baud_rate_9600	);
		insert_map(_map_baud, lib_sp::baud_rate_14400,	lib_sp::baud_rate_14400	);
		insert_map(_map_baud, lib_sp::baud_rate_19200,	lib_sp::baud_rate_19200	);
		insert_map(_map_baud, lib_sp::baud_rate_38400,	lib_sp::baud_rate_38400	);
		insert_map(_map_baud, lib_sp::baud_rate_56000,	lib_sp::baud_rate_56000	);
		insert_map(_map_baud, lib_sp::baud_rate_57600,	lib_sp::baud_rate_57600	);
		insert_map(_map_baud, lib_sp::baud_rate_115200, lib_sp::baud_rate_115200 );
		insert_map(_map_baud, lib_sp::baud_rate_921600, lib_sp::baud_rate_921600 );

		// 2. insert stop_bits
		//----------------------------------------------------------------------------------------
		insert_map(_map_sb, lib_sp::stop_1,		lib_sp::stop_1);
		insert_map(_map_sb, lib_sp::stop_1_5,	lib_sp::stop_1_5);
		insert_map(_map_sb, lib_sp::stop_2,		lib_sp::stop_2);


		// 3. insert flow_control
		//----------------------------------------------------------------------------------------
		insert_map(_map_fctrl, lib_sp::flow_ctl_none, 0);
		insert_map(_map_fctrl, lib_sp::flow_ctl_hardware, 1);
		insert_map(_map_fctrl, lib_sp::flow_ctl_software, 2);


		// 4. insert parity
		//----------------------------------------------------------------------------------------
		insert_map(_map_parity, lib_sp::parity_none, 0);
		insert_map(_map_parity, lib_sp::parity_odd, 1);
		insert_map(_map_parity, lib_sp::parity_even, 2);
#ifdef os_is_win
		insert_map(_map_parity, lib_sp::parity_mark, 3);
#endif //!os_is_win
		insert_map(_map_parity, lib_sp::parity_space, 4);


		// 5. data bits
		//----------------------------------------------------------------------------------------
		/*
		data_bits_5			= 5,
		data_bits_6			= 6,
		data_bits_7			= 7,
		data_bits_8			= 8,
		*/
		insert_map(_map_db, lib_sp::data_bits_5, lib_sp::data_bits_5);
		insert_map(_map_db, lib_sp::data_bits_6, lib_sp::data_bits_6);
		insert_map(_map_db, lib_sp::data_bits_7, lib_sp::data_bits_7);
		insert_map(_map_db, lib_sp::data_bits_8, lib_sp::data_bits_8);

	}


	/**
	*	@brief:
	*/
	int sp_params_check::map_find(map_type& spp_map, int key)
	{
		map_type::iterator it = spp_map.find(key);

		return ( spp_map.end() == it ? -1: 0);
	}

}


#endif // !has_cxx_11
