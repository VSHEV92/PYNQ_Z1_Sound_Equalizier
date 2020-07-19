
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity mux is
    Port ( data_0   : in  STD_LOGIC_VECTOR (7 downto 0);
           data_1   : in  STD_LOGIC_VECTOR (7 downto 0);
           sel      : in  STD_LOGIC;
           data_out : out STD_LOGIC_VECTOR (7 downto 0));
end mux;

architecture Behavioral of mux is

begin

data_out <= data_0 when sel = '0' else data_1;

end Behavioral;
