import tkinter as tk
from tkinter import ttk, messagebox, simpledialog
import keyboard
import mouse
import threading
import time
import json
import os
import sys
import ctypes
import math

def is_admin():
    try:
        return ctypes.windll.shell32.IsUserAnAdmin()
    except:
        return False

if not is_admin():
    ctypes.windll.shell32.ShellExecuteW(None, "runas", sys.executable, " ".join(sys.argv), None, 1)
    sys.exit()

class AutoClickerV3:
    def __init__(self, root):
        self.root = root
        self.root.title("MOG-AUTOCLICKER_V2 | Made by @misterofgames_yt")
        self.root.geometry("450x600")
        self.root.resizable(False, False)
        
        self.colors = {
            'bg': '#2e2e2e',
            'accent': '#4a76a8',
            'secondary': '#3a3a3a',
            'text': '#e0e0e0',
            'button': '#3a3a3a',
            'entry': '#3a3a3a',
            'border': '#4a4a4a',
            'success': '#5cb85c',
            'danger': '#d9534f',
            'warning': '#f0ad4e',
            'info': '#5bc0de'
        }
        
        self.root.configure(bg=self.colors['bg'])
        
        self.clicking = False
        self.click_thread = None
        self.click_mode = tk.StringVar(value='cps')
        self.click_value = tk.DoubleVar(value=10.0)
        self.custom_cps = tk.StringVar(value="10.0")
        self.click_button = tk.StringVar(value='left')
        self.hotkey = tk.StringVar(value='F6')
        self.randomization = tk.DoubleVar(value=0.0)
        self.hotkey_listener = None
        
        self.animation_angle = 0
        self.animation_running = False
        
        self.settings_file = "autoclicker_settings_v3.json"
        self.load_settings()
        
        self.setup_ui()
        self.setup_keyboard_listener()
        self.animate()

    def load_settings(self):
        if os.path.exists(self.settings_file):
            try:
                with open(self.settings_file, 'r') as f:
                    settings = json.load(f)
                    self.hotkey.set(settings.get('hotkey', 'F6'))
                    self.click_mode.set(settings.get('click_mode', 'cps'))
                    self.click_value.set(float(settings.get('click_value', 10.0)))
                    self.click_button.set(settings.get('click_button', 'left'))
                    self.randomization.set(float(settings.get('randomization', 0.0)))
                    self.custom_cps.set(settings.get('custom_cps', "10.0"))
            except Exception as e:
                print(f"Error loading settings: {e}")

    def save_settings(self):
        try:
            with open(self.settings_file, 'w') as f:
                json.dump({
                    'hotkey': self.hotkey.get(),
                    'click_mode': self.click_mode.get(),
                    'click_value': self.click_value.get(),
                    'click_button': self.click_button.get(),
                    'randomization': self.randomization.get(),
                    'custom_cps': self.custom_cps.get()
                }, f)
        except Exception as e:
            print(f"Error saving settings: {e}")

    def setup_ui(self):
        style = ttk.Style()
        style.theme_use('clam')
        
        style.configure('.', font=('Arial', 9))
        style.configure('TFrame', background=self.colors['bg'])
        style.configure('TLabel', background=self.colors['bg'], foreground=self.colors['text'])
        style.configure('TButton', 
                      background=self.colors['button'], 
                      foreground=self.colors['text'],
                      borderwidth=0)
        style.map('TButton',
                background=[('active', self.colors['accent'])])
        style.configure('TEntry', 
                      fieldbackground=self.colors['entry'],
                      foreground=self.colors['text'],
                      insertcolor=self.colors['text'])
        style.configure('TRadiobutton',
                       background=self.colors['bg'],
                       foreground=self.colors['text'])
        style.configure('TCheckbutton',
                       background=self.colors['bg'],
                       foreground=self.colors['text'])
        style.configure('TScale',
                       background=self.colors['bg'],
                       troughcolor=self.colors['secondary'])
        
        main_frame = ttk.Frame(self.root, padding="15")
        main_frame.pack(fill=tk.BOTH, expand=True)
        
        header_frame = ttk.Frame(main_frame)
        header_frame.pack(fill=tk.X, pady=(0, 15))
        
        self.logo_canvas = tk.Canvas(header_frame, width=40, height=40, bg=self.colors['bg'], 
                                    highlightthickness=0)
        self.logo_canvas.pack(side=tk.LEFT)
        
        title_label = tk.Label(
            header_frame,
            text="MOG-AUTOCLICKER_V2",
            bg=self.colors['bg'],
            fg=self.colors['accent'],
            font=('Arial', 16, 'bold')
        )
        title_label.pack(side=tk.LEFT, padx=10)
        
        mode_frame = ttk.LabelFrame(
            main_frame,
            text="Click Mode",
            padding=(15, 10)
        )
        mode_frame.pack(fill=tk.X, pady=5)
        
        for text, mode in [("Clicks Per Second (CPS)", 'cps'), ("Milliseconds (MS)", 'ms')]:
            rb = ttk.Radiobutton(
                mode_frame,
                text=text,
                variable=self.click_mode,
                value=mode
            )
            rb.pack(anchor=tk.W, pady=3)
        
        settings_frame = ttk.LabelFrame(
            main_frame,
            text="Click Settings",
            padding=(15, 10)
        )
        settings_frame.pack(fill=tk.X, pady=5)
        
        value_frame = ttk.Frame(settings_frame)
        value_frame.pack(fill=tk.X, pady=5)
        
        ttk.Label(
            value_frame,
            text="Value:"
        ).pack(side=tk.LEFT)
        
        self.value_entry = ttk.Entry(
            value_frame,
            textvariable=self.custom_cps,
            width=10
        )
        self.value_entry.pack(side=tk.LEFT, padx=5)
        
        self.value_unit = ttk.Label(
            value_frame,
            text="CPS"
        )
        self.value_unit.pack(side=tk.LEFT)
        
        apply_btn = ttk.Button(
            value_frame,
            text="Apply",
            command=self.apply_custom_cps
        )
        apply_btn.pack(side=tk.LEFT, padx=5)
        
        self.value_slider = ttk.Scale(
            settings_frame,
            from_=1,
            to=100,
            variable=self.click_value,
            command=self.update_value_display
        )
        self.value_slider.pack(fill=tk.X, pady=5)
        
        slider_labels = ttk.Frame(settings_frame)
        slider_labels.pack(fill=tk.X)
        
        ttk.Label(
            slider_labels,
            text="1"
        ).pack(side=tk.LEFT)
        
        ttk.Label(
            slider_labels,
            text="100"
        ).pack(side=tk.RIGHT)
        
        random_frame = ttk.Frame(settings_frame)
        random_frame.pack(fill=tk.X, pady=5)
        
        ttk.Label(
            random_frame,
            text="Randomization:"
        ).pack(side=tk.LEFT)
        
        self.random_slider = ttk.Scale(
            random_frame,
            from_=0,
            to=100,
            variable=self.randomization
        )
        self.random_slider.pack(side=tk.LEFT, padx=5, expand=True, fill=tk.X)
        
        ttk.Label(
            random_frame,
            textvariable=tk.StringVar(value=f"{self.randomization.get():.0f}%"),
            width=4
        ).pack(side=tk.LEFT)
        
        button_frame = ttk.Frame(settings_frame)
        button_frame.pack(fill=tk.X, pady=5)
        
        ttk.Label(
            button_frame,
            text="Mouse Button:"
        ).pack(side=tk.LEFT)
        
        for btn_text, btn_value in [("Left", 'left'), ("Right", 'right'), ("Middle", 'middle')]:
            rb = ttk.Radiobutton(
                button_frame,
                text=btn_text,
                variable=self.click_button,
                value=btn_value
            )
            rb.pack(side=tk.LEFT, padx=5)
        
        hotkey_frame = ttk.LabelFrame(
            main_frame,
            text="Hotkey Settings",
            padding=(15, 10)
        )
        hotkey_frame.pack(fill=tk.X, pady=5)
        
        ttk.Label(
            hotkey_frame,
            text="Current Hotkey:"
        ).pack(anchor=tk.W)
        
        self.hotkey_display = ttk.Label(
            hotkey_frame,
            textvariable=self.hotkey,
            font=('Arial', 10, 'bold'),
            foreground=self.colors['accent']
        )
        self.hotkey_display.pack(anchor=tk.W, pady=(0, 5))
        
        ttk.Button(
            hotkey_frame,
            text="Change Hotkey",
            command=self.change_hotkey
        ).pack(fill=tk.X)
        
        # Status frame
        status_frame = ttk.Frame(main_frame)
        status_frame.pack(fill=tk.X, pady=10)
        
        self.status_indicator = tk.Canvas(
            status_frame,
            width=20,
            height=20,
            bg=self.colors['secondary'],
            highlightthickness=0
        )
        self.status_indicator.pack(side=tk.LEFT, padx=5)
        self.update_status_indicator()
        
        self.status_label = ttk.Label(
            status_frame,
            text="Status: Stopped",
            font=('Arial', 10, 'bold')
        )
        self.status_label.pack(side=tk.LEFT)
        
        self.click_mode.trace_add('write', self.update_value_display)
        self.click_value.trace_add('write', self.update_value_display)
        self.randomization.trace_add('write', self.update_randomization_display)
        
        self.update_value_display()

    def apply_custom_cps(self):
        try:
            value = float(self.custom_cps.get())
            if value <= 0:
                messagebox.showerror("Invalid Value", "Value must be greater than 0")
                return
            
            if self.click_mode.get() == 'cps':
                if value > 1000:
                    if not messagebox.askyesno("High CPS Warning", 
                                             "Values above 1000 CPS may cause performance issues. Continue?"):
                        return
            else:
                if value < 1:
                    if not messagebox.askyesno("Low MS Warning",
                                             "Values below 1ms may cause performance issues. Continue?"):
                        return
            
            self.click_value.set(value)
            self.update_value_display()
        except ValueError:
            messagebox.showerror("Invalid Value", "Please enter a valid number")

    def update_randomization_display(self, *args):
        for widget in self.random_slider.master.winfo_children():
            if isinstance(widget, ttk.Label):
                widget.config(text=f"{self.randomization.get():.0f}%")
                break

    def update_value_display(self, *args):
        if self.click_mode.get() == 'cps':
            self.value_unit.config(text="CPS")
            self.value_slider.config(from_=1, to=100)
            self.custom_cps.set(f"{self.click_value.get():.1f}")
        else:
            self.value_unit.config(text="MS")
            self.value_slider.config(from_=1, to=1000)
            self.custom_cps.set(f"{self.click_value.get():.1f}")

    def update_status_indicator(self):
        color = self.colors['success'] if self.clicking else self.colors['danger']
        self.status_indicator.delete("all")
        self.status_indicator.create_oval(
            2, 2, 18, 18,
            fill=color,
            outline=color
        )

    def animate(self):
        if not self.animation_running:
            return
            
        self.animation_angle = (self.animation_angle + 5) % 360
        
        self.logo_canvas.delete("all")
        center_x, center_y = 20, 20
        radius = 15
        
        self.logo_canvas.create_oval(
            center_x - radius, center_y - radius,
            center_x + radius, center_y + radius,
            outline=self.colors['accent'],
            width=2
        )
        
        for i in range(8):
            angle = math.radians(i * 45 + self.animation_angle)
            outer_x = center_x + (radius + 5) * math.cos(angle)
            outer_y = center_y + (radius + 5) * math.sin(angle)
            inner_x = center_x + (radius - 3) * math.cos(angle)
            inner_y = center_y + (radius - 3) * math.sin(angle)
            
            self.logo_canvas.create_line(
                outer_x, outer_y,
                inner_x, inner_y,
                fill=self.colors['accent'],
                width=2
            )
        
        self.root.after(50, self.animate)

    def change_hotkey(self):
        if self.hotkey_listener is not None:
            try:
                keyboard.remove_hotkey(self.hotkey_listener)
            except:
                pass
        
        new_hotkey = simpledialog.askstring(
            "Change Hotkey",
            "Press the new hotkey (e.g., F6, Ctrl+Shift+A):\n\nNote: Some special keys may not work properly.",
            parent=self.root
        )
        
        if new_hotkey:
            try:
                test_listener = keyboard.add_hotkey(new_hotkey, lambda: None)
                keyboard.remove_hotkey(test_listener)
                
                self.hotkey.set(new_hotkey)
                self.setup_keyboard_listener()
                self.save_settings()
            except ValueError as e:
                messagebox.showerror("Hotkey Error", f"Invalid hotkey: {e}\nPlease try a different key combination.")
                self.setup_keyboard_listener()
        else:
            self.setup_keyboard_listener()

    def setup_keyboard_listener(self):
        if self.hotkey_listener is not None:
            try:
                keyboard.remove_hotkey(self.hotkey_listener)
            except:
                pass
        try:
            self.hotkey_listener = keyboard.add_hotkey(
                self.hotkey.get(),
                self.toggle_clicking
            )
        except ValueError as e:
            messagebox.showerror("Hotkey Error", f"Failed to register hotkey: {e}\nDefaulting to F6")
            self.hotkey.set("F6")
            self.hotkey_listener = keyboard.add_hotkey(
                self.hotkey.get(),
                self.toggle_clicking
            )

    def toggle_clicking(self):
        self.clicking = not self.clicking
        
        if self.clicking:
            self.status_label.config(text="Status: Running")
            self.animation_running = True
            self.animate()
            self.click_thread = threading.Thread(target=self.auto_click, daemon=True)
            self.click_thread.start()
        else:
            self.status_label.config(text="Status: Stopped")
            self.animation_running = False
            
        self.update_status_indicator()

    def get_click_delay(self):
        value = self.click_value.get()
        randomization = self.randomization.get() / 100.0
        
        if self.click_mode.get() == 'cps':
            base_delay = 1.0 / value
        else:
            base_delay = value / 1000.0
        
        if randomization > 0:
            random_factor = 1 + randomization * (2 * (0.5 - (time.time() % 1)))
            base_delay *= random_factor
        
        return max(0.001, base_delay)

    def auto_click(self):
        while self.clicking:
            try:
                mouse.click(button=self.click_button.get())
                time.sleep(self.get_click_delay())
            except:
                self.clicking = False
                self.root.after(0, self.update_status_indicator)
                self.root.after(0, lambda: self.status_label.config(text="Status: Error"))
                break

    def on_closing(self):
        # Clean up hotkey listener
        if self.hotkey_listener is not None:
            try:
                keyboard.remove_hotkey(self.hotkey_listener)
            except:
                pass
        
        # Save settings
        self.save_settings()
        
        self.clicking = False
        if self.click_thread is not None:
            self.click_thread.join(timeout=0.1)
        
        self.root.destroy()

def main():
    root = tk.Tk()
    try:
        app = AutoClickerV3(root)
        root.protocol("WM_DELETE_WINDOW", app.on_closing)
        
        window_width = 450
        window_height = 600
        screen_width = root.winfo_screenwidth()
        screen_height = root.winfo_screenheight()
        x = (screen_width - window_width) // 2
        y = (screen_height - window_height) // 2
        root.geometry(f"{window_width}x{window_height}+{x}+{y}")
        
        root.mainloop()
    except Exception as e:
        messagebox.showerror("Error", f"An unexpected error occurred:\n{str(e)}")
        sys.exit(1)

if __name__ == "__main__":
    main()
