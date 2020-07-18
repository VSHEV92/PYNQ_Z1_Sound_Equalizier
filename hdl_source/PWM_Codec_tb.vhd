library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

entity PWM_Codec_tb is
end PWM_Codec_tb;

architecture Behavioral of PWM_Codec_tb is

component PWM_Encoder is
    Port ( CLK          : in  STD_LOGIC;                      -- тактовый сигнал
           RESET_N      : in  STD_LOGIC;                      -- синхронный сброс, активный уровень '0'
           INPUT_DATA   : in  STD_LOGIC_VECTOR (7 downto 0);  -- входные данные
           PWM_DATA     : out STD_LOGIC                       -- выходные данные с ШИМ модуляцией        
           );
end component;

component PWM_Decoder is
    Port ( CLK          : in  STD_LOGIC;                      -- тактовый сигнал
           RESET_N      : in  STD_LOGIC;                      -- синхронный сброс, активный уровень '0'
           PWM_DATA     : in  STD_LOGIC;                      -- входные данные с ШИМ модуляцией
           DECODED_DATA : out STD_LOGIC_VECTOR (7 downto 0)   -- демодулированные данные
           );
end component;

constant clk_period : time := 416.666667 ns;
constant clk_cycles_per_sample : integer := 100;
constant indata_min : integer := -80;
constant indata_max : integer := 80;

signal indata_val   : integer;
signal clk          : STD_LOGIC := '0';
signal reset_n      : STD_LOGIC := '0';
signal pwm_data     : STD_LOGIC;
signal input_data   : STD_LOGIC_VECTOR (7 downto 0);  
signal decoded_data : STD_LOGIC_VECTOR (7 downto 0);  

begin

clk <= not clk after clk_period;

reset_proc: process
begin
    wait for 4 us;
    reset_n <= '1';
    wait;
end process;

indata_proc: process
begin
    for i in indata_min to indata_max loop 
        indata_val <= i;
        input_data <= STD_LOGIC_VECTOR(TO_SIGNED(indata_val, 8));
        wait for clk_period * clk_cycles_per_sample;
    end loop; 
end process;

Encoder: PWM_Encoder
Port Map( CLK          => clk,
          RESET_N      => reset_n,
          INPUT_DATA   => input_data,
          PWM_DATA     => pwm_data      
         );

Decoder: PWM_Decoder
Port Map( CLK          => clk,
          RESET_N      => reset_n,
          PWM_DATA     => pwm_data,
          DECODED_DATA => decoded_data      
         );


end Behavioral;
