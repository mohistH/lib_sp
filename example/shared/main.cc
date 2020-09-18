
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
		_psp = sp_create_win();
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
	int init(const serial_port_prop& spp, irecv_data* precv_data = nullptr) noexcept
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

private:
	iserial_port	*_psp = nullptr;
};


// the entry of this program
int main(int argc, char *argv[])
{	


	cout << "\n\n before started, it will show available comm information\n\n";
	std::list<lib_sp::sp_info> sp_list = serial_port_info::get_info();
	if (0 != sp_list.size())
	{
		for (auto item : sp_list)
		{
			cout << "\nname = " << item._name.c_str() << ", description = " << item._description.c_str() << "\n";
		}
	}


	cout << "\n\n\n";

//----------------------------------------------------------------------------------------
	int sp_id = 0;

	cout << "input serial port id = ";
	cin >> sp_id;

	int is_send = false;
	cout << sp_id << " is to send data ? (0 - false, 1 - true): ";
	cin >> is_send;


//---------------------------------------------------------------------------------
	std::unique_ptr<serial_port> sp(new(std::nothrow) serial_port);

	if (!sp)
	{
		std::cout << "\n\n 0. error, to create serial_port object failed\n\n\n";

#ifdef compiler_is_vs
		system("pause");
#endif // compiler_is_vs

		return 0;
	}
//---------------------------------------------------------------------------------
	
	// to prepare parameter of initializing
	serial_port_prop spp;

	// set comm name
	spp._name = std::string("COM" + to_string(sp_id));

	// to get function's return value
	int ret_val;

//---------------------------------------------------------------------------------
	// 1. initialise
	ret_val = sp->init(spp);
	
	// failed
	if (0 != ret_val )
	{
		std::cout << "\n 1. call init error, id = " << ret_val << "\n\n";

#ifdef compiler_is_vs
		system("pause");
#endif // compiler_is_vs
		ret_val = sp->close();

		return 0;
	}
	else
	{
		cout << "\n\n1. initialization successed\n\n";
	}

//---------------------------------------------------------------------------------
	// 2. open
	ret_val = sp->open();
	if (0 != ret_val)
	{
		std::cout << "\n2. call init error, id = " << ret_val << "\n\n";

#ifdef compiler_is_vs
		system("pause");
#endif // compiler_is_vs
		ret_val = sp->close();

		return 0;
	}

//---------------------------------------------------------------------------------	
	// 3. is_opened
	bool is_open = sp->is_opened();
	if (!is_open)
	{
		cout << "\n\n3. error,  comm doesnt open\n\n";

#ifdef compiler_is_vs
		system("pause");
#endif // compiler_is_vs

		ret_val = sp->close();

		return 0;
	}
	else
	{
		cout << "\n\n3. comm opened successfully\n\n";
	}

	
//---------------------------------------------------------------------------------
	// 4. get_version
	std::string str_ver = sp->get_version();
	cout << "\n\n4. version = " << str_ver.c_str() << "\n\n";

//---------------------------------------------------------------------------------
	// 5. send data
	if (1 == is_send)
	{ 
		std::string str_data = { "123ABC-+=" };

		for (int i = 0; i < 100; i++)
		{
			cout << "i = " << i + 1;
			ret_val = sp->send(str_data.c_str(), str_data.length());
			if (0 != ret_val)
			{
				cout << "5. to send failed,  id =  " << ret_val << "\n\n";
#ifdef compiler_is_vs
				system("pause");
#endif // compiler_is_vs

				ret_val = sp->close();

				return 0;
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


//---------------------------------------------------------------------------------
	// 6. close
	ret_val = sp->close();
	if (0 != ret_val)
	{
		cout << "6. to call close failed, id = " << ret_val << "\n\n";

#ifdef compiler_is_vs
		system("pause");
#endif // compiler_is_vs

		return 0;
	}
	else
	{
		cout << "\n\n6. to call close successed,\n\n";
	}

//---------------------------------------------------------------------------------
#ifdef compiler_is_vs
	system("pause");
#endif // compiler_is_vs

	return 0;
}