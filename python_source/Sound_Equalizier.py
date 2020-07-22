import tkinter as tk
from tkinter import ttk

from threading import Thread
from time import sleep

class Sound_Equalizier:
    def Record_Thread(self):
        while True:
            sleep(0.3)
            if self.record_threads_stop: break
            self.recorded_blocks_var.set(self.recorded_blocks_var.get() + 1)
            self.rec_blocks['text'] = str(self.recorded_blocks_var.get())

    def Play_Thread(self):
        while True:
            sleep(0.1)
            if self.start_threads_stop: break
            self.current_block_var.set(self.current_block_var.get() + 1)
            if self.current_block_var.get() > int(self.play_blocks_var.get()):
                self.current_block_var.set(0)
            self.cur_blocks['text'] = str(self.current_block_var.get())
            

    def radiobutton_command(self):
        # обработчик события для Radiobutton
        self.uart_socket.Serial_Port.write(bytes([254]))
        self.uart_socket.Serial_Port.write(bytes([self.rb_var.get()]))
        self.uart_socket.Serial_Port.write(bytes(5))

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

    def Record_Button_Pressed(self):
        if self.Play_Button['state'] == '!disabled':
            self.Play_Button['state'] = 'disabled'
            self.Record_Button['text'] = 'Stop'
            self.recorded_blocks_var.set(0)
            # запускаем поток для считывания числа записанных блоков
            self.record_threads_stop = False
            self.run_thread = Thread(target = self.Record_Thread)
            self.run_thread.setDaemon(True)
            self.run_thread.start()
            # посылаем флаг начала записи
##            self.uart_socket.Serial_Port.write(bytes([253]))
##            self.uart_socket.Serial_Port.write(bytes([1]))
##            self.uart_socket.Serial_Port.write(bytes(5))
        else:
            self.Play_Button['state'] = '!disabled'
            self.Record_Button['text'] = 'Record'
            # останавливаем поток для считывания числа записанных блоков
            self.record_threads_stop = True
            sleep(0.1)
            self.run_thread.join()
            # посылаем окончания начала записи
##            self.uart_socket.Serial_Port.write(bytes([253]))
##            self.uart_socket.Serial_Port.write(bytes([0]))
##            self.uart_socket.Serial_Port.write(bytes(5))
        

    def Play_Button_Pressed(self):
        if self.Record_Button['state'] == '!disabled':
            self.Record_Button['state'] = 'disabled'
            self.Play_Button['text'] = 'Stop'
            self.current_block_var.set(0)
            # запускаем поток для считывания числа записанных блоков
            self.start_threads_stop = False
            self.run_thread = Thread(target = self.Play_Thread)
            self.run_thread.setDaemon(True)
            self.run_thread.start()
        else:
            self.Record_Button['state'] = '!disabled'
            self.Play_Button['text'] = 'Play'
            # останавливаем поток для считывания числа записанных блоков
            self.start_threads_stop = True
            sleep(0.1)
            self.run_thread.join()
        
    def __init__(self, master, uart_socket):

        self.record_threads_stop = False
        self.start_threads_stop = False

        self.current_block_var = tk.IntVar()
        self.current_block_var.set(0)

        self.recorded_blocks_var = tk.IntVar()
        self.recorded_blocks_var.set(0)

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
        
        # создаем Frame для управления sdcard
        self.sdcard_f = ttk.Frame(self.f, width = 40, height = 200)
        self.sdcard_f.pack(padx = 20, pady = 10)

        ttk.Label(self.sdcard_f, text = 'Recorded Blocks:').grid(padx = 10, row = 0, column = 0)
        self.rec_blocks = ttk.Label(self.sdcard_f, text = '0')
        self.rec_blocks.grid(padx = 10, row = 0, column = 1)
        
        ttk.Label(self.sdcard_f, text = 'Current Block:').grid(padx = 10, row = 1, column = 0)
        self.cur_blocks = ttk.Label(self.sdcard_f, text = '0')
        self.cur_blocks.grid(padx = 10, row = 1, column = 1)

        self.play_blocks_var = tk.StringVar()
        self.play_blocks_var.set('1000')
        ttk.Label(self.sdcard_f, text = 'Blocks to play:').grid(padx = 10, row = 2, column = 0)
        self.play_blocks = ttk.Entry(self.sdcard_f, textvariable = self.play_blocks_var, width = 10, justify = 'center').grid(padx = 10, row = 2, column = 1)

        self.Record_Button = ttk.Button(self.sdcard_f, text = 'Record', state = '!disabled', command = self.Record_Button_Pressed)
        self.Record_Button.grid(pady = 12, padx = 15, row = 3, column = 0)

        self.Play_Button = ttk.Button(self.sdcard_f, text = 'Play', state = '!disabled', command = self.Play_Button_Pressed)
        self.Play_Button.grid(pady = 12, padx = 15, row = 3, column = 1)
        
    def get_frame(self):
        # метод возвращает Frame
        return self.f
    
