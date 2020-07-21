import tkinter as tk
from tkinter import ttk
import serial

class UART_Port:
    def Start_Button_Pressed(self):
        # обработчик нажатия Start_Button
        self.Serial_Port.port = self.Port_Comb_var.get()
        self.Serial_Port.baudrate = int(self.BaudRate_var.get())
        self.Serial_Port.bytesize = int(self.DataBits_var.get())
        self.Serial_Port.stopbits = int(self.StopBits_var.get())

        if self.Parity_var.get() == 'None':
            self.Serial_Port.parity = 'N'
        elif self.Parity_var.get() == 'Even':
            self.Serial_Port.parity = 'E'
        else:
            self.Serial_Port.parity = 'O'

        if self.Serial_Port.is_open:
            self.Serial_Port.close()
        self.Serial_Port.open()
        self.Start_Button['state'] = 'disabled'
        self.Stop_Button['state'] = '!disabled'
            
    def Stop_Button_Pressed(self):
        # обработчик нажатия Stop_Button
        self.Serial_Port.close()
        self.Start_Button['state'] = '!disabled'
        self.Stop_Button['state'] = 'disabled'
        
    def __init__(self, master):
        # создаем Frame
        self.f = ttk.LabelFrame(master, text = 'UART Connection', width = 20, height = 50)

        # создаем ComboBox для настройки соединения UART
        self.Port_Comb_var = tk.StringVar()
        self.Port_Comb_var.set('/dev/ttyUSB0')
        self.Port_Comb = ttk.Combobox(self.f, width = 13, textvariable = self.Port_Comb_var,
                                      values=["/dev/ttyUSB0", "/dev/ttyUSB1","COM1", "COM2", "COM3", "COM4", "COM5"])
        ttk.Label(self.f, text = 'Port Name:').grid(padx = 5, row = 0, column = 0)
        self.Port_Comb.grid(pady = 4, padx = 5, row = 0, column = 1)

        self.BaudRate_var = tk.StringVar()
        self.BaudRate_var.set('9600')
        self.BaudRate_Comb = ttk.Combobox(self.f, width = 13, state = 'readonly', textvariable = self.BaudRate_var,
                                          values=['9600', '14400', '19200', '28800', '31250', '38400', '57600', '115200'])
        ttk.Label(self.f, text = 'Buad Rate:').grid(padx = 5, row = 1, column = 0)
        self.BaudRate_Comb.grid(pady = 4, padx = 5, row = 1, column = 1)

        self.DataBits_var = tk.StringVar()
        self.DataBits_var.set('8')
        self.DataBits_Comb = ttk.Combobox(self.f, width = 13, state = 'readonly', textvariable = self.DataBits_var,
                                          values=['5', '6', '7', '8'])
        ttk.Label(self.f, text = 'Data Bits:').grid(padx = 5, row = 2, column = 0)
        self.DataBits_Comb.grid(pady = 4, padx = 5, row = 2, column = 1)

        self.StopBits_var = tk.StringVar()
        self.StopBits_var.set('1')
        self.StopBits_Comb = ttk.Combobox(self.f, width = 13, state = 'readonly', textvariable = self.StopBits_var,
                                          values=['1', '2'])
        ttk.Label(self.f, text = 'Stop Bits:').grid(padx = 5, row = 3, column = 0)
        self.StopBits_Comb.grid(pady = 4, padx = 5, row = 3, column = 1)

        self.Parity_var = tk.StringVar()
        self.Parity_var.set('None')
        self.Parity_Comb = ttk.Combobox(self.f, width = 13, state = 'readonly', textvariable = self.Parity_var,
                                          values=['None', 'Even', 'Odd'])
        ttk.Label(self.f, text = 'Stop Bits:').grid(padx = 5, row = 4, column = 0)
        self.Parity_Comb.grid(pady = 4, padx = 5, row = 4, column = 1)

        # создаем Buttons для начала и окончания соединения UART
        self.Start_Button = ttk.Button(self.f, text = '     Start\nConnection', state = '!disabled', command = self.Start_Button_Pressed)
        self.Start_Button.grid(pady = 12, padx = 15, row = 5, column = 0)
        
        self.Stop_Button = ttk.Button(self.f, text = '     Stop\nConnection', state = 'disabled', command = self.Stop_Button_Pressed)
        self.Stop_Button.grid(pady = 12, padx = 1, row = 5, column = 1)

        # создаем объкет для подключения к последовательному порту
        self.Serial_Port = serial.Serial()
 
    def get_frame(self):
        # метод возвращает Frame
        return self.f
       
