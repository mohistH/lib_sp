#include "serial_port/serial_port_win_info.h"

#ifdef os_is_win
#include "utils.h"
using namespace utils;
#ifdef has_cxx_11


#include <tchar.h>      // Includes the string functions.


#include <windows.h>    // Includes basic windows functionality.
#include <setupapi.h>   // Includes the SetupAPI.

namespace lib_sp
{
	/**
	*	@brief: get information
	*/
	std::list<lib_sp::sp_info> serial_port_win_info::get_info()noexcept
	{
		// 1. to prepare parameters
		lib_sp::sp_info sp_info_item;
		std::list<lib_sp::sp_info> list_sp;

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
										sp_info_item._name			= helper::wstr2str(str_port_name);
										sp_info_item._description	= helper::wstr2str(str_description);
#else
										sp_info_item._name			= std::string(str_port_name);
										sp_info_item._description	= std::string(str_description);
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
}

#endif // has_cxx_11
#endif //!os_is_win