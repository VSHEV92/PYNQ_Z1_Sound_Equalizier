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

# добавляем репозиторий с HLS IP ядром
set_property  ip_repo_paths  ../hls_source [current_project]
update_ip_catalog

# добавляем xdc files с constraints
add_files -fileset constrs_1 -norecurse LOC.xdc
add_files -fileset constrs_1 -norecurse Timing.xdc
add_files -fileset constrs_1 -norecurse Debug.xdc

# добавляем rtl files
add_files -norecurse ../hdl_source/mux.vhd
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
set_property -dict [list CONFIG.PCW_FPGA1_PERIPHERAL_FREQMHZ {100} CONFIG.PCW_EN_CLK1_PORT {1}] [get_bd_cells processing_system7_0]
make_bd_pins_external  [get_bd_pins processing_system7_0/FCLK_CLK0]
set_property NAME phone_clk [get_bd_ports /FCLK_CLK0_0]

# добавляем сигнал сброса для PL
set_property -dict [list CONFIG.PCW_EN_RST0_PORT {1}] [get_bd_cells processing_system7_0]

# добаляем SD MIO для для работы с SD картой
set_property -dict [list CONFIG.PCW_SD0_PERIPHERAL_ENABLE {1} CONFIG.PCW_SD0_GRP_CD_ENABLE {1} CONFIG.PCW_SD0_GRP_CD_IO {MIO 47}] [get_bd_cells processing_system7_0]

# добаляем GPIO master port у Zynq
set_property -dict [list CONFIG.PCW_USE_M_AXI_GP0 {1}] [get_bd_cells processing_system7_0]

# добаляем AXI GPIO
create_bd_cell -type ip -vlnv xilinx.com:ip:axi_gpio:2.0 axi_gpio_0
set_property -dict [list CONFIG.C_GPIO_WIDTH {8} CONFIG.C_ALL_INPUTS {1}] [get_bd_cells axi_gpio_0]

create_bd_cell -type ip -vlnv xilinx.com:ip:axi_gpio:2.0 axi_gpio_1
set_property -dict [list CONFIG.C_GPIO_WIDTH {8} CONFIG.C_GPIO2_WIDTH {1} CONFIG.C_IS_DUAL {1} CONFIG.C_ALL_OUTPUTS {1} CONFIG.C_ALL_OUTPUTS_2 {1}] [get_bd_cells axi_gpio_1]

apply_bd_automation -rule xilinx.com:bd_rule:axi4 -config { Clk_master {Auto} Clk_slave {Auto} Clk_xbar {Auto} Master {/processing_system7_0/M_AXI_GP0} Slave {/axi_gpio_0/S_AXI} ddr_seg {Auto} intc_ip {New AXI Interconnect} master_apm {0}}  [get_bd_intf_pins axi_gpio_0/S_AXI]

apply_bd_automation -rule xilinx.com:bd_rule:axi4 -config { Clk_master {Auto} Clk_slave {Auto} Clk_xbar {Auto} Master {/processing_system7_0/M_AXI_GP0} Slave {/axi_gpio_1/S_AXI} ddr_seg {Auto} intc_ip {New AXI Interconnect} master_apm {0}}  [get_bd_intf_pins axi_gpio_1/S_AXI]

# добавляем rtl блоки в block design
create_bd_cell -type module -reference PWM_Decoder PWM_Decoder_0
create_bd_cell -type module -reference PWM_Encoder PWM_Encoder_0
create_bd_cell -type module -reference mux mux_0

# добавляем внешние порты для микрофона и ФНЧ фильтра
create_bd_port -dir I phone_data
create_bd_port -dir O PWM_Sound
create_bd_port -dir O Mute_Sound

# подключение сигналов rtl блоков
connect_bd_net [get_bd_ports phone_data]           [get_bd_pins PWM_Decoder_0/PWM_DATA]
connect_bd_net [get_bd_pins PWM_Decoder_0/CLK]     [get_bd_pins processing_system7_0/FCLK_CLK0]
connect_bd_net [get_bd_pins PWM_Decoder_0/RESET_N] [get_bd_pins rst_ps7_0_2M/peripheral_aresetn]

connect_bd_net [get_bd_pins PWM_Encoder_0/CLK]     [get_bd_pins processing_system7_0/FCLK_CLK0]
connect_bd_net [get_bd_ports PWM_Sound]            [get_bd_pins PWM_Encoder_0/PWM_DATA]
connect_bd_net [get_bd_pins PWM_Encoder_0/RESET_N] [get_bd_pins rst_ps7_0_2M/peripheral_aresetn]

# добавляем константный сигнал равный '1'
create_bd_cell -type ip -vlnv xilinx.com:ip:xlconstant:1.1 xlconstant_0
connect_bd_net [get_bd_ports Mute_Sound] [get_bd_pins xlconstant_0/dout]

# добавляем fifo для реализации CDC
create_bd_cell -type ip -vlnv xilinx.com:ip:fifo_generator:13.2 fifo_generator_0

set_property -dict [list CONFIG.Fifo_Implementation {Independent_Clocks_Block_RAM} CONFIG.Input_Data_Width {8} CONFIG.Input_Depth {16} CONFIG.Output_Data_Width {8} CONFIG.Output_Depth {16} CONFIG.Reset_Pin {false} CONFIG.Reset_Type {Asynchronous_Reset} CONFIG.Full_Flags_Reset_Value {0} CONFIG.Use_Dout_Reset {false} CONFIG.Valid_Flag {true} CONFIG.Data_Count_Width {4} CONFIG.Write_Data_Count_Width {4} CONFIG.Read_Data_Count_Width {4} CONFIG.Full_Threshold_Assert_Value {13} CONFIG.Full_Threshold_Negate_Value {12} CONFIG.Enable_Safety_Circuit {false}] [get_bd_cells fifo_generator_0]

copy_bd_objs /  [get_bd_cells {fifo_generator_0}]

# добавляем sound equalizer и настраиваем соединения
create_bd_cell -type ip -vlnv xilinx.com:hls:Sound_Equalizier:1.0 Sound_Equalizier_0

apply_bd_automation -rule xilinx.com:bd_rule:axi4 -config { Clk_master {/processing_system7_0/FCLK_CLK0 (2 MHz)} Clk_slave {/processing_system7_0/FCLK_CLK1 (100 MHz)} Clk_xbar {/processing_system7_0/FCLK_CLK0 (2 MHz)} Master {/processing_system7_0/M_AXI_GP0} Slave {/Sound_Equalizier_0/s_axi_ctrl} ddr_seg {Auto} intc_ip {/ps7_0_axi_periph} master_apm {0}}  [get_bd_intf_pins Sound_Equalizier_0/s_axi_ctrl]

disconnect_bd_net /processing_system7_0_FCLK_CLK1 [get_bd_pins rst_ps7_0_100M/slowest_sync_clk]
connect_bd_net [get_bd_pins rst_ps7_0_100M/slowest_sync_clk] [get_bd_pins processing_system7_0/FCLK_CLK0]

connect_bd_net [get_bd_pins fifo_generator_0/dout] [get_bd_pins Sound_Equalizier_0/in_data_V]
connect_bd_net [get_bd_pins fifo_generator_0/valid] [get_bd_pins Sound_Equalizier_0/in_data_V_ap_vld]

connect_bd_net [get_bd_pins Sound_Equalizier_0/out_data_V] [get_bd_pins fifo_generator_1/din]
connect_bd_net [get_bd_pins Sound_Equalizier_0/out_data_V_ap_vld] [get_bd_pins fifo_generator_1/wr_en]

delete_bd_objs [get_bd_cells rst_ps7_0_100M]
delete_bd_objs [get_bd_nets rst_ps7_0_100M_peripheral_aresetn]
connect_bd_net [get_bd_pins Sound_Equalizier_0/ap_rst_n] [get_bd_pins rst_ps7_0_2M/peripheral_aresetn]
connect_bd_net [get_bd_pins ps7_0_axi_periph/M02_ARESETN] [get_bd_pins rst_ps7_0_2M/peripheral_aresetn]

# подключение сигналов mux
connect_bd_net [get_bd_pins PWM_Decoder_0/DECODED_DATA] [get_bd_pins mux_0/data_0]
connect_bd_net [get_bd_pins axi_gpio_1/gpio2_io_o] [get_bd_pins mux_0/sel]
connect_bd_net [get_bd_pins axi_gpio_1/gpio_io_o] [get_bd_pins mux_0/data_1]
connect_bd_net [get_bd_pins axi_gpio_0/gpio_io_i] [get_bd_pins PWM_Decoder_0/DECODED_DATA]

# подключение сигналов fifo
connect_bd_net [get_bd_pins fifo_generator_0/wr_clk] [get_bd_pins processing_system7_0/FCLK_CLK0]
connect_bd_net [get_bd_pins fifo_generator_0/rd_clk] [get_bd_pins processing_system7_0/FCLK_CLK1]
connect_bd_net [get_bd_pins fifo_generator_0/rd_en] [get_bd_pins xlconstant_0/dout]
connect_bd_net [get_bd_pins mux_0/data_out] [get_bd_pins fifo_generator_0/din]
connect_bd_net [get_bd_pins PWM_Decoder_0/DECODED_VALID] [get_bd_pins fifo_generator_0/wr_en]

connect_bd_net [get_bd_pins fifo_generator_1/wr_clk] [get_bd_pins processing_system7_0/FCLK_CLK1]
connect_bd_net [get_bd_pins fifo_generator_1/rd_clk] [get_bd_pins processing_system7_0/FCLK_CLK0]
connect_bd_net [get_bd_pins fifo_generator_1/rd_en] [get_bd_pins xlconstant_0/dout]
connect_bd_net [get_bd_pins fifo_generator_1/dout] [get_bd_pins PWM_Encoder_0/INPUT_DATA]
connect_bd_net [get_bd_pins fifo_generator_1/valid] [get_bd_pins PWM_Encoder_0/INPUT_VALID]

# проверяем, сохраняем и закрываем block design
validate_bd_design
regenerate_bd_layout
save_bd_design
close_bd_design [get_bd_designs zynq_bd]

# создаем hdl_wrapper
make_wrapper -files [get_files ../PYNQ_Z1_Sound_Equalizer/PYNQ_Z1_Sound_Equalizer.srcs/sources_1/bd/zynq_bd/zynq_bd.bd] -top
add_files -norecurse ../PYNQ_Z1_Sound_Equalizer/PYNQ_Z1_Sound_Equalizer.srcs/sources_1/bd/zynq_bd/hdl/zynq_bd_wrapper.v

