

#pragma once

#include <iostream>
#include <memory>
#include <chrono>
#include <thread>

using namespace std;

// to use iserial_port, you could include the following file
#include "serial_port/serial_port_interface.h"
// using namespace lib_sp
using namespace lib_sp;


// to recv data, it must inherit class irecv_data.
class serial_port : public irecv_data
{
public:
	serial_port() noexcept
	{
		// create an object
		_psp = sp_create();
	}

	virtual ~serial_port() noexcept
	{
		// release
		sp_release(_psp);
	}

	serial_port(const serial_port& instance) = delete;
	serial_port& operator = (const serial_port& instance) = delete;

	serial_port(const serial_port&& instance) = delete;
	serial_port&& operator = (const serial_port&& instance) = delete;



//------------------------------------------------------------------------------------------------------------------
	// to implemente this func to recv data
	void on_recv_data(const char* precv_data, const unsigned int len_recv_data) noexcept
	{
		cout << "\n\n7. received data, the length is " << len_recv_data << ", data = " << precv_data << "\n\n";
	}

//----------------------------------------------------------------------------------------------------------------
	int init(const sp_prop& spp, irecv_data* precv_data = nullptr) noexcept
	{
		if (nullptr != _psp)
		{
			// if you wanna recv data, set the second param this, otherwise, set it nullptr
			return _psp->init(spp, this);
		}
		
		return -10;
	}

	int open() noexcept 
	{
		if (nullptr != _psp)
			return _psp->open();

		return -10;
	}

	int close() noexcept
	{
		if (nullptr != _psp)
			return _psp->close();

		return -10;
	}

	bool is_opened() noexcept
	{
		if (nullptr != _psp)
			return _psp->is_opened();

		return false;
	}

	void set_read_notify(const unsigned int len) noexcept
	{
		if (nullptr != _psp)
			_psp->set_read_notify(len);
	}

	int send(const char* pdata, const unsigned int data_len) noexcept
	{
		if (nullptr != _psp)
			return _psp->send(pdata, data_len);

		return -10;
	}

	std::string get_version()noexcept 
	{
		if (nullptr != _psp)
			return _psp->get_version();

		return std::string("");
	}

	list_sp_name get_avail_serial_port()
	{
		if (nullptr != _psp)
			return _psp->get_available_serial_port();

		list_sp_name _list;
		return _list;
	}

private:
	iserial_port	*_psp = nullptr;
};

//----------------------------------------------------------------------------------------
// 
//----------------------------------------------------------------------------------------





// the entry of this program
int main(int argc, char *argv[])
{	


	// to create a serial_port object
//---------------------------------------------------------------------------------
	std::unique_ptr<serial_port> sp(new(std::nothrow) serial_port);
	// created failure
	if (!sp)
	{
		std::cout << "\n\n 0. error, to create serial_port object failed\n\n\n";

#ifdef compiler_is_vs
		system("pause");
#endif // compiler_is_vs

		return 0;
	}








	// to get function's return value
	int ret_val = 0;



	// 0.to get available serial port
//----------------------------------------------------------------------------------------
#ifdef os_is_win
	cout << "\n\n before started, it will show available comm information\n\n";

	list_sp_name sp_list = sp->get_avail_serial_port();
	if (0 != sp_list.size())
	{
		for (auto item : sp_list)
			cout << "\nname = " << item._name.c_str() << ", description = " << item._description.c_str() << "\n";
	}

	cout << "\n\n\n";

	int sp_id = 0;
	// to get input serial port id
	cout << "input serial port id = ";
	cin >> sp_id;

	int is_send = 0;
	cout << sp_id << " is to send data ? (0 - false, 1 - true): ";
	cin >> is_send;

#endif //!os_is_win





	// 2. to prepare parameter of initializing
//---------------------------------------------------------------------------------
	sp_prop spp;
#if defined( os_is_win )

	// set comm name
	spp._name = std::string("COM" + to_string(sp_id));
	spp._is_to_log = true;

#elif defined(os_is_linux)

	spp._name = std::string("/dev/ttyS0");

#endif //!



	// 3. to execute initialize
//---------------------------------------------------------------------------------
	ret_val = sp->init(spp);
	
	// failed
	if (0 != ret_val )
	{
		std::cout << "\n 1. call init error, id = " << ret_val << "\n\n";
		sp->close();

#ifdef compiler_is_vs
		system("pause");
#endif // compiler_is_vs

		return 0;
	}
	else
	{
		cout << "\n\n1. initialization successed\n\n";
	}





	// 4. to open serial port
//---------------------------------------------------------------------------------
	ret_val = sp->open();
	if (0 != ret_val)
	{
		std::cout << "\n2. call open error, id = " << ret_val << "\n\n";
		sp->close();

#ifdef compiler_is_vs
		system("pause");
#endif // compiler_is_vs

		return 0;
	}







	// 5.to check serial port status
//---------------------------------------------------------------------------------	
	bool is_open = sp->is_opened();
	if (!is_open)
	{
		cout << "\n\n3. error,  comm doesnt open\n\n";

#ifdef compiler_is_vs
		system("pause");
#endif // compiler_is_vs

		return 0;
	}
	else
	{
		cout << "\n\n3. comm opened successfully\n\n";
	}

	






	// 6. to get version information
//---------------------------------------------------------------------------------
	std::string str_ver = sp->get_version();
	cout << "\n\n4. version = " << str_ver.c_str() << "\n\n";

	
	
	
	
	
	// 7. send data
//---------------------------------------------------------------------------------
#if defined(os_is_win)
	if (1 == is_send)
	{ 
		std::string str_data = { "123ABC-+=" };

		for (int i = 0; i < 100; i++)
		{
			cout << "i = " << i + 1;

			ret_val = sp->send(str_data.c_str(), str_data.length());
			if (0 == ret_val)
			{
				cout << "5. to send failed\n\n";
				break;
			}
			else
			{
				cout << "5. to send successed\n\n";
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(40));
		}
	}
	else
	{
		cout << "wait for data ....\n\n";
		std::this_thread::sleep_for(std::chrono::seconds(10));
	}

//---------------------------------------------------------------------------------------
#elif defined (os_is_linux)

	bool is_to_send = false ; 
	if (is_to_send)
	{
		std::string str_data = { "123ABC-+=" };

		for (int i = 0; i < 100; i++)
		{
			cout << "i = " << i + 1;
			ret_val = sp->send(str_data.c_str(), str_data.length());
			if (0 == ret_val)
			{
				cout << "5. to send failed \n\n";
				break;
			}
			else
				cout << "5. to send successed\n\n";

			std::this_thread::sleep_for(std::chrono::milliseconds(40));
		}
	}
	else
	{
		cout << "waiting for data...... (10s)\n\n\n";
		std::this_thread::sleep_for(std::chrono::milliseconds(1000 * 10));
	}


#endif //! defined(os_is_win)






	// 8. to close serial port
//---------------------------------------------------------------------------------
	ret_val = sp->close();
	cout << "waitint is over, it will close right away\n\n";
	if (0 != ret_val)
		cout << "6. to call close failed, id = " << ret_val << "\n\n";
	else
		cout << "\n\n6. to call close successed,\n\n";

#ifdef compiler_is_vs
	system("pause");
#endif // compiler_is_vs

	return 0;
}
