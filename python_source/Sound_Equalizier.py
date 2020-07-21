import tkinter as tk
from tkinter import ttk

class Sound_Equalizier:

    def radiobutton_command(self):
        # обработчик события для Radiobutton
        pass

    def scale_command(self, *args):
        # обработчик команд Scale
        lpf_gian = self.max_scale - self.sl_var.get()
        bpf_gian = self.max_scale - self.sb_var.get()
        hpf_gian = self.max_scale - self.sh_var.get()

        self.uart_socket.Serial_Port.write(bytes([255]))
        self.uart_socket.Serial_Port.write(lpf_gian.to_bytes(2, byteorder='big', signed = False))
        self.uart_socket.Serial_Port.write(bpf_gian.to_bytes(2, byteorder='big', signed = False))
        self.uart_socket.Serial_Port.write(hpf_gian.to_bytes(2, byteorder='big', signed = False))
 
        self.ll['text'] = str(int(lpf_gian/self.max_scale*100)) + '%'
        self.lb['text'] = str(int(bpf_gian/self.max_scale*100)) + '%'
        self.lh['text'] = str(int(hpf_gian/self.max_scale*100)) + '%'
        
    def __init__(self, master, uart_socket):

        self.uart_socket = uart_socket
        
        # создаем Frame
        self.f = ttk.LabelFrame(master, text = 'Sound Equalizier', width = 20, height = 50)
        # содаем и инициализируем переменную для RadioButtons  
        self.rb_var = tk.IntVar()
        self.rb_var.set(0)
        # содаем и отображаем RadioButtons на Frame      
        self.rb_1 = tk.Radiobutton(self.f, text = "Microphone", variable = self.rb_var, value = 0, command = self.radiobutton_command)
        self.rb_2 = tk.Radiobutton(self.f, text = "SD Card     ", variable = self.rb_var, value = 1, command = self.radiobutton_command)
        self.rb_1.pack(pady = 10)
        self.rb_2.pack()
        # создаем Frame для Scales
        self.scale_f = ttk.Frame(self.f, width = 40, height = 200)
        self.scale_f.pack(padx = 20, pady = 10)
        # содаем и инициализируем переменные для Scales  
        self.max_scale = 0xFFFF
        self.sl_var = tk.IntVar()
        self.sl_var.set(0)
        self.sb_var = tk.IntVar()
        self.sb_var.set(0)
        self.sh_var = tk.IntVar()
        self.sh_var.set(0)

        # создаем Scales для красного, зеленого и синого светодиодов 
        self.sl = ttk.Scale(self.scale_f, from_ = 0, to = self.max_scale, orient = 'vertical', variable = self.sl_var, command = self.scale_command)
        self.sb = ttk.Scale(self.scale_f, from_ = 0, to = self.max_scale, orient = 'vertical', variable = self.sb_var, command = self.scale_command)
        self.sh = ttk.Scale(self.scale_f, from_ = 0, to = self.max_scale, orient = 'vertical', variable = self.sh_var, command = self.scale_command)
        self.ll = ttk.Label(self.scale_f, text = '100%', width = 4)
        self.lb = ttk.Label(self.scale_f, text = '100%', width = 4)
        self.lh = ttk.Label(self.scale_f, text = '100%', width = 4)
        self.ll.grid(padx = 10, row = 0, column = 0)
        self.lb.grid(padx = 10, row = 0, column = 1)
        self.lh.grid(padx = 10, row = 0, column = 2)
        self.sl.grid(padx = 10, row = 1, column = 0)
        self.sb.grid(padx = 10, row = 1, column = 1)
        self.sh.grid(padx = 10, row = 1, column = 2)
        ttk.Label(self.scale_f, text = '0 - 4\nMHz').grid(padx = 10, row = 2, column = 0)
        ttk.Label(self.scale_f, text = '4 - 8\nMHz').grid(padx = 10, row = 2, column = 1)
        ttk.Label(self.scale_f, text = '8 - 12\n MHz').grid(padx = 10, row = 2, column = 2)
        
         
    def get_frame(self):
        # метод возвращает Frame
        return self.f
    
