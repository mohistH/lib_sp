#include "serial_port/serial_port_linux_base.h"
#include "serial_port/serial_port_params.h"
#include "config.h"
#include "utils.h"
#include "serial_port/constdef.h"

#ifdef os_is_linux

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <iostream>
#include <sys/ioctl.h>

#include <sys/ioctl.h>
#include <linux/serial.h>
#include <dirent.h>

using namespace std;



#ifdef has_cxx_11

#ifdef use_fmt
	#include "fmt/format.h"
#endif //! use_fmt

namespace lib_sp
{
	/**
	*	@brief: constructor
	*/
	lib_sp::serial_port_linux_base::serial_port_linux_base() noexcept
	{
		_sp_param.zero();
	}


	/**
	*	@brief: descontructor
	*/
	serial_port_linux_base::~serial_port_linux_base() noexcept
	{
		// to close serial port
		if (is_opened())
		{
			try
			{
				this->close();
			}
			catch (...)
			{
			}
		}
	}

	/**
	*	@brief: to initialize parameters
	*/
	int serial_port_linux_base::init(const sp_prop& spp, irecv_data* precv_data /*= nullptr*/) noexcept
	{
		int ret_val = 0;

		// 1. check parameters 
		ret_val = sp_params_check::get_instance().is_right(std::move(spp));
		if (0 != ret_val)
			return ret_val;

		// 2. 
		_sp_param._spp = spp;
		if (nullptr != precv_data || NULL != precv_data)
			_sp_param._thread._precv_data = precv_data;

		return ret_val;
	}

	/**
	*	@brief:
	*/
	int serial_port_linux_base::open() noexcept
	{
		// 1. serial port has been opened
		if (is_opened())
			return -1;

		int ret_val = 0;
		// 2.try to open serial port
		sp_param_linux &base_param 	= _sp_param;
		sp_prop &spp 				= _sp_param._spp;

		/*
			O_RDONLY ��ֻ����ʽ��   
			O_WRONLY ��ֻд��ʽ��   	
			O_RDWR �Կɶ���д��ʽ��

			O_CREAT ����ļ��������򴴽����ļ�
			O_EXCL ���ʹ��O_CREATѡ�����ļ����ڣ��򷵻ش�����Ϣ
			O_NOCTTY ����ļ�Ϊ�նˣ���ô�ն˲����Ե���openϵͳ���õ��Ǹ����̵Ŀ����ն�
			O_TRUNC ����ļ��Ѿ�������ɾ���ļ���ԭ������
			O_APPEND ��׷�ӵķ�ʽ��
		*/
		base_param._fd = ::open(spp._name.c_str(), O_RDWR  | O_NOCTTY | O_NDELAY);

		// 3. failed to open that
		if (invalid_value == base_param._fd)
		{	
			ret_val = errno;
			return ret_val;

		}

		// 4. to set port block or unblock
		if (0 > fcntl(base_param._fd, F_SETFL, 0))
		{
			// ret_val = -3;
			ret_val = errno;
			
			this->close();
			return ret_val;
		}

		// 5. try to set params of this port, but faile
		if (0 != set_port_params())
		{
			// ret_val = -4;
			ret_val = errno;
			this->close();
			return ret_val;
		}

		// 6. to create a thread to recv data
		if ((nullptr != _sp_param._thread._precv_data) || (NULL != _sp_param._thread._precv_data))
		{
			log(utils::helper::str_format("it will create thread....\n"));
			// to create recv thread failed
			if (!start_thread_recv())
			{
				ret_val = errno;
				log(utils::helper::str_format("failed....\n"));
			}
		}

		return ret_val;
	}

	/**
	*	@brief:
	*/
	int serial_port_linux_base::close() noexcept
	{
		int ret_val = 0;

		if (!is_opened())
		{
			ret_val = -1;
		}
		else
		{
			try
			{
				// to stop recv thread 
				stop_thread_recv();

				// call system func to close port
				::close(_sp_param._fd);
				_sp_param._fd	= invalid_value;
				
			}
			catch(...)
			{
				ret_val = errno;
			}
		}

		return ret_val;
	}

	/**
	*	@brief:
	*/
	void serial_port_linux_base::set_read_notify(const unsigned int len) noexcept
	{
		if (lib_sp::len_buf_1024 < len)
			_sp_param._others._min_byte_read_notify = lib_sp::len_buf_1024;
		else
			_sp_param._others._min_byte_read_notify = std::move(len);
	}

	/**
	*	@brief: to check serial port status
	*/
	bool serial_port_linux_base::is_opened() noexcept
	{
		return (invalid_value == _sp_param._fd) ? false : true;
	}

	/**
	*	@brief:
	*/
	int serial_port_linux_base::send(const char* pdata, const unsigned int data_len)
	{
		// 0.
		if (nullptr == pdata || NULL == pdata || 0 == data_len)
		{
			return -1;
		}


		lock();

		// 1. comm doesnt opened
		if (!is_opened())
		{
			return -2;
		}


		int ret_val = 0;
		try
		{
			// 2. to send data , ret_val = -1 ? X > 0
			ret_val = write(_sp_param._fd, pdata, data_len);	
		}
		catch(...)
		{
			ret_val = errno;
		}

		unlock();

		return ret_val;
	}

	/**
	*	@brief:
	*/
	std::string serial_port_linux_base::get_version() noexcept
	{
		return utils::helper::str_format("%d.%d.%d.%d", version_1, version_2, version_3, version_4);
	}

	/**
	*	@brief:
	*/
	lib_sp::list_sp_name serial_port_linux_base::get_available_serial_port() noexcept
	{

		// this is from: https://stackoverflow.com/questions/2530096/how-to-find-all-serial-devices-ttys-ttyusb-on-linux-without-opening-them 
//------------------------------------------------------------------------------------------------------
		// 1. get_driver
		auto get_driver = [](const std::string& tty)->std::string
		{
			struct stat st;
			memset(&st, 0, sizeof(struct stat));
			std::string devicedir = tty;

			// Append '/device' to the tty-path
			devicedir += "/device";

			// Stat the devicedir and handle it if it is a symlink
			if (lstat(devicedir.c_str(), &st) == 0 && S_ISLNK(st.st_mode))
			{
				char buffer[1024] = { 0 };
				memset(buffer, 0, sizeof(buffer));

				// Append '/driver' and return basename of the target
				devicedir += "/driver";

				if (0 < readlink(devicedir.c_str(), buffer, sizeof(buffer)))
					return basename(buffer);
			}

			return std::string("");
		};


		// 2.register_comport
		auto register_comport = [&](std::list<std::string>& comList, std::list<std::string>& comList8250, const std::string& dir)->void
		{
			// Get the driver the device is using
			std::string driver = get_driver(dir);

			// Skip devices without a driver
			if (0 < driver.size())
			{
				std::string devfile = std::string("/dev/") + basename(dir.c_str());

				// Put serial8250-devices in a seperate list
				if (driver == std::string("serial8250"))
					comList8250.push_back(devfile);
				else
					comList.push_back(devfile);
			}
		};

		// 3. probe_serial8250_comports
		auto probe_serial8250_comports = [&](std::list<std::string>& comList, std::list<std::string> comList8250)->void
		{
			struct serial_struct serinfo;
			std::list<std::string>::iterator it = comList8250.begin();

			// Iterate over all serial8250-devices
			while (it != comList8250.end())
			{

				// Try to open the device
				int fd = ::open(	(*it).c_str(), 
									O_RDWR | 
									O_NONBLOCK | 
									O_NOCTTY );

				if (0 < fd)
				{
					// Get serial_info
					if ( 0 == ioctl(fd, TIOCGSERIAL, &serinfo) )
					{
						// If device type is no PORT_UNKNOWN we accept the port
						if (serinfo.type != PORT_UNKNOWN)
							comList.push_back(*it);
					}

					::close(fd);
				}
				it++;
			}
		};

//----------------------------------------------------------------------------------------=====------------
	
		
		std::list<std::string> comList8250;
		std::list<std::string> comlist;

		struct dirent **name_list		= nullptr;
		const std::string str_sys_dir	= { "/sys/class/tty/" };
		

		int count_name_list = scandir(str_sys_dir.c_str(), &name_list, NULL, NULL);

		if (0 <= count_name_list)
		{
			int result			= 0;
			char *pitem_name	= NULL;
			for (int index = 0; index < count_name_list; index++)
			{
				pitem_name = name_list[index]->d_name;
				result = strcmp("..", pitem_name) && strcmp(".", pitem_name);
				if (0 == result)
				{
					// the default directory, passed
				}
				else
				{
					std::string device_dir = str_sys_dir + std::string(name_list[index]->d_name);

					// Register the device
					register_comport(comlist, comList8250, device_dir);

				}
				free(name_list[index]);
			}// end for
			free(name_list);
		} // end scandir
		else
		{
			// read error
		}

		// Only non-serial8250 has been added to comList without any further testing
		// serial8250-devices must be probe to check for validity
		probe_serial8250_comports(comlist, comList8250);


//--------------------------------------------------------------------------------------------------------
 		const std::string str_pts_dir	= { "/dev/pts/" };
		 count_name_list = scandir(str_pts_dir.c_str(), &name_list, NULL, NULL);
		if (0 <= count_name_list)
		{
			int result			= 0;
			char *pitem_name	= NULL;
			for (int index = 0; index < count_name_list; index++)
			{
				pitem_name = name_list[index]->d_name;
				result = strcmp("..", pitem_name) && strcmp(".", pitem_name);
				if (0 == result)
				{
					// the default directory, passed
				}
				else
				{
					std::string device_dir = str_sys_dir + std::string(name_list[index]->d_name);
					comlist.push_back(device_dir);	
				}
				free(name_list[index]);
			}// end for
			free(name_list);

		} // end if
		else
		{
			// read error , 
		}
	

//----------------------------------------------------------------------------------------
		// copy the name from comlist to lib_sp::list_sp_name
		
		lib_sp::list_sp_name ret_list;
		sp_name_desc item_list;

		std::list<std::string>::iterator it = comlist.begin();

		while (it != comlist.end())
		{
			item_list._name = (*it);
			ret_list.push_back(item_list);
			item_list.zero();

			it++;
		}
		
		return ret_list;
	}



	/**
	*	@brief: to set base params of this port
	*/
	int serial_port_linux_base::set_port_params()
	{
		/**
		* {unsigned short c_iflag;  ����ģʽ��־
		* unsigned short c_oflag;  	���ģʽ��־
		* unsigned short c_cflag;  	����ģʽ��־
		* unsigned short c_lflag; 	����ģʽ��־�򱾵�ģʽ��־��ֲ�ģʽ
		* unsigned char c_line; 	�п���line discipline 
		* unsigned char c_cc[NCC];  �����ַ�����,���п��Ը��ĵ������ַ�
		*/
		int ret_val;
		struct termios options;
		sp_param_linux& base_param = _sp_param;
		sp_prop& spp = _sp_param._spp;

		// 1. to read props of this port
		//----------------------------------------------------------------------------------------
		ret_val = tcgetattr(base_param._fd, &options);

		// failed to read prop
		if (ret_val < 0)
		{
			ret_val = -1;
			return ret_val;
		}

		// 2. to set options
		//----------------------------------------------------------------------------------------
		speed_t baud = base_param._spp._baud_rate;// sp_params_check::get_instance().get_baud(base_param._spp._baud_rate);
		cfsetispeed(&options, baud);
		cfsetospeed(&options, baud);

		// 3.data bits
		//----------------------------------------------------------------------------------------
		switch (base_param._spp._data_bits)
		{
		case lib_sp::data_bits_5:
			options.c_cflag &= ~CSIZE;
			options.c_cflag |= CS5;
			break;

		case lib_sp::data_bits_6:
			options.c_cflag &= ~CSIZE;
			options.c_cflag |= CS6;
			break;

		case lib_sp::data_bits_7:
			options.c_cflag &= ~CSIZE;
			options.c_cflag |= CS7;
			break;

		case lib_sp::data_bits_8:
			options.c_cflag &= ~CSIZE;
			options.c_cflag |= CS8;
			break;

		default:
			break;
		}

		// 4. stop bits
		//----------------------------------------------------------------------------------------
		switch (base_param._spp._stop_bits)
		{
		case lib_sp::stop_1:
			options.c_cflag &= ~CSTOPB;
			break;

		case lib_sp::stop_1_5:
			options.c_cflag &= ~CSTOPB;
			break;

		case lib_sp::stop_2:
			options.c_cflag &= ~CSTOPB;
			break;

		default:
			break;
		}

		// 5. flow control
		//----------------------------------------------------------------------------------------
		switch (base_param._spp._flow_ctl)
		{
		case lib_sp::flow_ctl_none:
			options.c_cflag &= ~CRTSCTS;
			break;

		case lib_sp::flow_ctl_hardware:
			options.c_cflag |= CRTSCTS;
			break;

		case lib_sp::flow_ctl_software:
			options.c_cflag |= IXON | IXOFF | IXANY;
			break;

		default:
			break;
		}

		// 6. parity
		//----------------------------------------------------------------------------------------
		switch (base_param._spp._parity)
		{
		case lib_sp::parity_none:
			options.c_cflag &= ~PARENB;		
			options.c_cflag &= ~INPCK;		
			break;

		case lib_sp::parity_odd:
			options.c_cflag |= PARENB;		
			options.c_cflag |= PARODD;		
			options.c_cflag |= INPCK;		
			options.c_cflag |= ISTRIP;		
			break;

		case lib_sp::parity_even:
			options.c_cflag |= PARENB;		
			options.c_cflag &= ~PARODD;		
			options.c_cflag |= INPCK;		
			options.c_cflag |= ISTRIP;		
			break;

		case lib_sp::parity_space:
			options.c_cflag &= ~PARENB;		
			options.c_cflag &= ~CSTOPB;		
			break;


		default:
			break;
		}

		
		options.c_oflag &= ~OPOST;				
			
		// ??????????????
		options.c_lflag &= ~(ICANON		|		
								ECHO	|		
								ECHOE	|		
								ISIG);			

		options.c_iflag &= ~(BRKINT		|		
								ICRNL	|		
								INPCK	|		
								ISTRIP	|		
								IXON);			


		options.c_cc[VTIME]		= 0;			
		options.c_cc[VMIN]		= 1;			

		tcflush(base_param._fd, TCIFLUSH);

		// to set config of serial port
		ret_val = tcsetattr(base_param._fd, TCSANOW, &options);

		// 4. set configure failed
		if (0 > ret_val)
		{
			// 
			ret_val = -2;
		}

		return ret_val;
	}

	/**
	*	@brief:
	*/
	bool serial_port_linux_base::start_thread_recv()
	{
		sp_linux_thread& thread_param			= _sp_param._thread;
		set_thread_recv_data_is_running(true);
		int ret									= pthread_create(&thread_param._thread_recv, NULL, thread_recv_data_monitor, (void*)this);
		
		return (0 > ret) ? false : true;
	}

	/**
	*	@brief:
	*/
	int serial_port_linux_base::stop_thread_recv()
	{
		sp_linux_thread& thread_param = _sp_param._thread;
		set_thread_recv_data_is_running(false);

		pthread_join(thread_param._thread_recv, NULL);

		thread_param._thread_recv = invalid_value;

		return 0;
	}

	/**
	*	@brief:
	*/
	void * serial_port_linux_base::thread_recv_data_monitor(void * param)
	{
		serial_port_linux_base* sp_obj = reinterpret_cast<serial_port_linux_base*>(param);

		if (NULL == sp_obj || nullptr == sp_obj)
		{
			pthread_exit(NULL);
		}
		else
		{
			sp_param_linux& sp_param		= sp_obj->get_sp_param_linux();
			sp_linux_thread& thread_param	= sp_param._thread;
			sp_linux_others& others			= sp_param._others;
			char buf_read[lib_sp::len_buf_1024]		= { 0 };
			int has_read_bytes				= 0;

			

			while (sp_obj->get_thread_recv_data_is_running())
			{

				ioctl(sp_param._fd, FIONREAD, &has_read_bytes);

				// its too long�� passed
				if (lib_sp::len_buf_1024 < has_read_bytes)
					continue;


				// notify the callback function
				if (has_read_bytes >= others._min_byte_read_notify)
				{
					sp_obj->log(" 222222222222222222 notify the callback function......");
					sp_obj->lock();

					if (sp_obj->is_opened())
					{
						sp_obj->log(" 33333333333333333333 notify the callback function......");
						/*
						����ֵ���ɹ����ض�ȡ���ֽ�������������-1������errno������ڵ�read֮ǰ�ѵ����ļ�ĩβ�������read����0
						*/
						int ret_read = ::read(sp_param._fd, buf_read, has_read_bytes);
						if ( 0 < ret_read)
						{
							sp_obj->log(" 444444444444444444444444444444 notify the callback function......");
							if (thread_param._precv_data)
								thread_param._precv_data->on_recv_data(buf_read, ret_read);
						}
						else
						{
							// an error occurred, err_id = errno
							;
						}

						// reset buffer
						memset(buf_read, 0, len_buf_1024);
					}
					else
					{
						// it doesnt open
					}

					sp_obj->unlock();
				}
				else
				{
					// continue to prepare data before reading
				}

			} // end while
		} // sp_obj is not null

		sp_obj->log(" 555555555555555555555555 its over......");
		pthread_exit(NULL);
	} // end thread recv func

}


#endif // has_cxx_11
#endif //!os_is_linux
