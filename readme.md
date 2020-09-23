## 1.About  
---
  * lib_serial_port is an library to operate serial port, which is written by c++11. .It is convenient to read and write data to the serial port after openning. And also, it supports to query the information of available serial port.  
  * to receive data, this library creates a thread to do that.  
  * If an error occured, it will output the error information to the log file using spdlog library
  * Now, it only works on windows.  


## 2. Directory & File 
  the following is the description of this library  
```
.
│   CMakeLists.txt		# cmake file
│   del.bat				# to delete middle objects of Visual Studio produced 
│   readme.md			# readme written by markdown
│   update-log.txt		# the log of this project 
│   
├───build				
│       del.bat			# to delete middle objects of Visual Studio produced 
├───config
│       config.h.in		# definite some macro
│       
├───example		
│   └───shared			# shared demo
│           main.cc 	# source file
│           
├───ext					# other project, like spdlog, fmt
├───include				# header files' directory
│   │   universe_def.h	# definite some macro
│   │   utils.h			# a helper to switch basic type, like int -> std::string
│   │   
│   └───serial_port		# the serial port's header files
│           serial_port_interface.h		# the serial port's interface
│           serial_port_win_base.h		# definite some class to oprator serial port on windows platform	
│           serial_port_win_info.h		# include some interfaces to get serial port's information on windows platform  
├───other_tools
│   │   VSPDP_9349.zip			# Virtual Serial Port Driver Pro 
│   │   
│   └───AccessPort_23021		# Access port
│           
└───src		# the source files of serial port library
        serial_port_interface.cpp	# the source file of interface file
        serial_port_win_base.cpp	# the source file of serial_port_win_base.h
        serial_port_win_info.cpp	# the implemention of serial_port_win_base class
        utils.cpp					# the implemention of helper class
```


## 3. Configure & Generate 
  * You need **[CMake](https://cmake.org/)**, whose version is greater than 3.14.  
  * Cmd  
```
$ cd xx/serial_port/build
$ cmake ..
```
  then, you will get the serial port's project files. And also, you could use the following cmd to **compile** your program:  
```
cmake --build .. --config Release
```


## 4. Usage  
  If you wanna receive data, you should definite anther class to inherit **irecv_data** class and implement the **on_recv_data** function. Thus, you could get the data from serial port in the **on_recv_data** function.  

> ### 4,1 include file:
```
#include "serial_port/serial_port_interface.h"
```
> ### 4.2 use namespace **lib_sp**:  
```
using namespace lib_sp;
```
> ### 4.2 initialize 
```
serial_port_prop spp;
spp._name = std::string("COM" + to_string(sp_id));
int ret_val = sp->init(spp);
```
> ### 4.3 open  
```
int ret_val = sp->open();
```
> ### 4.4 send data
```
const char arr_send[] = {"123ABC-=+"};
int ret_val = sp->send(arr_send, sizof(arr_send));
```
> ### 4.5 to check if opened
```
bool is_open = sp->is_opened();
```
> ### 4.6 close serial port
```
int ret_val = sp->close();
```
> ### 4.7 to get information of library 
```
std::string str_ver = sp->get_version();
cout << "version = " << str_ver.c_str() << "\n\n"; // 1.0.0.18-09-2020
```
> ### 4.8 to receive data
  Definite another class inherited **irecv_data** class to receive data, Like this:  
```
class serial_port : public irecv_data
{
// ...
}
```
  then ,set the second parameter of **init** to **this** :  
```
// if you wanna recv data, set the second param this, otherwise, set it nullptr
_psp->init(spp, this);
```

> ### 4.9 to create an library object  
  You could call **sp_create_win**(in the **serial_port_interface.h**) function to create an object
```
iserial_port	*_psp = sp_create_win();
```
> ### 4.10 to release an object
  You must call **sp_release**  to release the object created by **sp_create_win** function.
```
sp_release(_psp);
```


## 5. Example  
  You could get further information to use it from `xx/serial_port/example/shared/main.cc`  


## 6. Other tools  
  **Note**  These tools only works on **Windows**
  If you dont have any physical serial port, **Configure Virtual Serial Port Driver** could  create more available virtual serial port pairs. **Note**, virtual serial port must be used in pairs. Like **10** and **20**, **11** and **21**   
  > ### 6.1 configure serial port
  use **Configure Virtual Serial Port Driver** to add and delete serial port pairs  
![Image](https://github.com/mohistH/lib_sp/blob/master/other_tools/sp_vsp.png)
  > ### 6.2 Access port  
  This tool could monitor and communicate with serial port.  
![Image](https://github.com/mohistH/lib_sp/blob/master/other_tools/sp_acceport.png)


## 7 License  
**[3-Clause BSD](https://opensource.org/licenses/BSD-3-Clause)**  
**[spdlog-License](https://github.com/gabime/spdlog/blob/v1.x/LICENSE)**

