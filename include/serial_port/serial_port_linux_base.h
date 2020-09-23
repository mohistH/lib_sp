#pragma once
#include "serial_port/serial_port_interface.h"
#include "serial_port/constdef.h"
#include <string.h>

#ifdef has_cxx_11
#ifdef os_is_linux

#define linux_is_debug
#ifdef linux_is_debug
	#include <iostream>
#endif // linux_is_debug


//----------------------------------------------------------------------------------------
// to definite use fmt and spdlog
//----------------------------------------------------------------------------------------
// fmt
#ifndef use_fmt
	// #define use_fmt
#endif // use_fmt

// spdlog
#ifndef use_spdlog
	// #define use_spdlog
#endif // use_spdlog

//----------------------------------------------------------------------------------------
#ifdef use_spdlog
	#include "spdlog/spdlog.h"
	#include "spdlog/sinks/rotating_file_sink.h"
#endif // !use_spdlog





namespace lib_sp
{
	struct serial_port_param_linux_thread_
	{
		// to recv data
		irecv_data	*_precv_data			= nullptr;

		bool		_thread_recv_is_running = false;

		pthread_t _thread_recv				= invalid_value;
		pthread_mutex_t _mutex_recv;

		void zero()
		{
			_precv_data						= nullptr;
			_thread_recv_is_running			= false;
			_thread_recv					= invalid_value;
			memset(&_mutex_recv, 0, sizeof(pthread_mutex_t));
		}

		serial_port_param_linux_thread_()
		{
			zero();
		}
	};

	using sp_linux_thread = serial_port_param_linux_thread_;


	/**
	* @brief: others params of serial port on linux
	*/
	struct serial_port_param_others_
	{
		unsigned int _min_byte_read_notify	= 1;

		void zero()
		{
			_min_byte_read_notify			= 1;
		}

		serial_port_param_others_()
		{
			zero();
		}
	};

	using sp_linux_others = serial_port_param_others_;



	/**
	* @brief: the parameters of linux to initialize serial port
	*/
	struct serial_port_param_linux_
	{
		// the 
		sp_prop				_spp;

		// thread
		sp_linux_thread		_thread;
	
		// universal flag to operate serial port
		int					_fd;

		// others params
		sp_linux_others		_others;

		void zero()
		{
			_spp.zero();
			_thread.zero();
			_fd					= invalid_value;
			_others.zero();
		}

		serial_port_param_linux_()
		{
			zero();
		}
	};

	// the parameters of linux to initialize serial port
	using sp_param_linux = serial_port_param_linux_;

//----------------------------------------------------------------------------------------
// 
//----------------------------------------------------------------------------------------

	/**
	* @brief: serial port of linux 
	*/
	class serial_port_linux_base : public iserial_port
	{
	public:
		serial_port_linux_base() noexcept;
		virtual ~serial_port_linux_base() noexcept;

		serial_port_linux_base(const serial_port_linux_base& instance) noexcept = delete;
		serial_port_linux_base& operator = (const serial_port_linux_base& instance) noexcept = delete;

		serial_port_linux_base(const serial_port_linux_base&& instance) noexcept = delete;
		serial_port_linux_base& operator = (const serial_port_linux_base&& instance) noexcept = delete;
		//----------------------------------------------------------------------------------------------------


		/**
		*  @ brief: to initialize serial port's parameters
		*  @ spp - the properties of serial port
		*  @ precv_data - the object inherited irecv_data class is to recv data
						nullptr - doesnt recv data
		*  @ return - int
					0 - success
					-1 - failure, the serial port name is null
		*/
		int init(const sp_prop& spp, irecv_data* precv_data = nullptr) noexcept;

		/**
		*  @ brief: to open serial port
		*  @ return - int
					0 - success
					-1 - failure, the comm is openned
					-2 - failure, to set comm's configure failed
		*/
		int open() noexcept;

		/**
		*  @ brief: to close serial port that has been opened
		*  @ return - int
					0 - success
					-1 - failure, serial port doesnt open
					x > 0 - failure, x is the result of calling GetLastError
		*/
		int close() noexcept;

		/**
		*  @ brief: to set minimal bytes of reading to notify
		*  @ len - the length of minimal bytes
		*  @ return - void
		*/
		void set_read_notify(const unsigned int len) noexcept;

		/**
		*  @ brief: to check if the current serial port opened
		*  @ return - bool
					true - it is opened
					false - it is closed
		*/
		bool is_opened() noexcept;

		/**
		*  @ brief: write data to the serial port after opened
		*  @ pdata - write data
		*  @ data_len - the length of written data
		*  @ return - int
					-1 - failure, A??pdata parameter is null /  data_len is 0
								  B??call send data return
					-2 - failure, serial port doesnt open
					x > 0 - failure, x is the result of calling GetLastError
					0 - succcess
		*/
		int send(const char* pdata, const unsigned int data_len);

		/**
		*  @ brief: to get  information of current library version
		*  @ return - std::string
					the string of version
		*/
		std::string get_version() noexcept;


		/**
		*  @ brief: to get available information of serial port
		*  @ return - lib_sp::list_sp_name

		*/
		list_sp_name get_available_serial_port() noexcept;

		/**
		* @brief: the thread to read data
		*/
		static void * thread_recv_data_monitor(void * param);

		/**
		* @brief: to get recv data thread flag
		*/
		bool get_thread_recv_data_is_running()
		{
			return _sp_param._thread._thread_recv_is_running;
		}

		/**
		* @brief: to set recv data thread flag
		*/
		void set_thread_recv_data_is_running(const bool is_running)
		{
			_sp_param._thread._thread_recv_is_running = is_running;
		}


		sp_param_linux& get_sp_param_linux()
		{
			return _sp_param;
		}


		void lock()
		{
			pthread_mutex_lock(&_sp_param._thread._mutex_recv);
		}

		void unlock()
		{
			pthread_mutex_unlock(&_sp_param._thread._mutex_recv);
		}


		void log(std::string str)
		{
#ifdef linux_is_debug
			std::cout << str.c_str() << "\n\n";
#endif // !linux_is_debug
		}


	private:
		/**
		* @brief: set params of this port opened
		*/
		int set_port_params();

		bool start_thread_recv();
		int stop_thread_recv();



	private:
		sp_param_linux		_sp_param;

	};
}


#endif //!os_is_linux

#endif //!has_cxx_11



