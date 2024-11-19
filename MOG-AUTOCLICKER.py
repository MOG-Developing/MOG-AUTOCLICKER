import tkinter as tk
from tkinter import ttk, messagebox
import keyboard
import mouse
import threading
import time

class AutoClicker:
    def __init__(self, root):
        self.root = root
        self.root.title("Auto Clicker")
        self.root.geometry("350x400")  # Increased size for better layout
        self.root.resizable(False, False)  # Fixed window size
        
        # Dark theme colors
        self.colors = {
            'bg': '#1a1a1a',
            'accent': '#4a148c',
            'text': '#ffffff',
            'button': '#2a2a2a',
            'entry': '#2d2d2d',
            'border': '#3d3d3d'
        }
        
        # Configure root window
        self.root.configure(bg=self.colors['bg'])
        
        # Variables
        self.clicking = False
        self.click_thread = None
        self.cps = tk.DoubleVar(value=1.0)
        self.click_button = tk.StringVar(value='left')
        self.custom_cps = tk.StringVar(value='1.0')
        
        self.setup_ui()
        self.setup_keyboard_listener()

    def setup_ui(self):
        # Style configuration
        style = ttk.Style()
        style.configure('TScale', background=self.colors['bg'])
        style.configure('TLabel', background=self.colors['bg'], foreground=self.colors['text'])
        style.configure('TFrame', background=self.colors['bg'])
        
        # Main container
        main_frame = ttk.Frame(self.root, padding="10")
        main_frame.pack(fill=tk.BOTH, expand=True)
        
        # Title
        title_label = tk.Label(
            main_frame,
            text="MOG-AUTOCLICKER",
            bg=self.colors['bg'],
            fg=self.colors['accent'],
            font=('Arial', 14, 'bold')
        )
        title_label.pack(pady=(0, 15))
        
        # Mouse Button Selection
        button_frame = tk.Frame(main_frame, bg=self.colors['bg'])
        button_frame.pack(fill=tk.X, pady=(0, 10))
        
        button_label = tk.Label(
            button_frame,
            text="Mouse Button:",
            bg=self.colors['bg'],
            fg=self.colors['text'],
            font=('Arial', 10, 'bold')
        )
        button_label.pack(pady=(0, 5))
        
        radio_frame = tk.Frame(button_frame, bg=self.colors['bg'])
        radio_frame.pack()
        
        for btn_text, btn_value in [("Left Click", 'left'), ("Right Click", 'right')]:
            radio = tk.Radiobutton(
                radio_frame,
                text=btn_text,
                variable=self.click_button,
                value=btn_value,
                bg=self.colors['bg'],
                fg=self.colors['text'],
                selectcolor=self.colors['accent'],
                activebackground=self.colors['bg'],
                activeforeground=self.colors['text'],
                font=('Arial', 9)
            )
            radio.pack(side=tk.LEFT, padx=10)
        
        # CPS Settings Frame
        cps_frame = tk.LabelFrame(
            main_frame,
            text="CPS Settings (0.1 - 10,000)",
            bg=self.colors['bg'],
            fg=self.colors['text'],
            font=('Arial', 10, 'bold'),
            padx=10,
            pady=10
        )
        cps_frame.pack(fill=tk.X, pady=10)
        
        # Custom CPS Entry
        custom_frame = tk.Frame(cps_frame, bg=self.colors['bg'])
        custom_frame.pack(fill=tk.X, pady=(0, 10))
        
        custom_label = tk.Label(
            custom_frame,
            text="Custom CPS:",
            bg=self.colors['bg'],
            fg=self.colors['text'],
            font=('Arial', 9)
        )
        custom_label.pack(side=tk.LEFT, padx=(0, 5))
        
        self.custom_entry = tk.Entry(
            custom_frame,
            textvariable=self.custom_cps,
            width=10,
            bg=self.colors['entry'],
            fg=self.colors['text'],
            insertbackground=self.colors['text'],
            relief=tk.FLAT
        )
        self.custom_entry.pack(side=tk.LEFT, padx=5)
        
        apply_button = tk.Button(
            custom_frame,
            text="Apply",
            command=self.apply_custom_cps,
            bg=self.colors['accent'],
            fg=self.colors['text'],
            relief=tk.FLAT,
            padx=10
        )
        apply_button.pack(side=tk.LEFT, padx=5)
        
        # CPS Slider
        self.cps_slider = ttk.Scale(
            cps_frame,
            from_=0.1,
            to=1000.0,  # Increased to 1000
            variable=self.cps,
            orient='horizontal',
            style='TScale'
        )
        self.cps_slider.pack(fill=tk.X, pady=5)
        
        # Slider range labels
        slider_labels = tk.Frame(cps_frame, bg=self.colors['bg'])
        slider_labels.pack(fill=tk.X)
        
        tk.Label(
            slider_labels,
            text="0.1",
            bg=self.colors['bg'],
            fg=self.colors['text'],
            font=('Arial', 8)
        ).pack(side=tk.LEFT)
        
        tk.Label(
            slider_labels,
            text="1000",
            bg=self.colors['bg'],
            fg=self.colors['text'],
            font=('Arial', 8)
        ).pack(side=tk.RIGHT)
        
        # Current CPS Display
        self.current_cps = tk.Label(
            cps_frame,
            text="Current CPS: 1.0",
            bg=self.colors['bg'],
            fg=self.colors['text'],
            font=('Arial', 9)
        )
        self.current_cps.pack()
        
        # Status Frame
        status_frame = tk.Frame(main_frame, bg=self.colors['bg'])
        status_frame.pack(fill=tk.X, pady=10)
        
        self.status_label = tk.Label(
            status_frame,
            text="Status: Stopped",
            bg=self.colors['bg'],
            fg=self.colors['text'],
            font=('Arial', 10, 'bold')
        )
        self.status_label.pack()
        
        # Instructions
        self.instructions = tk.Label(
            main_frame,
            text="Press 'G' to toggle auto-clicking",
            bg=self.colors['bg'],
            fg=self.colors['accent'],
            font=('Arial', 10, 'bold')
        )
        self.instructions.pack(pady=10)
        
        # Update CPS label when slider moves
        self.cps_slider.bind('<Motion>', self.update_cps_label)

    def apply_custom_cps(self):
        try:
            custom_value = float(self.custom_cps.get())
            if 0.1 <= custom_value <= 10000:  # Increased maximum to 10000
                self.cps.set(custom_value)
                self.update_cps_label()
            else:
                messagebox.showwarning("Invalid Input", "Please enter a value between 0.1 and 10,000")
        except ValueError:
            messagebox.showwarning("Invalid Input", "Please enter a valid number")

    def setup_keyboard_listener(self):
        keyboard.on_press_key('g', lambda _: self.toggle_clicking())

    def update_cps_label(self, event=None):
        self.current_cps.config(text=f"Current CPS: {self.cps.get():.1f}")

    def toggle_clicking(self):
        self.clicking = not self.clicking
        
        if self.clicking:
            self.status_label.config(text="Status: Running", fg='#00ff00')
            self.click_thread = threading.Thread(target=self.auto_click)
            self.click_thread.daemon = True
            self.click_thread.start()
        else:
            self.status_label.config(text="Status: Stopped", fg=self.colors['text'])

    def auto_click(self):
        while self.clicking:
            if self.clicking:  # Double-check to prevent extra clicks
                mouse.click(button=self.click_button.get())
                time.sleep(1 / self.cps.get())

def main():
    root = tk.Tk()
    app = AutoClicker(root)
    root.mainloop()

if __name__ == "__main__":
    main()