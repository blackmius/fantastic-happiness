import tkinter as tk
from tkinter import messagebox
import time
import socket
from datetime import datetime

class ToolTip(object):

    def __init__(self, widget):
        self.widget = widget
        self.tipwindow = None
        self.id = None
        self.x = self.y = 0

    def showtip(self, text):
        "Display text in tooltip window"
        self.text = text
        if self.tipwindow or not self.text:
            return
        x, y, cx, cy = self.widget.bbox("insert")
        x = x + self.widget.winfo_rootx() + 20
        y = y + cy + self.widget.winfo_rooty() + 27
        self.tipwindow = tw = tk.Toplevel(self.widget)
        tw.wm_overrideredirect(1)
        tw.wm_geometry("+%d+%d" % (x, y))
        label = tk.Label(tw, text=self.text, justify=tk.LEFT,
                      background="#ffffe0", relief=tk.SOLID, borderwidth=1,
                      font=("tahoma", "8", "normal"))
        label.pack(ipadx=1)

    def hidetip(self):
        tw = self.tipwindow
        self.tipwindow = None
        if tw:
            tw.destroy()

def CreateToolTip(widget, text):
    toolTip = ToolTip(widget)
    def enter(event):
        toolTip.showtip(text())
    def leave(event):
        toolTip.hidetip()
    widget.bind('<Enter>', enter)
    widget.bind('<Leave>', leave)

class LabeledEntry(tk.Frame):
    def __init__(self, master=None, name='', value=''):
        super().__init__(master)
        self.value = tk.StringVar()
        self.value.set(value)
        self.error = tk.StringVar()
        self.cb = tk.IntVar()
        cb = tk.Checkbutton(self, state='disabled', variable=self.cb)
        CreateToolTip(cb, self.get_cb_tip)
        cb.pack(side='left', pady=20)
        tk.Label(self, text=name, anchor='w').pack(fill='both')
        tk.Entry(self, textvariable=self.value).pack(fill='x')
        tk.Label(self, textvariable=self.error, anchor='w', fg='red').pack(fill='both')
    
    def get_cb_tip(self):
        if self.cb.get():
            return 'Подключен к серверу'
        return 'Не подключен к серверу'
        
class ConnectButton(tk.Button):
    def __init__(self, master, label, s):
        super().__init__(master)
        self.master = master
        self.label = label
        self.s = s
        self.config(text='Подключиться к серверу ' + str(s), command=self.cmd)
    
    def config_buttons(self):
        state = 'normal' if self.label.cb.get() else 'disabled'
        if self.s == 1:
            self.master.b1.config(state=state)
            self.master.b2.config(state=state)
        else:
            self.master.b3.config(state=state)
            self.master.b4.config(state=state)
        
    def cmd(self):
        try:
            self.label.error.set('')
            if self.label.cb.get():
                if self.sock:
                    self.sock.close()
                self.label.cb.set(0)
            else:
                self.label.cb.set(0)
                self.config(cursor='watch', state='disabled', text='Подключаюсь...')
                
                val = self.label.value.get()
                host, port = val.split(':')
                port = int(port)
                self.sock = socket.socket()
                self.sock.connect((host, port))
                
                self.label.cb.set(1)
            
        except Exception as e:
            self.label.error.set(str(e))
        finally:
            text = 'Отключиться от сервера' if self.label.cb.get() else 'Подключиться к серверу'
            self.config(cursor='', state='normal', text=text + ' ' + str(self.s))
            self.config_buttons()
    

class App(tk.Frame):
    def __init__(self, master=None):
        super().__init__(master)
        self.pack(pady=10, padx=10)
        
        s1 = LabeledEntry(self, name='Адрес сервера 1', value='127.0.0.1:8000')
        s1.pack()
        s2 = LabeledEntry(self, name='Адрес сервера 2', value='127.0.0.1:8001')
        s2.pack()
        
        self.cn1 = ConnectButton(self, s1, 1)
        self.cn1.pack(pady=5)
        self.cn2 = ConnectButton(self, s2, 2)
        self.cn2.pack(pady=5)
        
        tk.Label(self).pack(pady=15)
        self.b1 = tk.Button(
            self, state='disabled',
            text='Ширина и высота основного монитора',
            command=self.resolution
        )
        self.b1.pack(pady=5, fill='x')
        self.b2 = tk.Button(
            self, state='disabled',
            text='Цвет пиксела',
            command=self.pixel
        )
        self.b2.pack(pady=5, fill='x')
        self.b3 = tk.Button(
            self, state='disabled',
            text='Идентификатор серверного процесса',
            command=self.pid
        )
        self.b3.pack(pady=5, fill='x')
        self.b4 = tk.Button(
            self, state='disabled',
            text='Дескриптор серверного процесса',
            command=self.fd
        )
        self.b4.pack(pady=5, fill='x')
    
    def resolution(self):
        self.cn1.sock.sendall(bytearray([1]))
        time = int.from_bytes(self.cn1.sock.recv(4), 'little')
        time = datetime.fromtimestamp(time).ctime()
        width = int.from_bytes(self.cn1.sock.recv(2), 'little')
        height = int.from_bytes(self.cn1.sock.recv(2), 'little')
        info = f'Время на сервере {time}\nРазмеры монитора {width}x{height}'
        messagebox.showinfo('Ответ', info)
    
    def pixel(self):
        def run():
            payload = bytearray([2]) + \
                int.to_bytes(int(x.get()), 2, 'little') + \
                int.to_bytes(int(y.get()), 2, 'little')
            self.cn1.sock.sendall(payload)
            time = int.from_bytes(self.cn1.sock.recv(4), 'little')
            time = datetime.fromtimestamp(time).ctime()
            color = self.cn1.sock.recv(7).decode()
            info = f'Время на сервере {time}\nЦвет пиксела с x={x.get()}, y={y.get()}\n{color}'
            messagebox.showinfo('Ответ', info)
    
        w = tk.Toplevel(self)
        w.title("Введите параметры запроса")
        f = tk.Frame(w)
        f.pack(padx=10, pady=10)
        
        xf = tk.Frame(f)
        xf.pack()
        tk.Label(xf, text='Координата X', anchor='w').pack(fill='both', side='left')
        x = tk.StringVar()
        tk.Entry(xf,
            textvariable=x, validate='all',
            validatecommand=(self.register(lambda x: x == '' or str.isdigit(x)), '%P')
        ).pack(fill='x', side='right')
        
        yf = tk.Frame(f)
        yf.pack()
        tk.Label(yf, text='Координата Y', anchor='w').pack(fill='both', side='left')
        y = tk.StringVar()
        tk.Entry(yf,
            textvariable=y, validate='all',
            validatecommand=(self.register(lambda x: x == '' or str.isdigit(x)), '%P')
        ).pack(fill='x', side='right')
        
        tk.Button(f, text='OK', command=run).pack(pady=10)
    
    def pid(self):
        self.cn2.sock.sendall(bytearray([3]))
        time = int.from_bytes(self.cn2.sock.recv(4), 'little')
        time = datetime.fromtimestamp(time).ctime()
        pid = int.from_bytes(self.cn2.sock.recv(4), 'little')
        info = f'Время на сервере {time}\nИдентификатор серверного процесса = {pid}'
        messagebox.showinfo('Ответ', info)
    
    def fd(self):
        self.cn2.sock.sendall(bytearray([4]))
        time = int.from_bytes(self.cn2.sock.recv(4), 'little')
        time = datetime.fromtimestamp(time).ctime()
        fd = int.from_bytes(self.cn2.sock.recv(4), 'little')
        info = f'Время на сервере {time}\nДескриптор серверного процесса = {fd}'
        messagebox.showinfo('Ответ', info)
    
        
root = tk.Tk(className='Клиент для курсовой работы')
app = App(root)
root.mainloop()
