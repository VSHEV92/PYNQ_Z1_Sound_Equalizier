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
           INPUT_VALID  : in  STD_LOGIC;                      -- строб сигнал входных данных
           PWM_DATA     : out STD_LOGIC                       -- выходные данные с ШИМ модуляцией        
           );
end component;

component PWM_Decoder is
    Port ( CLK           : in  STD_LOGIC;                      -- тактовый сигнал
           RESET_N       : in  STD_LOGIC;                      -- синхронный сброс, активный уровень '0'
           PWM_DATA      : in  STD_LOGIC;                      -- входные данные с ШИМ модуляцией
           DECODED_DATA  : out STD_LOGIC_VECTOR (7 downto 0);  -- демодулированные данные
           DECODED_VALID : out STD_LOGIC                       -- строб сигнал для демодулированных данных
           );
end component;

constant clk_period : time := 416.666667 ns;
constant clk_cycles_per_sample : integer := 100;
constant indata_min : integer := -80;
constant indata_max : integer := 80;

signal indata_val   : integer := 0;
signal clk          : STD_LOGIC := '0';
signal reset_n      : STD_LOGIC := '0';
signal pwm_data     : STD_LOGIC;
signal input_data   : STD_LOGIC_VECTOR (7 downto 0);  
signal decoded_data : STD_LOGIC_VECTOR (7 downto 0);  
signal decoded_valid : STD_LOGIC;
signal sigma_delta_out  : STD_LOGIC := '0';
signal sigma_delta_integr   : integer := 0;

signal decoded_data_2 : STD_LOGIC_VECTOR (7 downto 0);  
signal decoded_valid_2 : STD_LOGIC;

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

sigma_delta_mod: process
begin
    if sigma_delta_out = '1' then
        sigma_delta_integr <= sigma_delta_integr + indata_val - indata_max;
    else
        sigma_delta_integr <= sigma_delta_integr + indata_val - indata_min;
    end if;
    
    if sigma_delta_integr > 0 then
        sigma_delta_out <= '1';
    else
        sigma_delta_out <= '0';
    end if;
    wait for clk_period;
end process;

Decoder: PWM_Decoder
Port Map( CLK           => clk,
          RESET_N       => reset_n,
          PWM_DATA      => sigma_delta_out,
          DECODED_DATA  => decoded_data,
          DECODED_VALID => decoded_valid     
         );

Encoder: PWM_Encoder
Port Map( CLK          => clk,
          RESET_N      => reset_n,
          INPUT_DATA   => decoded_data,
          INPUT_VALID  => decoded_valid,
          PWM_DATA     => pwm_data      
         );

Decoder_2: PWM_Decoder
Port Map( CLK           => clk,
          RESET_N       => reset_n,
          PWM_DATA      => pwm_data,
          DECODED_DATA  => decoded_data_2,
          DECODED_VALID => decoded_valid_2     
         );

end Behavioral;
