cmake_minimum_required(VERSION 3.7)
project(ButtonInterface C ASM)

set(PROG_TYPE usbtiny)
#set(PROG_ARGS -c usbtiny -p m32u4 -U flash:w:${OUTPUT_FILE_HEX})

set(MCU atmega32u4)
set(F_CPU 16000000UL)

add_definitions(-DF_CPU=${F_CPU})
add_definitions(-DBOARD=BOARD_LEONARDO)
add_definitions(-DUSE_LUFA_HEADER)
add_definitions(-DF_USB=${F_CPU})
add_definitions(-DUSE_EXTERNAL_INTERRUPT)

set(AVRCPP avr-g++)
set(AVRC avr-gcc)
set(AVRSTRIP avr-strip)
set(OBJCOPY avr-objcopy)
set(OBJDUMP avr-objdump)
set(AVRSIZE avr-size)
set(AVRDUDE avrdude)

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_CXX_COMPILER ${AVRCPP})
set(CMAKE_C_COMPILER ${AVRC})
set(CMAKE_ASM_COMPILER ${AVRC})

set(CSTANDARD "-std=gnu99")
set(CDEBUG "-gstabs -g -ggdb")
set(CWARN     "-Wall -Wextra -Wstrict-prototypes -Wl,--gc-sections -Wl,--relax")
set(CTUNING   "-funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums -ffunction-sections -fdata-sections -fno-strict-aliasing -fno-jump-tables")
set(COPT      "-Os -lm -lprintf_flt")
set(CMCU      "-mmcu=${MCU}")

set(BASE_PATH "${${PROJECT_NAME}_SOURCE_DIR}")
set(INC_PATH "${BASE_PATH}/inc")
set(SRC_PATH "${BASE_PATH}/src")

include_directories(${INC_PATH})
set(SOURCE ${SRC_PATH}/LUFA/Descriptors.c
    ${SRC_PATH}/mcp23017.c
    ${SRC_PATH}/midi.c
    ${SRC_PATH}/i2cmaster.S
    ${SRC_PATH}/LUFA/CDCClassDevice.c
    ${SRC_PATH}/LUFA/Device_AVR8.c
    ${SRC_PATH}/LUFA/EndpointStream_AVR8.c
    ${SRC_PATH}/LUFA/Endpoint_AVR8.c
    ${SRC_PATH}/LUFA/Template_Endpoint_Control_R.c
    ${SRC_PATH}/LUFA/Template_Endpoint_Control_W.c
    ${SRC_PATH}/LUFA/Template_Endpoint_RW.c
    ${SRC_PATH}/LUFA/Template_Pipe_RW.c
    ${SRC_PATH}/LUFA/USBController_AVR8.c
    ${SRC_PATH}/LUFA/USBInterrupt_AVR8.c
    ${SRC_PATH}/LUFA/ConfigDescriptors.c
    ${SRC_PATH}/LUFA/DeviceStandardReq.c
    ${SRC_PATH}/LUFA/Events.c
    ${SRC_PATH}/LUFA/USBTask.c
    ${SRC_PATH}/VirtualSerial.c)

set(CFLAGS   "${CMCU} ${CDEBUG} ${CDEFS} ${COPT} ${CWARN} ${CSTANDARD} ${CTUNING}")

set(CMAKE_C_FLAGS   "${CFLAGS}")
set(CMAKE_ASM_FLAGS   "${CFLAGS}")

include_directories(${INC_PATH} ${LIB_INC_PATH})
add_executable(${PROJECT_NAME} ${SOURCE})
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "${PROJECT_NAME}.elf")

# Compiling targets
add_custom_target(strip ALL     ${AVRSTRIP} "${PROJECT_NAME}.elf" DEPENDS ${PROJECT_NAME})
add_custom_target(hex   ALL     ${OBJCOPY} -R .eeprom -O ihex "${PROJECT_NAME}.elf" "${PROJECT_NAME}.hex" DEPENDS strip)
add_custom_target(eeprom        ${OBJCOPY} -j .eeprom --change-section-lma .eeprom=0 -O ihex "${PROJECT_NAME}.elf" "${PROJECT_NAME}.eeprom" DEPENDS strip)
add_custom_target(disassemble   ${OBJDUMP} -S "${PROJECT_NAME}.elf" > "${PROJECT_NAME}.lst" DEPENDS strip)
# Flashing targets
add_custom_target(flash         ${AVRDUDE} -c ${PROG_TYPE} -p ${MCU} ${PROG_ARGS} -U flash:w:${PROJECT_NAME}.hex DEPENDS hex)
add_custom_target(flash_usbtiny ${AVRDUDE} -c usbtiny -p ${MCU} -U flash:w:${PROJECT_NAME}.hex DEPENDS hex)
add_custom_target(flash_usbasp  ${AVRDUDE} -c usbasp -p ${MCU} -U flash:w:${PROJECT_NAME}.hex DEPENDS hex)
add_custom_target(flash_ardisp  ${AVRDUDE} -c avrisp -p ${MCU} -b 19200 -P ${USBPORT} -U flash:w:${PROJECT_NAME}.hex DEPENDS hex)
add_custom_target(flash_109     ${AVRDUDE} -c avr109 -p ${MCU} -b 9600 -P ${USBPORT} -U flash:w:${PROJECT_NAME}.hex DEPENDS hex)
add_custom_target(flash_eeprom  ${AVRDUDE} -c ${PROG_TYPE} -p ${MCU} ${PROG_ARGS} -U eeprom:w:${PROJECT_NAME}.hex DEPENDS eeprom)
# Fuses (For ATMega328P-PU, Calculated using http://eleccelerator.com/fusecalc/fusecalc.php?chip=atmega328p)
add_custom_target(reset         ${AVRDUDE} -c ${PROG_TYPE} -p ${MCU} ${PROG_ARGS} -e)
add_custom_target(fuses_1mhz    ${AVRDUDE} -c ${PROG_TYPE} -p ${MCU} ${PROG_ARGS} -U lfuse:w:0x62:m)
add_custom_target(fuses_8mhz    ${AVRDUDE} -c ${PROG_TYPE} -p ${MCU} ${PROG_ARGS} -U lfuse:w:0xE2:m)
add_custom_target(fuses_16mhz   ${AVRDUDE} -c ${PROG_TYPE} -p ${MCU} ${PROG_ARGS} -U lfuse:w:0xFF:m)
add_custom_target(fuses_uno     ${AVRDUDE} -c ${PROG_TYPE} -p ${MCU} ${PROG_ARGS} -U lfuse:w:0xFF:m -U hfuse:w:0xDE:m -U efuse:w:0x05:m)
add_custom_target(set_eeprom_save_fuse   ${AVRDUDE} -c ${PROG_TYPE} -p ${MCU} ${PROG_ARGS} -U hfuse:w:0xD1:m)
add_custom_target(clear_eeprom_save_fuse ${AVRDUDE} -c ${PROG_TYPE} -p ${MCU} ${PROG_ARGS} -U hfuse:w:0xD9:m)
# Utilities targets
add_custom_target(avr_terminal  ${AVRDUDE} -c ${PROG_TYPE} -p ${MCU} ${PROG_ARGS} -nt)

set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "${PROJECT_NAME}.hex;${PROJECT_NAME}.eeprom;${PROJECT_NAME}.lst")

# Config logging
message("* ")
message("* Project Name:\t${PROJECT_NAME}")
message("* Project Source:\t${SRC_PATH}")
message("* Project Include:\t${INC_PATH}")
message("* Library Include:\t${LIB_INC_PATH}")
message("* ")
message("* Project Source Files:\t${SOURCE}")
message("* Library Source Files:\t${LIB_SRC_FILES}")
message("* ")
message("* C Flags:\t${CMAKE_C_FLAGS}")
message("* ")
message("* CXX Flags:\t${CMAKE_C_FLAGS}")
message("* ")
