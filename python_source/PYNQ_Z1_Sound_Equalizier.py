import tkinter as tk
from Sound_Equalizier import *
from UART_Port import *

# создаем основное окно 
root = tk.Tk()
root.title('PYNQ Z1 Sound_Equalizier')

# создаем и настраиваем widget управления UART соединением
UART_Connect = UART_Port(root)
UART_Connect.get_frame().pack(side = 'right')

# создаем и настраиваем widget управления LEDs
Equalizier = Sound_Equalizier(root, UART_Connect)
Equalizier.get_frame().pack(side = 'right')

root.mainloop()
