-------------------------------------------
------------- ШИМ Модулятор ---------------
-------------------------------------------

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

entity PWM_Encoder is
    Port ( CLK          : in  STD_LOGIC;                      -- тактовый сигнал
           RESET_N      : in  STD_LOGIC;                      -- синхронный сброс, активный уровень '0'
           INPUT_DATA   : in  STD_LOGIC_VECTOR (7 downto 0);  -- входные данные
           INPUT_VALID  : in  STD_LOGIC;                      -- строб сигнал входных данных
           PWM_DATA     : out STD_LOGIC                       -- выходные данные с ШИМ модуляцией        
           );
end PWM_Encoder;

architecture Behavioral of PWM_Encoder is

constant Clk_Cycles_per_Semple : integer := 100;

signal Input_Data_Reg : integer range -Clk_Cycles_per_Semple to Clk_Cycles_per_Semple;
signal Accum_Value : integer range -Clk_Cycles_per_Semple to Clk_Cycles_per_Semple;
signal Clk_Cycles : integer range 0 to Clk_Cycles_per_Semple;

begin

process(CLK)
begin
    if rising_edge(CLK) then
        if RESET_N = '0' then
            Clk_Cycles <= 0;
            Accum_Value <= -Clk_Cycles_per_Semple;
            Input_Data_Reg <= 0;
            PWM_DATA <= '0';
        else
            Accum_Value <= Accum_Value + 2;
             
            if Accum_Value < Input_Data_Reg then
                PWM_DATA <= '1';
            else
                PWM_DATA <= '0';
            end if;
             
            if INPUT_VALID = '1' then 
                Accum_Value <= -Clk_Cycles_per_Semple;
                Input_Data_Reg <= TO_INTEGER(SIGNED(INPUT_DATA));
            end if;

        end if;
    end if;
end process;



end Behavioral;
