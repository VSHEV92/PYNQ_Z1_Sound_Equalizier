-------------------------------------------
------ Демодулятор ШИМ сигнала ------------
-------------------------------------------

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

entity PWM_Decoder is
    Port ( CLK           : in  STD_LOGIC;                      -- тактовый сигнал
           RESET_N       : in  STD_LOGIC;                      -- синхронный сброс, активный уровень '0'
           PWM_DATA      : in  STD_LOGIC;                      -- входные данные с ШИМ модуляцией
           DECODED_DATA  : out STD_LOGIC_VECTOR (7 downto 0);  -- демодулированные данные
           DECODED_VALID : out STD_LOGIC                       -- строб сигнал для демодулированных данных
           );
end PWM_Decoder;

architecture Behavioral of PWM_Decoder is

constant Clk_Cycles_per_Semple : integer := 100;

signal Decoded_Value : integer range -Clk_Cycles_per_Semple to Clk_Cycles_per_Semple;
signal Clk_Cycles : integer range 0 to Clk_Cycles_per_Semple;

begin

process(CLK)
begin
    if rising_edge(CLK) then
        if RESET_N = '0' then
            Clk_Cycles <= 0;
            Decoded_Value <= 0;
            DECODED_DATA <= (others => '0');
            DECODED_VALID <= '0';
        else
            DECODED_VALID <= '0';
            Clk_Cycles <= Clk_Cycles + 1;
            if PWM_DATA = '0' then
                Decoded_Value <= Decoded_Value - 1;
            else
                Decoded_Value <= Decoded_Value + 1;
            end if;
            -- если посчитали Clk_Cycles_per_Semple тактов, выдаем результат
            if Clk_Cycles = (Clk_Cycles_per_Semple - 1) then
                Clk_Cycles <= 0;
                Decoded_Value <= 0;
                DECODED_DATA <= STD_LOGIC_VECTOR(TO_SIGNED(Decoded_Value, 8));
                DECODED_VALID <= '1';
            end if; 
        end if;
    end if;
end process;



end Behavioral;
