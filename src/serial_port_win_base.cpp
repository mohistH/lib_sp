#include "serial_port/serial_port_win_base.h"


// to check if os is win
#ifdef os_is_win
#include <process.h>
#include "utils.h"
#include "serial_port/serial_port_params.h"

#include <tchar.h>      // Includes the string functions.
#include <windows.h>    // Includes basic windows functionality.
#include <setupapi.h>   // Includes the SetupAPI.

// to check cxx11 status
#ifdef has_cxx_11

#ifdef use_fmt
#include "fmt/format.h"
#endif //! use_fmt

//--------------------------------------------------------------------------------------

namespace lib_sp
{
	/*
	*	@brief: constructor
	*/
	serial_port_win_base::serial_port_win_base() noexcept
	{
		_sp_param._spp.zero();
		_sp_param._comm_info._over_lapped_monitor.hEvent = CreateEvent(NULL, true, false, NULL);
	}

	/*
	*	@brief:destructor
	*/
	serial_port_win_base::~serial_port_win_base() noexcept
	{
		// it doesnt close
		if (is_opened())
		{
			// to close
			close();

			if (_sp_param._spp._is_to_log)
			{
#ifdef use_spdlog
				try
				{
					if (_sp_param._plog)
						spdlog::shutdown();
				}
				catch (const spdlog::spdlog_ex &ex)
				{
					// an error occured, but do anything
				}

#endif // use_spdlog
			}
			

		}
		else
		{
			// closed
		}
	}
//----------------------------------------------------------------------------------------------------------------

	/*
	*	@brief:
	*/
	int serial_port_win_base::init(const sp_prop& spp, irecv_data* precv_data /*= nullptr*/) noexcept
	{
		int ret_val = 0;

		// 1. to check params
		ret_val = sp_params_check::get_instance().is_right(std::move(spp));
		if (0 != ret_val)
			return ret_val;

		// 2. to get params 
		_sp_param._spp = spp;

		// 3. to get recv_data obj
		if (nullptr != precv_data || NULL != precv_data)
			_sp_param._thread._precv_data = precv_data;

		// 4. set flag is true: is_init
		_sp_param._is_init = true;


#ifdef use_spdlog
		if (_sp_param._spp._is_to_log)
		{
			auto max_size = 1024 * 1024 * 5;
			auto max_files = 3;
			_sp_param._plog = spdlog::rotating_logger_mt("lib_sp_log", "log/lib_sp_log.log", max_size, max_files);
		}
#endif //! use_spdlog

		log("init-success");

		return ret_val;
	}


	/*
	*	@brief:
	*/
	int serial_port_win_base::open() noexcept
	{
		// to define return value of this function
		int ret_val					= 0;

		comm_info_win& com			= _sp_param._comm_info;
		sp_prop& spp				= _sp_param._spp;
		sp_param_win& param			= _sp_param;
		sp_thread_win& sp_thread	= _sp_param._thread;

		// 1. doesnt initialize 
		if (!_sp_param._is_init)
		{
			ret_val = -1;
			log("open-failure, the comm doesnt open, you should run <init> function to init");
			return ret_val;
		}

		// 2. create file , before that, to prepare params firstly
		unsigned long cfg_size		= sizeof(COMMCONFIG);
		com._cfg.dwSize				= cfg_size;


		DWORD flags_attributes		= 0;
		if (lib_sp::mode_async		== spp._op_mode)
			flags_attributes		+= FILE_FLAG_OVERLAPPED;
	
		TCHAR *tc_com_name			= nullptr;
		spp._name					= std::string("\\\\.\\") + spp._name;
#ifdef UNICODE
		std::wstring wstr			= utils::helper::str2wstr(spp._name);
		tc_com_name					= const_cast<TCHAR *>(wstr.c_str());
#else
		tc_com_name					= const_cast<TCHAR*>(spp._name.c_str());
#endif // !UNICODE

		// doesnt opened
		if ( !this->is_opened() )
		{
			com._handle = CreateFile(	tc_com_name,
											GENERIC_READ | GENERIC_WRITE,	// read/write types
											0,								// comm devices must be opened with exclusive access
											NULL,							// no security attributes
											OPEN_EXISTING,					// comm devices must use OPEN_EXISTING
											flags_attributes,				// Async I/O or sync I/O
											NULL);

			// opened success
			if (INVALID_HANDLE_VALUE != com._handle)
			{
				// to get default parameter
				GetCommConfig(com._handle, &com._cfg, &cfg_size);
				GetCommState(com._handle, &(com._cfg.dcb));

				// set parameter
				com._cfg.dcb.BaudRate		= spp._baud_rate;
				com._cfg.dcb.ByteSize		= spp._data_bits;
				com._cfg.dcb.Parity			= spp._parity;
				com._cfg.dcb.StopBits		= spp._stop_bits;

				com._cfg.dcb.fBinary		= true;
				com._cfg.dcb.fInX			= false;
				com._cfg.dcb.fOutX			= false;
				com._cfg.dcb.fAbortOnError	= false;
				com._cfg.dcb.fNull			= false;

				set_flow_ctl(spp._flow_ctl);
				
				if (SetCommConfig(com._handle, &com._cfg, cfg_size))
				{
					// to clear reading and writting buffer
					PurgeComm(com._handle, PURGE_TXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR | PURGE_RXABORT);
				
					// to initialize event driven approach
					if (lib_sp::mode_async == spp._op_mode)
					{
						com._time_out.ReadIntervalTimeout			= MAXDWORD;
						com._time_out.ReadTotalTimeoutMultiplier	= 0;
						com._time_out.ReadTotalTimeoutConstant		= 0;
						com._time_out.WriteTotalTimeoutMultiplier	= 0;
						com._time_out.WriteTotalTimeoutConstant		= 0;

						SetCommTimeouts(com._handle, &com._time_out);

						// to set comm event , to mask need modify
						if (SetCommMask(com._handle, EV_TXEMPTY | EV_RXCHAR | EV_DSR)) 
						{
							sp_thread._is_running		= true;
							bool ret_value = start_thread_monitor();

							if (!ret_value)
							{
								sp_thread._is_running	= false;
								//ret_val.set(GetLastError(), "GetLastError" );
								ret_val = GetLastError();
								log("open-failure, to create thread error, GetLastError() = {}", ret_val);
								
							} /// !
						}
						else
						{
							// Failed to set Comm Mask. params are invalid
							//ret_val.set(-3, "parameters are invalid" );
							ret_val = -3;
							log("open-failure, parameters to set comm mask are invalid");

						} /// !SetCommMask
					} ///!lib_sp::mode_async == spp._op_mode
					else  
					{
						com._time_out.ReadIntervalTimeout			= MAXDWORD;
						com._time_out.ReadTotalTimeoutMultiplier	= 0;
						com._time_out.ReadTotalTimeoutConstant		= 0;
						com._time_out.WriteTotalTimeoutMultiplier	= 100;
						com._time_out.WriteTotalTimeoutConstant		= 500;

						SetCommTimeouts(com._handle, &com._time_out);
					}
				}/// !SetCommConfig
				else
				{
					//ret_val.set(-3, "parameters are invalid" );
					ret_val = -6;
					log("open-failure, parameters to set comm mask are invalid");
				}
			}
			else /// INVALID_HANDLE_VALUE == com._handle_com, failure
			{
				// open com error, get the information 
				switch (GetLastError())
				{
						// not found
					case ERROR_FILE_NOT_FOUND:
					{
						//ret_val.set(-2, "failure, cannot find the com");
						log("open-failure, failure, cannot find the comm");
						ret_val = -2;
					}
					break;

					// denied
					case ERROR_ACCESS_DENIED:
					{
						//ret_val.set(-3, "failure, the com denied access");
						log("open-failure, failure, the comm denied access");
						ret_val = -3;
					}
					break;

					default:
						//ret_val.set(-4, "failure, unknow error" );
						log("open-failure, failure,  unknow error");
						ret_val = -4;
						break;
				} /// swich
			} /// end handle == INVALID_HANDLE_VALUE
		}
		else /// is opened
		{
			//ret_val.set(-5, "failure, the comm is opened" );
			ret_val = -5;
			log("open-failure, failure,  the comm is opened");
		}

		// to close port if an error occured
		//if (0 != ret_val.id() )
		if (0 != ret_val)
		{
			close();
		}

		log("open-success");
	
		return ret_val;
	}

	/*
	*	@brief: to close comm
	*/
	int serial_port_win_base::close() noexcept
	{
		int ret_val						= 0;
		comm_info_win& comm_info		= _sp_param._comm_info;

		try
		{
			if (is_opened())
			{
				stop_thread_monitor();

				if (INVALID_HANDLE_VALUE != comm_info._handle)
				{
					// stop all event
					SetCommMask(comm_info._handle, 0);//SetCommMask(m_handle,0) stop WaitCommEvent()

					//Discards all characters from the output or input buffer of a specified communications resource. It can also terminate pending read or write operations on the resource.
					PurgeComm(comm_info._handle, PURGE_TXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR | PURGE_RXABORT);

					CloseHandle(comm_info._handle);
					comm_info._handle = INVALID_HANDLE_VALUE;
				}

				ResetEvent(comm_info._over_lapped_monitor.hEvent);
			}
			else
			{
				//ret_val.set(-1, "failure, serial port doesnt open");
				ret_val = -1;
				log("close-failure, serial port doesnt open");
			}
		}
		catch (...)
		{
			//ret_val.set( GetLastError(), "GetLastError" );
			ret_val = GetLastError();
			log("close-failure, an error occurred, GetLastError() = {}", ret_val);
		}

		log("close-success");

		return ret_val;
	}


	/*
	*	@brief:
	*/
	void serial_port_win_base::set_read_notify(const unsigned int len) noexcept
	{
		_sp_param._comm_info._min_bytes_read_notify = (0 == len) ? 1 : len;
	}

	/*
	*	@brief: to check if comm opened
	*/
	bool serial_port_win_base::is_opened() noexcept
	{
		return (INVALID_HANDLE_VALUE == _sp_param._comm_info._handle) ? false : true;
	}

	/*
	*	@brief: to send data 
	*/
	int serial_port_win_base::send(const char* pdata, const unsigned int data_len) 
	{
		int ret_val		= 0;

		// 1. to check params
		if (NULL	== pdata || nullptr == pdata || 0 == data_len)
		{
			//ret_val.set(-1, "failure, pdata parameter is null / data_len is 0");
			ret_val		= -1;
			log("send-failure, pdata parameter is null / data_len is 0");

			return ret_val;
		}
		
		try
		{
			// 2. to check comm status
			if (is_opened())
			{
				sp_prop& spp	= _sp_param._spp;
				comm_info_win& comm		= _sp_param._comm_info;
				BOOL	breturn			= TRUE;
				BOOL	bwrite			= TRUE;
				DWORD	len_real_write	= 0;

				if (lib_sp::mode_async	== spp._op_mode)
				{
					comm._over_lapped_write.Internal		= 0;
					comm._over_lapped_write.InternalHigh	= 0;
					comm._over_lapped_write.Offset			= 0;
					comm._over_lapped_write.OffsetHigh		= 0;
					comm._over_lapped_write.hEvent			= CreateEvent(NULL, true, false, NULL);

					// The GetLastError code ERROR_IO_PENDING is not a failure; 
					// it designates the write operation is pending completion asynchronously.
					breturn = WriteFile(comm._handle, (void*)pdata, (DWORD)data_len, &len_real_write, &comm._over_lapped_write);

					if (!breturn)
					{
						// an error occured
						DWORD error_id = GetLastError();
						switch (error_id)
						{
						case ERROR_IO_PENDING:
						{
							bwrite			= FALSE;
							log("send-success(400)");
							len_real_write	= -1;

							ret_val = data_len;
						}
						break;

						// comm denied to access
						case ERROR_ACCESS_DENIED:
						{
							log("send-failure 403, an error occurred while writing, GetLastError() ={}", error_id);
						}
						break;

						// comm is invalid , or it doesnt open
						case ERROR_INVALID_HANDLE:
						{
							log("send-failure 410, an error occurred while writing, GetLastError() ={}", error_id);
						}
						break;

						// disconnected while connectting
						case ERROR_BAD_COMMAND:
						{
							log("send-failure 417, an error occurred while writing, GetLastError() ={}", error_id);
						}
						break;

						default:
							break;
						} // switch
					}
					else
					{
						// it written successfully
						log("send-success(433)");
					}

					// 
					if (!bwrite)
					{
						breturn = GetOverlappedResult(comm._handle, &comm._over_lapped_write, &len_real_write, TRUE);

					}

					// an error occured while writting
					if (!breturn)
					{
						//ret_val.set(GetLastError(), "GetLastError");
						ret_val = GetLastError();
						log("send-failure 443, an error occurred while writing, GetLastError() ={}", ret_val);
					}
					else
					{
						// alright, it written;
					}
				} // ib_sp::mode_async- if
				else
				{
					if (WriteFile(comm._handle, (void*)pdata, (DWORD)data_len, &len_real_write, NULL))
					{
						// successfully
						log("send-success");
					}
					else
					{
						//ret_val.set(GetLastError(), "GetLastError");
						ret_val = GetLastError();
						log("send-failure 461, GetLastError()={}", ret_val);
					}

				} /// lib_sp::mode_async end

				return ret_val;
			}
			else
			{
				// comm doesnt open
				log("send-failure 471, the comm doesnt open");
			}

			// 3. doesnt open, canot send the data 
			//ret_val.set(-2, "failure, serial port doesnt open");
			ret_val = -2;
		}
		catch (...)
		{
			//ret_val.set(GetLastError(), "GetLastError");
			ret_val = GetLastError();
			log("send-failure 482, an error occured, GetLastError() = {}", ret_val);
		}

		return ret_val;
	}

	/*
	*	@brief:
	*/
	std::string serial_port_win_base::get_version() noexcept
	{
		return utils::helper::str_format("%d.%d.%d.%d", version_1, version_2, version_3, version_4);
	}


	/**
	*	@brief:
	*/
	lib_sp::list_sp_name serial_port_win_base::get_available_serial_port() noexcept 
	{
		// 1. to prepare parameters
		lib_sp::sp_name_desc sp_info_item;
		list_sp_name list_sp;

		try
		{
			HDEVINFO hdev_info = INVALID_HANDLE_VALUE;
			// to get information
			hdev_info = SetupDiGetClassDevs(&GUID_DEVINTERFACE_COMPORT,
				NULL,
				NULL,
				DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

			if (INVALID_HANDLE_VALUE != hdev_info)
			{
				SP_DEVICE_INTERFACE_DETAIL_DATA *pdev_data = nullptr;
				DWORD didd_size = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA) + 256;

				pdev_data = (SP_DEVICE_INTERFACE_DETAIL_DATA*)new(std::nothrow) char[didd_size];
				if (NULL != pdev_data && nullptr != pdev_data)
				{
					// 
					pdev_data->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

					SP_DEVICE_INTERFACE_DATA did_item = { 0 };
					did_item.cbSize = sizeof(did_item);

					// alright, to get dev info
					BOOL ret_val = TRUE;
					for (DWORD index = 0; ret_val; index++)
					{
						// to get the device interfaces
						ret_val = SetupDiEnumDeviceInterfaces(hdev_info,	// DeviceInfoSet
							NULL,		// DeviceInfoData
							&GUID_DEVINTERFACE_COMPORT, // InterfaceClassGuid
							index,		// MemberIndex 
							&did_item);	// DeviceInterfaceData

						if (ret_val)
						{
							SP_DEVINFO_DATA dev_data = { sizeof(SP_DEVINFO_DATA) };

							// to get dev details
							ret_val = SetupDiGetDeviceInterfaceDetail(hdev_info,	// DeviceInfoSet
								&did_item,	// DeviceInterfaceData
								pdev_data,	// DeviceInterfaceDetailData
								didd_size,	// DeviceInterfaceDetailDataSize
								NULL,		// RequiredSize
								&dev_data);// DeviceInfoData

							if (ret_val)
							{
								const int buf_size_256 = 256;
								wchar_t str_description[buf_size_256] = { 0 };

								ret_val = SetupDiGetDeviceRegistryProperty(hdev_info,			// DeviceInfoSet
									&dev_data,			// DeviceInfoData
									SPDRP_FRIENDLYNAME,	// Property
									NULL,				// PropertyRegDataType
									(PBYTE)str_description,	// PropertyBuffer
									buf_size_256,		// PropertyBufferSize
									NULL);				// RequiredSize

								if (ret_val)
								{
									wchar_t str_port_name[buf_size_256] = { 0 };

									// to get name
									HKEY key_dev = SetupDiOpenDevRegKey(hdev_info,	// DeviceInfoSet
										&dev_data,	// DeviceInfoData
										DICS_FLAG_GLOBAL,	// Scope	
										0,			// HwProfile
										DIREG_DEV,	// KeyType
										KEY_READ);	// samDesired

									if (INVALID_HANDLE_VALUE != key_dev)
									{
										DWORD name_len_255 = 255;

										RegQueryValueEx(key_dev,			// hKey
											_T("PortName"),		// lpValueName
											NULL,				// lpReserved
											NULL,				// lpType
											(BYTE*)str_port_name,	// lpData
											&name_len_255);		// lpcbData
										RegCloseKey(key_dev);

										ret_val = TRUE;
									}
									else
									{
										ret_val = FALSE;
									}/// SetupDiOpenDevRegKey


									if (ret_val)
									{
#ifdef UNICODE
										sp_info_item._name = utils::helper::wstr2str(str_port_name);
										sp_info_item._description = utils::helper::wstr2str(str_description);
#else
										sp_info_item._name = std::string(str_port_name);
										sp_info_item._description = std::string(str_description);
#endif
										list_sp.push_back(sp_info_item);
										sp_info_item.zero();
									}
									else
									{
										// doesnt get the name,
									}
								}
								else
								{

								} /// SetupDiGetDeviceRegistryProperty

							}
							else
							{
								// an error occurred. 
								const int error_id = GetLastError();
								delete[](char*)pdev_data;
								pdev_data = nullptr;
								break;
							} /// SetupDiGetDeviceInterfaceDetail
						}
						else
						{

						} /// SetupDiEnumDeviceInterfaces
					} /// for

					// to release data
					delete[](char*)pdev_data;
					pdev_data = nullptr;

				} ///  if (NULL != pdev_data && nullptr != pdev_data)
				else
				{

				}

				// to close after reading
				SetupDiDestroyDeviceInfoList(hdev_info);

			} // INVALID_HANDLE_VALUE != hdev_info
			else
			{
				// an error occurred
			}
		}
		catch (...)
		{
			list_sp.clear();
		}

		return list_sp;
	}

	/*
	*	@brief:
	*/
	bool serial_port_win_base::get_recv_thread_is_running() noexcept
	{
		bool ret_val = false;

		_sp_param._thread._mutex_is_running.lock();
		ret_val		= _sp_param._thread._is_running;
		_sp_param._thread._mutex_is_running.unlock();

		return ret_val;
	}

	/*
	*	@brief:
	*/
	void serial_port_win_base::set_recv_thread_is_running(bool bvalue) noexcept
	{
		_sp_param._thread._mutex_is_running.lock();
		_sp_param._thread._is_running = bvalue;
		_sp_param._thread._mutex_is_running.unlock();
	}

	/*
	*	@brief:
	*/
	lib_sp::sp_param_win& serial_port_win_base::get_sp_param_win()noexcept
	{
		return _sp_param;
	}

	/*
	*	@brief: to set flow control
	*/
	void serial_port_win_base::set_flow_ctl(const lib_sp::flow_control &fc)noexcept
	{
		// to prepare param
		comm_info_win &com					= _sp_param._comm_info;

		if (is_opened())
		{
			switch (fc)
			{
			case lib_sp::flow_ctl_none:
				com._cfg.dcb.fOutxCtsFlow	= FALSE;
				com._cfg.dcb.fRtsControl	= RTS_CONTROL_DISABLE;
				com._cfg.dcb.fInX			= FALSE;
				com._cfg.dcb.fOutX			= FALSE;

				SetCommConfig(com._handle, &com._cfg, sizeof(COMMCONFIG));
				break;

			case lib_sp::flow_ctl_hardware:
				com._cfg.dcb.fOutxCtsFlow	= FALSE;
				com._cfg.dcb.fRtsControl	= RTS_CONTROL_DISABLE;
				com._cfg.dcb.fInX			= TRUE;
				com._cfg.dcb.fOutX			= TRUE;

				SetCommConfig(com._handle, &com._cfg, sizeof(COMMCONFIG));
				break;

			case lib_sp::flow_ctl_software:
				com._cfg.dcb.fOutxCtsFlow	= TRUE;
				com._cfg.dcb.fRtsControl	= RTS_CONTROL_HANDSHAKE;
				com._cfg.dcb.fInX			= FALSE;
				com._cfg.dcb.fOutX			= FALSE;

				SetCommConfig(com._handle, &com._cfg, sizeof(COMMCONFIG));
				break;

			default: /// unknow flow,  do nothing
				break;
			}
		}
		else /// doesnt open
		{
			/// do nothing
		}
	}

	/*
	*	@brief: to create a thread 
	*/
	bool serial_port_win_base::start_thread_monitor() 
	{
		sp_thread_win& sp_thread	= _sp_param._thread;

		try
		{
#ifdef use_cxx11_thread
			sp_thread._thread_recv = std::thread(comm_thread_monitor, this);
			// sp_thread._future_recv		= std::async(std::launch::async, comm_thread_monitor, this);

			sp_thread.cxx11_get_thread_recv_id();
#else
			sp_thread._handle		= (HANDLE)_beginthreadex(NULL,
				0,
				comm_thread_monitor,
				(LPVOID)this,
				0,
				NULL);
#endif // 

		}
		catch (...)
		{
			return false;
		}

		return (INVALID_HANDLE_VALUE == sp_thread._handle) ? false : true;
	}

	/*
	*	@brief: 
	*/
	bool serial_port_win_base::stop_thread_monitor()
	{
		SetCommMask(_sp_param._thread._handle, 0);
		
		set_recv_thread_is_running(false);

		try
		{
			if (INVALID_HANDLE_VALUE != _sp_param._thread._handle)
			{
#ifdef use_cxx11_thread

				_sp_param._thread._thread_recv.detach();
#else
				WaitForSingleObject(_sp_param._thread._handle, 512);
				CloseHandle(_sp_param._thread._handle);
#endif // 
				_sp_param._thread._handle = INVALID_HANDLE_VALUE;
			}

		}
		catch (...)
		{
			return false;
		}

		return true;
	}

	/*
	*	@brief: thread process
	*/
	unsigned int __stdcall serial_port_win_base::comm_thread_monitor(void *lpparam) noexcept
	{
		serial_port_win_base* psp_win_base = reinterpret_cast<serial_port_win_base*>(lpparam);
		if (NULL == psp_win_base || nullptr == psp_win_base)
		{
			return 0;
		}

		//---------------------------------------------------------------------------------
		
		// to prepare parameters
		COMSTAT comm_status;
		DWORD error_id				= 0;
		DWORD event_mask			= 0;
		int		ret_val				= 0;
		sp_param_win& sp_param		= psp_win_base->get_sp_param_win();
		comm_info_win& comm			= sp_param._comm_info;
		sp_prop& spp		= sp_param._spp;
		sp_param_win& param			= sp_param;
		sp_thread_win& sp_thread	= sp_param._thread;


		if (INVALID_HANDLE_VALUE == comm._handle)
		{
			return 0;
		}

		ResetEvent(comm._handle);

		while (psp_win_base->get_recv_thread_is_running())
		{
			// 
			if (!WaitCommEvent(comm._handle, &event_mask, &comm._over_lapped_monitor))
			{
				if (ERROR_IO_PENDING != GetLastError())
				{
					//WaitCommEvent error
				}
				else
				{
					// alright
				}
			} /// end if (WaitCommEvent ...)

			if (WAIT_OBJECT_0 == WaitForSingleObject(comm._over_lapped_monitor.hEvent, INFINITE))
			{
				// the event occurred
				DWORD lpNumberOfBytesTransferred = 0;
				if (!GetOverlappedResult(comm._handle, &comm._over_lapped_monitor, &lpNumberOfBytesTransferred, false))
				{
					return 1;
				}

				// the inputting buffer got an new charactor.
				if (EV_RXCHAR & event_mask)
				{
					ClearCommError(comm._handle, &error_id, &comm_status);
					if (comm._min_bytes_read_notify <= comm_status.cbInQue) // ???????,???? 1
					{
						// to read comm data
						psp_win_base->read_data();
					}
				}

				//  the sending buffer is null 
				if (EV_TXEMPTY & event_mask)
				{
					DWORD lpNumberOfBytesTransferred = 0;
					GetOverlappedResult(comm._handle, &comm._over_lapped_monitor, &lpNumberOfBytesTransferred, true);
					//std::cout << "EV_TXEMPTY" << std::endl;
				}

				// the signal of dsr changed
				if (EV_DSR & event_mask)
				{
					//std::cout << "EV_DSR" << std::endl;
				}
			} /// waitforsingleobject
		} /// while

		return 0;
	}

	/*
	*	@brief: to call read_file gets data
	*/
	int serial_port_win_base::read_data() noexcept
	{
		// to prepare params 
		comm_info_win& comm			= _sp_param._comm_info;
		sp_prop& spp		= _sp_param._spp;
		sp_param_win& param			= _sp_param;
		sp_thread_win& sp_thread	= _sp_param._thread;
		DWORD max_size				= lib_sp::len_buf_1024;

		if (INVALID_HANDLE_VALUE	!= comm._handle)
		{
			return 0;
		}
		
		if (lib_sp::mode_async		== spp._op_mode)
		{
			DWORD dwError = 0;
			COMSTAT comstat;
			ClearCommError(comm._handle, &dwError, &comstat);
			max_size = comstat.cbInQue;

			// to avoid array overflowing
			if (lib_sp::len_buf_1024 < max_size)
				max_size = lib_sp::len_buf_1024;
		}
		else
		{
			//Synchronous ClearCommError doesnt work
			//max_size = lib_sp::len_buf_1024;
		}

		//-----------------------------------------------------------------------------
		// to read all data from comm
		//-----------------------------------------------------------------------------

		// to reset arrary of reading
		static char	arr_read[len_buf_1024] = { 0 };
		memset(arr_read, 0, len_buf_1024);

		// mode_async ?????
		if (lib_sp::mode_async == _sp_param._spp._op_mode)
		{
			comm._over_lapped_read.Internal			= 0;
			comm._over_lapped_read.InternalHigh		= 0;
			comm._over_lapped_read.Offset			= 0;
			comm._over_lapped_read.OffsetHigh		= 0;
			comm._over_lapped_read.hEvent			= CreateEvent(NULL, true, false, NULL);
			DWORD nNumberOfBytesToRead				= 0;

			if (ReadFile(comm._handle, (void*)arr_read, (DWORD)max_size, &nNumberOfBytesToRead, &comm._over_lapped_read))
			{
				// alright, and notify on_recv_data
				if (nullptr != sp_thread._precv_data)
				{
					if ( lib_sp::len_buf_1024		< nNumberOfBytesToRead )
					{
						// 
						// sp_thread._precv_data->on_recv_data(arr_read, len_buf_1024);
					}
					else
					{
						sp_thread._precv_data->on_recv_data(arr_read, nNumberOfBytesToRead);
					}
				}
			}
			else
			{
				// to get data failed 
				// comm doesnt have any data
				if (ERROR_IO_PENDING == GetLastError() )
				{
					// to check further information
					GetOverlappedResult(comm._handle, &comm._over_lapped_read, &nNumberOfBytesToRead, true);
				}
				else
				{
					// other errors, 
					nNumberOfBytesToRead = (DWORD)-1;
				}
			}
			// read is over 
			CloseHandle(comm._over_lapped_read.hEvent);
		}
		else
		{
			// mode_sync ??????
			DWORD nNumberOfBytesToRead = 0;

			if (ReadFile(comm._handle, (void*)arr_read, (DWORD)max_size, &nNumberOfBytesToRead, NULL))
			{
				// alright
				if (lib_sp::len_buf_1024 < nNumberOfBytesToRead)
				{
					// 
					// sp_thread._precv_data->on_recv_data(arr_read, len_buf_1024);
				}
				else
				{
					sp_thread._precv_data->on_recv_data(arr_read, nNumberOfBytesToRead);
				}
			}
			else
			{
				// an error occurred
				nNumberOfBytesToRead = (DWORD)-1;
			}
		} /// end if mode_operate

		return 0;
	}


} // !lib_sp


#endif // !has_cxx_11

#endif // !os_is_win

