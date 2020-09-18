
#include "serial_port/serial_port_interface.h"

// to check if it has cxx11
#ifdef has_cxx_11

#ifdef os_is_win
#include "serial_port/serial_port_win_base.h"
#include "serial_port/serial_port_win_info.h"
#endif ///!os_is_win


namespace lib_sp
{

//--------------------------------------------------------------------------------------------------
	/*
	*	@brief: to  create an object on windows
	*/
	iserial_port * sp_create_win() noexcept
	{
		iserial_port* psp = nullptr;
#ifdef os_is_win
		psp					= new(std::nothrow) serial_port_win_base;
#endif ///!os_is_win

		return psp;
	}

	/*
	*	@brief: to release object, whose type is iserial_port*
	*/
	void sp_release(const iserial_port* psp) noexcept
	{
		if (nullptr != psp || NULL != psp)
		{
			delete psp;
			psp = nullptr;
		}
	}



/**
*	@brief:
*/
	std::list<lib_sp::sp_info> serial_port_info::get_info() noexcept
	{
#ifdef os_is_win
		return serial_port_win_info::get_info();
#elif os_is_linux

#elif os_is_apple

#endif // os_is_win
	}

}


#endif/// !has_cxx_11