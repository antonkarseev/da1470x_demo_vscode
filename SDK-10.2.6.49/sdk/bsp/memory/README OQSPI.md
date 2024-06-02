OQSPI Flash Support {#flash_support}
================================================================================

## Overview
This document describes the oqspi_automode.{c h} API, which is responsible for 
supporting the XiP flash memory connected to OQSPI controller (OQSPIC) in DA1470x 
SDK platform. Moreover, the procedure of adding support for a non officially 
supported flash memory is described.

## API file tree

The file tree structure of the oqspi_automode API is shown below: 

```
sdk 
│
└───memory
|   |   README OQSPI.md 
│   │
│   └───include
│       │   oqspi_automode.h
│       │   oqspi_common.h
│       │   oqspi_<manufacturer>_<bus_mode>.h
│       │   oqspi_<manufacturer_prefix><memory_name>.h
│       │   
│       src
│       │   oqspi_automode.c
```

where: 

- **<manufacturer>:** The name of the manufacturer, e.g. Macronix, Winbond, 
  Adesto, Gigadevice etc.
- **<bus_mode>**: The bus mode of the memory, e.g. quad, octa.
- **<manufacturer_prefix>**: A prefix that determines the memory manufacturer, 
  e.g. mx (Macronix), w (Winbond), at (Adesto), gd (Gigadevice).
- **<memory_name>**: The memory name.

### Files description
- The **oqspi_common.h** contains definitions of common macros and structures.
- The header files **oqspi_<manufacturer>_<bus_mode>.h** contain all the common 
  macros and functions used by a specific group of memories, e.g. 
  **the oqspi_macronix_octa.h** contains common code for octa bus Macronix flash 
  memories.
- The files **oqspi_<manufacturer_prefix><memory_name>.h** are the flash memories
  drivers, which contain all needed settings and callback functions in order for 
  the API to be configured properly for a specific memory.

## API configuration options 

### OQSPI flash memory detection
The API supports two compilation options regarding whether the connected XiP flash 
memory, will be **Pre-configured** at compile time or **Auto-detected** at runtime:

- **Pre-configured**: `dg_configOQSPI_FLASH_AUTODETECT` = 0 (default)
- **Auto-detected**: `dg_configOQSPI_FLASH_AUTODETECT` = 1

**Warning**: This option should NOT be confused with the auto/manual mode of 
operation of the OQSPI controller.

#### Pre-configured
If the **Pre-configured** option is enabled, the connected flash memory is  
explicitly configured at compile-time by setting the next compilation options:

- **dg_configOQSPI_FLASH_HEADER_FILE**
- **dg_configOQSPI_FLASH_CONFIG**

For instance:

````
#define dg_configOQSPI_FLASH_HEADER_FILE        "oqspi_mx66um1g45g.h"
#define dg_configOQSPI_FLASH_CONFIG             oqspi_mx66um1g45g_cfg
````

On top of the aforementioned options, it's the optional one
`dg_configOQSPI_FLASH_CONFIG_VERIFY` (disabled by default), which determines 
whether the jedec ID of the connected memory will be read and verified during 
system startup. This option is very useful for debugging purposes, yet not 
recommended for production builds, since it adds an extra overhead.

#### Auto-detected
If the **Auto-detected** option is chosen, the flash memory is detected at runtime
by reading and detecting its JEDEC id, provided that the connected flash memory
is one of the SDK officially supported memories.

**Warning 1**: The **Auto-detected** option significantly increases the code size 
and the retained SYSRAM usage, hence it is **NOT recommended for production builds**.

**Warning 2**: If the **Auto-detected** option is enabled in OQSPI build, it has
to be validated, that the initialization sequence determined by the product header
is compatible with all flash memories which must be supported by the application.
This sequence is used by the bootrom in order to initialize the OQSPIC, and this 
configuration is used up to the point that the OQSPIC is reconfigured by the 
oqspi_automode API for high performance based on the flash driver settings. 
If the initialization sequence is not compatible with a memory and it is flashed 
on it, the application will end up to a fault before getting out from bootrom.

**Warning 3**: To support a non officially supported flash memory the API must 
be built with the `dg_configUNDISCLOSED_UNSUPPORTED_FLASH_DEVICES` defined.

### OQSPI flash drivers
The OQSPI memory drivers are header files located under `sdk/bsp/memory`, which
contain an instance of the OQSPI flash configuration structure `oqspi_flash_config_t`, 
as well as a set of callback and helper functions. The `oqspi_flash_config_t` is
defined in `oqspi_common.h` and determines the required settings of the OQSPIC 
for a specific memory.

- **jedec.manufacturer_id**: The JEDEC manufacturer ID.
- **jedec.type**: The JEDEC device type.
- **jedec.density**: The JEDEC device density.
- **jedec.density_mask**: The JEDEC device density mask, which is used to mask 
  the device density reading, if needed. If not, must be set 0xFF.

- **size_mbits**: The memory size in MBits.
- **address_size**: The address size of the commands, which can be either 24bits 
  or 32bits length.
- **clk_mode**: 
- **opcode_len**: The length of the command opcode, which can be either 1 byte 
  or 2 bytes length. If the latter applies, the second byte of the opcode equals 
  to the inverted value of the first one.

- **read_instr_cfg.opcode_bus_mode**: The bus mode of the opcode phase of the 
  reading command.
- **read_instr_cfg.addr_bus_mode**: The bus mode of the address phase of the 
  reading command.
- **read_instr_cfg.extra_byte_bus_mode**: The bus mode of the extra byte phase 
  of the reading command. This phase is used in order to determine whether the 
  next reading command will be in continuous mode of operation or not. When the 
  continuous mode of operation is enabled, the opcode is sent only on the first 
  reading and is omitted on all subsequent reading commands. 
- **read_instr_cfg.dummy_bus_mode**: The bus mode of the dummy bytes phase of 
  the reading command.
- **read_instr_cfg.data_bus_mode**: The bus mode of the data phase of the
  reading command.
- **read_instr_cfg.continuous_mode**: Enable/disable the continuous mode of 
  operation. If enabled, the `read_instr_cfg.extra_byte_cfg` must be enabled as 
  well, and the `read_instr_cfg.extra_byte_value` must be set according to the 
  datasheet of the memory in order to maintain the continuous mode of operation.
- **read_instr_cfg.extra_byte_cfg**: Enable/disable the extra byte phase.
- **read_instr_cfg.extra_byte_half_cfg**: Enable/disable transmitting only the 
  high nibble of the extra byte, meaning that the output is switched to high-z
  during the transmission of the low nibble. Not applicable in octa bus mode, 
  where it must always remain disabled. This setting is very rarely enabled 
  in general.
- **read_instr_cfg.opcode**: The opcode of the reading command.
- **read_instr_cfg.extra_byte_value**: The value of the byte sent during
  the extra byte phase provided that the `read_instr_cfg.extra_byte_cfg` is 
  enabled. If the continuous mode of operation is enabled, set this field 
  according to the memory. If disabled, is usually set to 0xFF yet many other
  values could serve the same scope.
- **read_instr_cfg.cs_high_delay_nsec**: The minimum required delay (in nsec) 
  that the CS signal has to stay at idle state between two consecutive read 
  commands.

- **erase_instr_cfg.opcode_bus_mode**: The bus mode of the opcode phase of the 
  erasing command.
- **erase_instr_cfg.addr_bus_mode**: The bus mode of the address phase of the 
  erasing command.
- **erase_instr_cfg.hclk_cycles**: The delay in terms of AMBA hclk clock cycles
  without flash memory reading requests before performing an erase or erase 
  resume command.
- **erase_instr_cfg.opcode**: The opcode of the erasing command. 
- **erase_instr_cfg.cs_high_delay_nsec**: The minimum required delay (in nsec) 
  that the CS signal has to stay at idle state between a Write Enable, Erase, 
  Erase Suspend or Erase Resume command and the next consecutive command.

- **read_status_instr_cfg.opcode_bus_mode**: The bus mode of the opcode phase of 
  the read status register command.
- **read_status_instr_cfg.receive_bus_mode**: The bus mode of the receive phase 
  of the read status register command.
- **read_status_instr_cfg.dummy_bus_mode**: The bus mode of the dummy bus phase 
  of the read status register command.
- **read_status_instr_cfg.dummy_value**: The value that is transferred on the 
  OQSPI bus during the dummy cycles phase. In fact, the value of the dummy bytes
  is out of interest. However, there are some octa flash memories (e.g. Macronix)
  which require an address phase, during which the transmitted data must equal to 
  0x00. Since the OQSPIC does not support a dedicated address phase for the read 
  status register command, this option allows forcing the dummy bytes value to 
  0x00 achieving the same result. The flash driver `oqspi_mx66um1g45g.h` is an
  indicative example.
- **read_status_instr_cfg.busy_level**: The level where the busy bit (low or high)
  is considered as busy.
- **read_status_instr_cfg.busy_pos**: The position of the busy bit in the status
  register.
- **read_status_instr_cfg.dummy_bytes**: The number of the dummy bytes of the  
  read status register command. It should not been confused with the dummy bytes
  of read command.
- **read_status_instr_cfg.opcode**: The opcode of the read status command. 
- **read_status_instr_cfg.delay_nsec**: The minimum delay in nsec between the 
  read status register command and the previous erase command. Usually is not 
  needed, thus is set 0.
  
- **write_enable_instr_cfg.opcode_bus_mode**: The bus mode of the opcode phase 
  of the write enable command.
- **write_enable_instr_cfg.opcode**: The opcode of write enable command.

- **page_program_instr_cfg.opcode_bus_mode**: The bus mode of the opcode phase 
  of the page program command.
- **page_program_instr_cfg.addr_bus_mode**: The bus mode of the address phase 
  of the page program command.
- **page_program_instr_cfg.data_bus_mode**: The bus mode of the data phase 
  of the page program command.
- **page_program_instr_cfg.opcode**: The opcode of page program command.

- **suspend_resume_instr_cfg.suspend_bus_mode**: The bus mode of the opcode 
  phase of the suspend command.
- **suspend_resume_instr_cfg.resume_bus_mode**: The bus mode of the opcode 
  phase of the resume command.
- **suspend_resume_instr_cfg.suspend_opcode**: The opcode of the suspend command.
- **suspend_resume_instr_cfg.resume_opcode**: The opcode of the resume command.
- **suspend_resume_instr_cfg.suspend_latency_usec**: The minimum required latency 
  in usec to suspend an erase operation. The next consecutive erase resume command 
  cannot be issued before this delay has elapsed.
- **suspend_resume_instr_cfg.resume_latency_usec**: The minimum required latency
  in usec to resume an erase operation. Once the resume command is issued, the
  currently suspended erase operation resumes within this time.
- **suspend_resume_instr_cfg.res_sus_latency_usec**: The minimum required latency 
  in usec between an erase resume and the next consequent erase suspend command.

- **exit_continuous_mode_instr_cfg.opcode_bus_mode**: The bus mode of the opcode 
  phase of the exit from continuous mode command.
- **exit_continuous_mode_instr_cfg.sequence_len**: The number of the bytes needed 
  to be shifted out, in order for the flash memory to get out of continuous mode. 
  Once the memory is switched to continuous mode, whether the next read command 
  will be sent in continuous mode or not, in other words without opcode or with,
  is determined by the value of the extra byte. The extra byte is sent immediately
  after the address phase, which means that the the number of the bytes must be 
  at least 4 or 5, for 24 bit and 32 bit address size respectively. Any higher 
  amount of bytes work fine as well, yet in will cause a redundant overhead. 
- **exit_continuous_mode_instr_cfg.disable_second_half**: Disable the output 
  during the second half of the sequence. Not applicable in octa bus mode.
- **exit_continuous_mode_instr_cfg.opcode**: The opcode of the exit from 
  continuous mode command.

- **delay.reset_usec**: The minimum required delay in usec after a reset sequence.
- **delay.power_down_usec**: The minimum required delay between an enter power 
  down sequence and the moment the memory enters the power down mode.
- **delay.release_power_down_usec**: The minimum required delay between a release 
  power down sequence and the moment the memory exits from power down mode.
- **delay.power_up_usec**: The minimum required power up delay.

- **callback.initialize_cb**: Flash memory initialization callback function which 
  is called at system startup.
- **callback.sys_clk_cfg_cb**: This function is called every time the system
  clock changes in order to re-configure the OQSPIC.  
- **callback.exit_opi_qpi_cb**: Callback function which exits the flash memory 
  from OPI/QPI mode for octa/quad flash memories respectively.   
- **callback.get_dummy_bytes_cb**: Callback function which returns the number 
  of the dummy bytes for any given system clock.
- **callback.is_suspended_cb**: Callback function which checks whether the flash 
  erase operation is suspended. 
- **callback.is_busy_cb**: Callback function which checks whether the flash 
  memory is busy.
- **callback.read_status_reg_cb**: Callback function to read the status register
  of the memory.
- **callback.write_status_reg_cb**: Callback function to write the status register
  of the memory.

### Adding support for a new OQSPI flash memory
The SDK provides support for a specific set of Octa and Quad bus flash memories. 
Each of those memories has a dedicated OQSPI flash driver, which resides on its 
own header file. In order to add support for a new memory, a new driver must be 
implemented. It is recommended to use as a starting point either the header file 
of an already existing driver or the template header file `oqspi_xxx.h`. If the 
new memory belongs to an already supported group of memories, the macros and the 
functions from `oqspi_<manufacturer>_<bus_mode>.h` files can be used in order to 
eliminate the development effort. 

The following steps are usually needed to create a new flash driver:

1. Copy and rename either an existing driver header file or the template header 
   file `oqspi_xxx.h`. It is recommended to follow the naming convention 
   `oqspi_<manufacturer_prefix><memory_name>.h`.

2. Rename the prefixes of all functions and variables accordingly. All drivers 
   reside in the same name-space, therefore the names of all function and variable 
   must be unique.

3. Set all JEDEC ID fields (ID, type and density) according to the datasheet.

4. Set the size of the memory in MBits, the address size and the opcode length:
	- size_mbits
	- address_size
	- opcode_len

5. Set the opcode and the bus mode of all phases of all next commands 
  (instructions) according to the datasheet:
   
   - READ command 
		- read_instr_cfg.opcode
		- read_instr_cfg.opcode_bus_mode
		- read_instr_cfg.addr_bus_mode
		- read_instr_cfg.extra_byte_bus_mode
		- read_instr_cfg.dummy_bus_mode
		- read_instr_cfg.data_bus_mode
		
   - ERASE SECTOR command
		- erase_instr_cfg.opcode 
		- erase_instr_cfg.opcode_bus_mode
		- erase_instr_cfg.addr_bus_mode
		
   - READ STATUS command
		- read_status_instr_cfg.opcode
		- read_status_instr_cfg.opcode_bus_mode
		- read_status_instr_cfg.receive_bus_mode
		- read_status_instr_cfg.dummy_bus_mode
	
   - WRITE ENABLE command
		- write_enable_instr_cfg.opcode
		- write_enable_instr_cfg.opcode_bus_mode
	
   - PAGE PROGRAM command
		- page_program_instr_cfg.opcode
		- page_program_instr_cfg.opcode_bus_mode
		- page_program_instr_cfg.addr_bus_mode
		- page_program_instr_cfg.data_bus_mode
		
   - ERASE SUSPEND & ERASE RESUME commands
		- suspend_resume_instr_cfg.suspend_opcode
		- suspend_resume_instr_cfg.resume_opcode
		- suspend_resume_instr_cfg.suspend_bus_mode
		- suspend_resume_instr_cfg.resume_bus_mode

6. Set all next timing parameters according to the datasheet:

   - read_instr_cfg.cs_idle_delay_nsec
   - erase_instr_cfg.cs_idle_delay_nsec
   - suspend_resume_instr_cfg.suspend_latency_usec
   - suspend_resume_instr_cfg.resume_latency_usec
   - suspend_resume_instr_cfg.res_sus_latency_usec
   - delay.reset_usec
   - delay.power_down_usec
   - delay.release_power_down_usec
   - delay.power_up_usec

The name of the timing parameters may vary between different manufacturers, even
between different memory variants of the same manufacturers. The next table  
indicates the names of these parameters according to the datasheets of the 
supported memories and can be used as a reference point.

| manufacturer 	| Read cs_idle_delay 	| Erase cs_idle_delay 	| suspend_latency 	| resume_latency 	| res_sus_latency 	|            reset            	| power_down 	| release_power_down 	|  power_up  	|
|:------------:	|:------------------:	|:-------------------:	|:---------------:	|:--------------:	|:---------------:	|:---------------------------:	|:----------:	|:------------------:	|:----------:	|
|   Macronix   	|    tSHSL (read)    	|    tSHSL (erase)    	|       tESL      	|        (*)      	|       tERS      	| tREADY2  (4KB Sector Erase) 	|     tDP    	|        tRES1       	|    tVSL    	|
|    Winbond   	|       tSHSL1       	|        tSHSL2       	|       tSUS      	|        (*)       	|       tSUS      	|             tRST            	|     tDP    	|        tRES1       	|    tVSL    	|
|  Gigadevice  	|        tSHSL       	|        tSHSL        	|       tSUS      	|        (*)       	|       tRS       	|            tRST_E           	|     tDP    	|        tRES1       	|    tVSL    	|
|    Adesto    	|        tCSH        	|         tCSH        	|       tSUS      	|     (*)/tRES     	|       tSUS       	|         tRST/tSWRST         	|  tDP/tEDPD 	|     tRES1/tRDPD    	| tVSL/tVCSL 	|

(*) This latency is usually not mentioned in the AC characteristics table of the 
datasheet. Please, read carefully the description of the erase suspend/resume
commands for more information.

7. Set properly all next settings regarding the continuous mode of operation.  
   The continuous mode is also named by some memory manufacturers as performance 
   enhanced mode or as burst mode. When this feature is enabled, the opcode is 
   sent only once during the first read command, and then all subsequent read 
   commands are issued without it. The OQSPI flash memory enters and stays in 
   continuous mode, as long as the value of the extra byte phase maintains a 
   specific pattern, which is memory dependant. In order to get the memory 
   out from continuous mode this pattern must be violated (the value 0xFF is 
   usually used, which works for all supported flash memories).

    - read_instr_cfg.continuous_mode
    - read_instr_cfg.extra_byte_cfg
    - read_instr_cfg.extra_byte_half_cfg
	- read_instr_cfg.extra_byte_value

    - exit_continuous_mode_instr_cfg.opcode_bus_mode
    - exit_continuous_mode_instr_cfg.sequence_len
    - exit_continuous_mode_instr_cfg.disable_second_half
	
	Two indicative reference points on how to enable or not the continuous mode 
	are the drivers of the Winbond W25Q64JWIM (`oqspi_w25q64jwim.h`) and Macronix 
	MX66UM1G45G (`oqspi_mx66um1g45g.h`) respectively.

8. Implement and set all next callback functions. If the new memory belongs to  
   an already supported group of memories consider using the already implemented 
   callback and helper functions from `oqspi_<manufacturer>_<bus_mode>.h`. Even
   if the callbacks of the new flash driver has to be diffentiated, parts of the
   aforementioned functions could be used to eliminate the effort. 

	- callback.initialize_cb: Make sure that all needed register fields are set
	  properly. Refer to the datasheet for more information. Typical configurations 
	  set by this function: Bus Mode, Dummy Bytes, Address Size, Slew Rate, 
	  Current Strength etc.
	
    - callback.sys_clk_cfg_cb: Reconfigure the dummy bytes and any other parameter 
	  of the flash memory based on the new system clock.
	
	- callback.exit_opi_qpi_cb: Issue the right sequence in order to exit the 
	  memory from OPI/QPI mode.
	
	- callback.get_dummy_bytes_cb: Return the dummy bytes for any given system 
	  clock. This function is used by initialize_cb() and sys_clk_cfg_cb() in 
	  order to set the dummy bytes properly.
	  
	- callback.is_suspended_cb: Implement the right sequence in order to check 
	  whether an Erase or Page Program operation is suspended.
	
	- callback.is_busy_cb: Implement the right sequence in order to check whether
      the memory is busy or not.	
	
	- callback.read_status_reg_cb: Implement the right sequence in order to read
	  the status register.
	
	- callback.write_status_reg_cb: Implement the right sequence in order to write
	  the status register.
