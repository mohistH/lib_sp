
#include "serial_port/serial_port_interface.h"

// to check if it has cxx11
#ifdef has_cxx_11

//--------------------------------------------------------------------------------------------------
#if defined(os_is_win)

	#include "serial_port/serial_port_win_base.h"

#elif defined(os_is_linux)

	#include "serial_port/serial_port_linux_base.h"

#else

#endif ///!os_is_win
//--------------------------------------------------------------------------------------------------


namespace lib_sp
{

	/**
	*	@brief: to  create an object on windows
	*/
	iserial_port * sp_create() noexcept
	{
		iserial_port* psp = nullptr;
#if defined(os_is_win)
		psp = new(std::nothrow) serial_port_win_base;
#elif defined(os_is_linux)
		psp = new(std::nothrow) serial_port_linux_base;
#endif ///!os_is_win

		return psp;
	}


	/**
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


	//--------------------------------------------------------------------------------------------------
} // lib_sp

#endif/// !has_cxx_11

