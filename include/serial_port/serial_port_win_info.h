#pragma once
#include "serial_port/serial_port_interface.h"

#ifdef os_is_win

#ifdef has_cxx_11


namespace lib_sp
{
	class serial_port_win_info
	{
	public:
		/** 
		*  @ brief: get serial port information on windows
		*  @ return - std::list<lib_sp::sp_info>
		*/
		static std::list<lib_sp::sp_info> get_info() noexcept;



//----------------------------------------------------------------------------------------
		serial_port_win_info(serial_port_win_info& instance) = delete ;
		serial_port_win_info& operator = (serial_port_win_info& instance) = delete;

		serial_port_win_info(serial_port_win_info&& instance) = delete;
		serial_port_win_info&& operator = (serial_port_win_info&& instance) = delete;
//----------------------------------------------------------------------------------------
	};

}

#endif //has_cxx_11
#endif// os_is_win