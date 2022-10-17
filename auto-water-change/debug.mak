#Generated by VisualGDB (http://visualgdb.com)
#DO NOT EDIT THIS FILE MANUALLY UNLESS YOU ABSOLUTELY NEED TO
#USE VISUALGDB PROJECT PROPERTIES DIALOG INSTEAD

BINARYDIR := Debug

#Additional flags
PREPROCESSOR_MACROS := DEBUG=1 USE_HAL_DRIVER STM32F103xB
INCLUDE_DIRS := ../User_code/flash ../User_code/hmi ../User_code/init ../User_code/uart ../User_code/ir ..\Core\Inc ..\Drivers\STM32F1xx_HAL_Driver\Inc ..\Drivers\STM32F1xx_HAL_Driver\Inc\Legacy ..\Drivers\CMSIS\Device\ST\STM32F1xx\Include ..\Drivers\CMSIS\Include ..\Middlewares\Third_Party\FreeRTOS\Source\include ..\Middlewares\Third_Party\FreeRTOS\Source\CMSIS_RTOS_V2 ..\Middlewares\Third_Party\FreeRTOS\Source\portable\GCC\ARM_CM3
LIBRARY_DIRS := 
LIBRARY_NAMES := 
ADDITIONAL_LINKER_INPUTS := 
MACOS_FRAMEWORKS := 
LINUX_PACKAGES := 

CFLAGS := -ggdb -ffunction-sections -O0
CXXFLAGS := -ggdb -ffunction-sections -fno-exceptions -fno-rtti -O0
ASFLAGS := 
LDFLAGS := -Wl,-gc-sections
COMMONFLAGS := 
LINKER_SCRIPT := C:\Users\lices\Desktop\auto-water-change\STM32F103C8TX_FLASH.ld

START_GROUP := -Wl,--start-group
END_GROUP := -Wl,--end-group

#Additional options detected from testing the toolchain
USE_DEL_TO_CLEAN := 1
CP_NOT_AVAILABLE := 1

ADDITIONAL_MAKE_FILES := stm32.mak
GENERATE_BIN_FILE := 1
GENERATE_IHEX_FILE := 0
