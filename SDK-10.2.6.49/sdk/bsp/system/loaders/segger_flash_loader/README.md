Brief Description {#segger_flash_loader}
================================================================================

The project is used to generate SEGGER's Open Flashloader firmware. There are 
two (2) build configurations for each combination of device family and memory 
controller type:

- xxx-00_yyy_Debug_RAM
- xxx-00_yyy_Release

where: 

- xxx: Device Family
- yyy: Type of memory controller
- QFLASH: Flash connected to Quad SPI controller
- OQFLASH: Flash connected to Octa SPI controller
- EFLASH: Embedded Flash

The Debug build configuration allows to debug the flash algorithm during development. 
The configuration includes a `main.c` containing the typical function call order, 
executed by the J-Link DLL during flash programming. It behaves as a typical RAM 
build configuration. 

The Release build configuration does not allow debugging yet creates the output 
`*.elf` file which can be referenced from within the JLinkDevices.xml file as 
"Loader". 

For more info refer to [Open Flashloader wiki page](https://wiki.segger.com/Open_Flashloader).

Prerequisites
================================================================================
 1. Compile the Release build configuration of the corresponding device. Please
    note that DA1470X device family supports two flash devices, OQFLASH/QFLASH. 
	
 2. Make sure that the corresponding `segger_flash_loader.elf` files have been 
    built under the paths:
	
	- DA1469X: 
		* QFLASH:  ~/sdk10/binaries/segger_flash_loader.elf
	- DA1470X: 
		* OQFLASH: ~/sdk10/binaries/da1470x/oflash/segger_flash_loader.elf
		* QFLASH:  ~/sdk10/binaries/da1470x/qflash/segger_flash_loader.elf
		
 3. Make sure that an appropriate JLink version is installed and selected in 
    Smart Snippets Studio.
	
 4. Make sure that the valid XML Tags and Attributes have been added to the  
    `JLinkDevices.xml` so that the SEGGER tools can use the `segger_flash_loader.elf`
	properly: 
	
	- ~/sdk10/config/segger/DA1469x/JLinkDevices.xml
	- ~/sdk10/config/segger/DA1470x/JLinkDevices.xml

__DA1469x__
~~~~{.xml}
<DataBase>
  <!--                 -->
  <!-- DIALOG (DA1469x)-->
  <!--                 -->
  <Device>
    <ChipInfo Vendor="Dialog Semiconductor"   Name="DA1469x" WorkRAMAddr="0x810000" WorkRAMSize="0x10000" Core="JLINK_CORE_CORTEX_M33" />
    <FlashBankInfo Name="MX25U3225F" BaseAddr="0x36000000" MaxSize="0x2000000" Loader="../../../binaries/segger_flash_loader.elf" LoaderType="FLASH_ALGO_TYPE_OPEN" />
  </Device>
</DataBase>
~~~~
__DA1470x__
~~~~{.xml}
<DataBase>
  <!--                 -->
  <!-- DIALOG (DA1470x)-->
  <!--                 -->
  <Device>
    <!-- WorkRAM as declared in sections.ld -->
    <ChipInfo Vendor="Dialog Semiconductor"   Name="DA1470x" WorkRAMAddr="0x10010000" WorkRAMSize="0x00040000" Core="JLINK_CORE_CORTEX_M33" />
    <FlashBankInfo Name="OQSPIC Flash" BaseAddr="0x38000000" MaxSize="0x8000000" Loader="../../../binaries/da1470x/oflash/segger_flash_loader.elf" LoaderType="FLASH_ALGO_TYPE_OPEN" />
    <FlashBankInfo Name="QSPIC Flash" BaseAddr="0x48000000" MaxSize="0x8000000" Loader="../../../binaries/da1470x/qflash/segger_flash_loader.elf" LoaderType="FLASH_ALGO_TYPE_OPEN" />
  </Device>
</DataBase>
~~~~
_Note: In fact only the first step is necessary to be done in order for the user
       to setup the SEGGER Flash loader. The steps 2 - 4 are useful just for 
	   validating that everything has been configured properly_

Integrating JLinkDevices.xml on SDK
================================================================================
For easier maintenance the SDK integrates the `JLinkDevices.xml` using the  J-Link 
command `-JLinkDevicesXMLPath <path to JLinkDevices.xml>` according to 
[SEGGER's suggestion](https://forum.segger.com/index.php/Thread/4209-Add-a-new-flash-device-to-the-JLInkDevices-database/?postID=15141#post15141).
This command allows a SmartSnippets launcher to flash a device by making use 
of `segger_flash_loader.elf` functionality. 

The user can use one of the following launchers to flash and debug an application
built for a non-volatile memory:

	- DA1469X: 
		* There is no launcher for 69X family. Please, refer to the next section 
		  `Setting up SEGGER J-Link tools` for this purpose. 
	- DA1470X: 
		* OQFLASH: OQSPI_DA1470x_segger_flasher
		* QFLASH:  There is no launcher for the flash which is connected to QSPIC.
		           Likewise with 69X family refer to the next section for this purpose.
				   
To have a better understanding how the JLinkDevices.xml is integrated by a 
launcher open the `Debugger` Tab of one of them, for instance the 
`OQSPI_DA1470x_segger_flasher` and have a look at the field `Other options:` 
where the next attribute is included:

`-JLinkDevicesXMLPath ${workspace_loc}/config/segger/DA1470x/JLinkDevices.xml`

_IMPORTANT NOTE_

In order to build an application capable to be flashed with Segger Flash Loader 
it is necessary to define `dg_configUSE_SEGGER_FLASH_LOADER=1`. Otherwise the 
binary file will not include the corresponding product headers, image headers 
etc, causing the booter to get stuck and the application will not boot.

Moreover, keep in mind that, in case of DA1469X, the linker script is only 
applicable to the default flash memory, i.e. MX25U3225F. If you need to build an 
application for another memory, you need to modify the linker script so that the
proper product headers etc. are included in the binary file. Otherwise, the
application will not boot for the same reason as described above.

Setting up SEGGER J-Link tools
================================================================================
An alternative to flash the non-volatile memories is by using the SEGGER J-Link
Tools. To make that possible you need to follow the next steps, assuming that 
the SEGGER J-Link is installed under the path `C:\Program Files (x86)\SEGGER\JLink_V640`:

- Copy the corresponding `segger_flash_loader.elf` files under the following path: 
  
	- DA1469X: 
		* QFLASH:  `C:\Program Files (x86)\SEGGER\JLink_V640\Devices\Dialog\da1469x\qflash\segger_flash_loader.elf`
	- DA1470X: 
		* OQFLASH: `C:\Program Files (x86)\SEGGER\JLink_V640\Devices\Dialog\da1470x\oflash\segger_flash_loader.elf`
		* QFLASH:  `C:\Program Files (x86)\SEGGER\JLink_V640\Devices\Dialog\da1470x\qflash\segger_flash_loader.elf` 
  
- Update the `C:\Program Files (x86)\SEGGER\JLink_V640\JLinkDevices.xml` by 
  adding the following between `<DataBase>` and `</DataBase>`

__DA1469x__
~~~~{.xml}
  <!--                 -->
  <!-- DIALOG (DA1469x)-->
  <!--                 -->
  <Device>
    <ChipInfo Vendor="Dialog Semiconductor"   Name="DA1469x" WorkRAMAddr="0x810000" WorkRAMSize="0x10000" Core="JLINK_CORE_CORTEX_M33" />
    <FlashBankInfo Name="MX25U3225F" BaseAddr="0x36000000" MaxSize="0x2000000" Loader="Devices/Dialog/da1469x/qflash/segger_flash_loader.elf" LoaderType="FLASH_ALGO_TYPE_OPEN" />
  </Device>

__DA1470x__
~~~~{.xml}
  <!--                 -->
  <!-- DIALOG (DA1470x)-->
  <!--                 -->
  <Device>
    <ChipInfo Vendor="Dialog Semiconductor"   Name="DA1470x" WorkRAMAddr="0x10010000" WorkRAMSize="0x00040000" Core="JLINK_CORE_CORTEX_M33" />
    <FlashBankInfo Name="Code Flash" BaseAddr="0x38000000" MaxSize="0x8000000" Loader="Devices/Dialog/da1470x/oflash/segger_flash_loader.elf" LoaderType="FLASH_ALGO_TYPE_OPEN" />
	<FlashBankInfo Name="QSPIC1 (Storage Flash)" BaseAddr="0x48000000" MaxSize="0x8000000" Loader="Devices/Dialog/da1470x/qflash/segger_flash_loader.elf" LoaderType="FLASH_ALGO_TYPE_OPEN" />
  </Device>
~~~~

- To verify quickly whether the setup configuration is valid you can flash a 
  binary to a device using the `JFlashLite.exe`:
  
  * Run `JFlashLite.exe`
  * Choose Device, for instance DA1470X and press OK
  * Choose the binary file and set the `Prog. addr. (bin file only)` with the 
    physical address of the memory you want to program, for instance if this is 
	the OQSPI Flash of DA1470X set 0x38000000, if the QSPI Flash of DA1470X set
	0x48000000 and so on, then press the button `Program Device`

