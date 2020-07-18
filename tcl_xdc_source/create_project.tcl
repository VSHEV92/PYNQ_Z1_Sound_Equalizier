# если проект уже существует, удаляем его
if {[file exist ../PYNQ_Z1_Sound_Equalizer]} {
    file delete -force ../PYNQ_Z1_Sound_Equalizer
}

# создаем проект и указываем board file
create_project PYNQ_Z1_Sound_Equalizer ../PYNQ_Z1_Sound_Equalizer -part xc7z020clg400-1
set_property board_part www.digilentinc.com:pynq-z1:part0:1.0 [current_project]

# настраиваем ip cache каталого
config_ip_cache -import_from_project -use_cache_location ../ip_cache
update_ip_catalog

# добавляем xdc files с constraints
add_files -fileset constrs_1 -norecurse LOC.xdc
add_files -fileset constrs_1 -norecurse Debug.xdc

# добавляем rtl files
add_files -norecurse ../hdl_source/PWM_Decoder.vhd
add_files -norecurse ../hdl_source/PWM_Encoder.vhd
add_files -fileset sim_1 ../hdl_source/PWM_Codec_tb.vhd

# создаем block design, добавляем zynq и делаем block automation
create_bd_design "zynq_bd"
create_bd_cell -type ip -vlnv xilinx.com:ip:processing_system7:5.5 processing_system7_0
apply_bd_automation -rule xilinx.com:bd_rule:processing_system7 -config {make_external "FIXED_IO, DDR" apply_board_preset "1" Master "Disable" Slave "Disable" }  [get_bd_cells processing_system7_0]

# удаляем всю переферию, которая добавлена по умолчанию
set_property -dict [list CONFIG.PCW_USE_M_AXI_GP0 {0} CONFIG.PCW_EN_CLK0_PORT {0} CONFIG.PCW_EN_RST0_PORT {0} CONFIG.PCW_QSPI_PERIPHERAL_ENABLE {0} CONFIG.PCW_QSPI_GRP_SINGLE_SS_ENABLE {1} CONFIG.PCW_ENET0_PERIPHERAL_ENABLE {0} CONFIG.PCW_SD0_PERIPHERAL_ENABLE {0} CONFIG.PCW_UART0_PERIPHERAL_ENABLE {0} CONFIG.PCW_USB0_PERIPHERAL_ENABLE {0} CONFIG.PCW_GPIO_MIO_GPIO_ENABLE {0}] [get_bd_cells processing_system7_0]

# добавляем UART_0 и устанавливаем baud rate в 9600
set_property -dict [list CONFIG.PCW_UART0_BAUD_RATE {9600} CONFIG.PCW_UART0_PERIPHERAL_ENABLE {1} CONFIG.PCW_UART0_UART0_IO {MIO 14 .. 15}] [get_bd_cells processing_system7_0]

# добавляем тактовый сигнал для микрофона
set_property -dict [list CONFIG.PCW_FPGA0_PERIPHERAL_FREQMHZ {2.4} CONFIG.PCW_EN_CLK0_PORT {1}] [get_bd_cells processing_system7_0]
make_bd_pins_external  [get_bd_pins processing_system7_0/FCLK_CLK0]
set_property NAME phone_clk [get_bd_ports /FCLK_CLK0_0]

# добавляем сигнал сброса для PL
set_property -dict [list CONFIG.PCW_EN_RST0_PORT {1}] [get_bd_cells processing_system7_0]

# добаляем SD MIO для для работы с SD картой
set_property -dict [list CONFIG.PCW_SD0_PERIPHERAL_ENABLE {1} CONFIG.PCW_SD0_GRP_CD_ENABLE {1} CONFIG.PCW_SD0_GRP_CD_IO {MIO 47}] [get_bd_cells processing_system7_0]

# добавляем rtl блоки в block design
create_bd_cell -type module -reference PWM_Decoder PWM_Decoder_0
create_bd_cell -type module -reference PWM_Encoder PWM_Encoder_0

# добавляем внешние порты для микрофона и ФНЧ фильтра
create_bd_port -dir I phone_data
create_bd_port -dir O PWM_Sound
create_bd_port -dir O Mute_Sound

# подключение сигналов rtl блоков
connect_bd_net [get_bd_ports phone_data]           [get_bd_pins PWM_Decoder_0/PWM_DATA]
connect_bd_net [get_bd_pins PWM_Decoder_0/RESET_N] [get_bd_pins processing_system7_0/FCLK_RESET0_N] 
connect_bd_net [get_bd_pins PWM_Decoder_0/CLK]     [get_bd_pins processing_system7_0/FCLK_CLK0]

connect_bd_net [get_bd_pins PWM_Encoder_0/CLK]     [get_bd_pins processing_system7_0/FCLK_CLK0]
connect_bd_net [get_bd_pins PWM_Encoder_0/RESET_N] [get_bd_pins processing_system7_0/FCLK_RESET0_N]
connect_bd_net [get_bd_ports PWM_Sound]            [get_bd_pins PWM_Encoder_0/PWM_DATA]

# временные соединения
create_bd_cell -type ip -vlnv xilinx.com:ip:xlconstant:1.1 xlconstant_0
connect_bd_net [get_bd_ports Mute_Sound] [get_bd_pins xlconstant_0/dout]
connect_bd_net [get_bd_pins PWM_Encoder_0/INPUT_DATA] [get_bd_pins PWM_Decoder_0/DECODED_DATA]

# проверяем, сохраняем и закрываем block design
validate_bd_design
regenerate_bd_layout
save_bd_design
close_bd_design [get_bd_designs zynq_bd]

# создаем hdl_wrapper
make_wrapper -files [get_files ../PYNQ_Z1_Sound_Equalizer/PYNQ_Z1_Sound_Equalizer.srcs/sources_1/bd/zynq_bd/zynq_bd.bd] -top
add_files -norecurse ../PYNQ_Z1_Sound_Equalizer/PYNQ_Z1_Sound_Equalizer.srcs/sources_1/bd/zynq_bd/hdl/zynq_bd_wrapper.v

