#pragma once
#include "serial_port/serial_port_interface.h"

// to check if os is win
#ifdef os_is_win
#include <windows.h>

// to check cxx11 status
#ifdef has_cxx_11

#include <thread>
#include <chrono>
#include <sstream>
#include <mutex>

// use cxx11 thread 
//#define use_cxx11_thread  

//--------------------------------------------------------------------------------------


namespace lib_sp
{
	/*
	* @brief: the version information and others enumeration
	*/
	enum
	{
		// max_buffer_read  1k
		len_buf_1024 = 1024,

	};

	/*
	* @brief: thread params
	*/
	struct serial_port_thread_win_
	{
		// to protect _is_running parameter of this structure
		std::mutex			_mutex_is_running;

		// to flag if thread is running
		bool				_is_running = false;

#ifdef use_cxx11_thread

		std::thread			_thread_recv;
		//std::future<unsigned int> _future_recv;
		long long			_thread_recv_id;
#endif /// use_cxx11_thread

		// the unique flag of this thread
		HANDLE	_handle = INVALID_HANDLE_VALUE;

		// to show data
		irecv_data* _precv_data = nullptr;
		
		void zero() noexcept
		{
			_is_running		= false;

#ifdef use_cxx11_thread
			_thread_recv_id = 0;
#endif /// use_cxx11_thread

			// the unique flag of this thread
			_handle			= INVALID_HANDLE_VALUE;

			// recv function
			_precv_data		= nullptr;
		}


		// get recv thread's id
		void cxx11_get_thread_recv_id() 
		{
#ifdef use_cxx11_thread
			std::stringstream sin;

			sin << _thread_recv.get_id();

			_thread_recv_id = std::stoll(sin.str());

			_handle = OpenThread(THREAD_ALL_ACCESS, FALSE, DWORD(_thread_recv_id));
#endif /// use_cxx11_thread
		}

		serial_port_thread_win_() noexcept
		{
			zero();
		}
	};
	// the thread params
	using sp_thread_win = serial_port_thread_win_;

	/*
	* @brief: some configure of com
	*/ 
	struct comm_info_win_
	{
		// the configure of comm
		COMMCONFIG		_cfg;

	
		COMMTIMEOUTS	_time_out;

		// 
		OVERLAPPED		_over_lapped_monitor;

		// 
		OVERLAPPED		_over_lapped_write;

		OVERLAPPED		_over_lapped_read;

		// a flag to operator com
		HANDLE			_handle				= INVALID_HANDLE_VALUE;

		unsigned int	_min_bytes_read_notify = 1;

		void zero() noexcept
		{
			memset(this, 0, sizeof(this));
			_over_lapped_monitor.hEvent		= INVALID_HANDLE_VALUE;
			_over_lapped_write.hEvent		= INVALID_HANDLE_VALUE;
			_over_lapped_read.hEvent		= INVALID_HANDLE_VALUE;
			_min_bytes_read_notify			= 1;
			_handle							= INVALID_HANDLE_VALUE;
		}

		comm_info_win_() noexcept
		{
			zero();
		}

	};
	// the comm information
	using comm_info_win = comm_info_win_;



	/*
	* @brief: thre parameters of serial port
	*/
	struct serial_port_param_win_
	{
		// to get serial port's properties
		serial_port_prop	_spp;

		// thread params
		comm_info_win		_comm_info;

		// thread info
		sp_thread_win		_thread;

		// has initialized
		bool				_is_init	= false;

		// has opened
		bool				_is_opened	= false;

		void zero() noexcept
		{
			_spp.zero();
			_comm_info.zero();
			_is_init	= false;
			_is_opened	= false;
			_thread.zero();
		}

		serial_port_param_win_() noexcept
		{
			zero();
		}
	};

	// the params of serial port on windows
	using sp_param_win = serial_port_param_win_;

//----------------------------------------------------------------------------------------------------


	/*
	* @brief: the implemention of windows 
	*/
	class serial_port_win_base : public iserial_port 
	{
	public:
		serial_port_win_base() noexcept;
		virtual ~serial_port_win_base() noexcept;

		serial_port_win_base(const serial_port_win_base& instance) noexcept = delete;
		serial_port_win_base& operator = (const serial_port_win_base& instance) noexcept = delete;

		serial_port_win_base(const serial_port_win_base&& instance) noexcept = delete;
		serial_port_win_base& operator = (const serial_port_win_base&& instance) noexcept = delete;
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
		int init(const serial_port_prop& spp, irecv_data* precv_data = nullptr) noexcept;

		/** 
		*  @ brief: to open serial port
		*  @ return - int
					0 - success
					-1 - failure, comm doesnt initialize
					-2 - failure, cannot find the comm
					-3 - failure, the com denied access
					-4 - failure, unknow error
					-5 - failure, the comm is opened
					-6 - failure, parameters are invalid
					x > 0 - failure, x is the result of calling GetLastError
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
					-1 - failure, pdata parameter is null /  data_len is 0
					-2 - failure, serial port doesnt open 
					x > 0 - failure, x is the result of calling GetLastError
					0 - succcess
		*/
		int send(const char* pdata, const unsigned int data_len) ;

		/** 
		*  @ brief: to get  information of current library version
		*  @ return - std::string
					the string of version
		*/
		std::string get_version() noexcept;

		
//----------------------------------------------------------------------------------------
		
		/** 
		*  @ brief: to check thread status
		*  @ return - bool
					true - running 
					false - over
		*/
		bool get_recv_thread_is_running() noexcept;

		/**
		* @brief: set thread running flag;
		*/
		void set_recv_thread_is_running(bool bvalue) noexcept;

		/**
		* @brief: to get parameters in recv thread
		*/
		sp_param_win& get_sp_param_win() noexcept;

	private:
		/**
		* @brief: to set flow control property 
		*/
		void set_flow_ctl(const lib_sp::flow_control &fc) noexcept;

		bool start_thread_monitor() ;
		bool stop_thread_monitor() ;

		/**
		* @brief:  the thread of receiving data
		*/
		static unsigned int __stdcall comm_thread_monitor(void *lpparam) noexcept;

		/**
		* @brief: to call function ReadFile receives data
		*/
		int read_data() noexcept;

	private:
		sp_param_win	_sp_param;
	};
}

#endif //!has_cxx_11

#endif //!os_is_win

