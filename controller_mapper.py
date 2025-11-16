import pygame
import json
import tkinter as tk
from tkinter import ttk, scrolledtext
import threading
import time

class ControllerMapper:
    def __init__(self, root):
        self.root = root
        self.root.title("pdEMU Controller Mapper")
        self.root.geometry("800x600")
        
        pygame.init()
        pygame.joystick.init()
        
        self.controllers = []
        self.selected_controller = None
        self.mapping_active = False
        self.current_button_index = 0
        self.mapping_data = {}
        
        # Mapping sequence for buttons
        self.button_sequence = [
            ("A", "button"),
            ("B", "button"),
            ("X", "button"),
            ("Y", "button"),
            ("R1", "button"),
            ("L1", "button"),
            ("R2", "button"),
            ("L2", "button"),
            ("R3", "button"),
            ("L3", "button"),
            ("Start", "button"),
            ("Select", "button"),
            ("D-Up", "button"),
            ("D-Down", "button"),
            ("D-Left", "button"),
            ("D-Right", "button"),
            ("Left-Stick-Up", "axis"),
            ("Left-Stick-Down", "axis"),
            ("Left-Stick-Left", "axis"),
            ("Left-Stick-Right", "axis"),
            ("Right-Stick-Up", "axis"),
            ("Right-Stick-Down", "axis"),
            ("Right-Stick-Left", "axis"),
            ("Right-Stick-Right", "axis"),
        ]
        
        self.setup_ui()
        self.refresh_controllers()
        
    def setup_ui(self):
        # Controller selection frame
        select_frame = ttk.LabelFrame(self.root, text="Controller Selection", padding=10)
        select_frame.pack(fill="x", padx=10, pady=5)
        
        ttk.Label(select_frame, text="Select Controller:").pack(side="left", padx=5)
        
        self.controller_var = tk.StringVar()
        self.controller_dropdown = ttk.Combobox(select_frame, textvariable=self.controller_var, 
                                                state="readonly", width=50)
        self.controller_dropdown.pack(side="left", padx=5, fill="x", expand=True)
        self.controller_dropdown.bind("<<ComboboxSelected>>", self.on_controller_selected)
        
        ttk.Button(select_frame, text="Refresh", command=self.refresh_controllers).pack(side="left", padx=5)
        
        # Mapping control frame
        control_frame = ttk.Frame(self.root, padding=10)
        control_frame.pack(fill="x", padx=10, pady=5)
        
        self.start_button = ttk.Button(control_frame, text="Start Mapping", 
                                       command=self.start_mapping, state="disabled")
        self.start_button.pack(side="left", padx=5)
        
        self.stop_button = ttk.Button(control_frame, text="Stop Mapping", 
                                      command=self.stop_mapping, state="disabled")
        self.stop_button.pack(side="left", padx=5)
        
        self.restart_button = ttk.Button(control_frame, text="Restart Mapping", 
                                        command=self.restart_mapping, state="disabled")
        self.restart_button.pack(side="left", padx=5)
        
        # Status frame
        status_frame = ttk.LabelFrame(self.root, text="Current Action", padding=10)
        status_frame.pack(fill="x", padx=10, pady=5)
        
        self.status_label = ttk.Label(status_frame, text="Select a controller to begin", 
                                     font=("Arial", 14, "bold"))
        self.status_label.pack()
        
        # Input visualization frame
        viz_frame = ttk.LabelFrame(self.root, text="Input Visualization", padding=10)
        viz_frame.pack(fill="both", expand=True, padx=10, pady=5)
        
        self.viz_text = scrolledtext.ScrolledText(viz_frame, height=10, state="disabled")
        self.viz_text.pack(fill="both", expand=True)
        
        # Log frame
        log_frame = ttk.LabelFrame(self.root, text="Mapping Log", padding=10)
        log_frame.pack(fill="both", expand=True, padx=10, pady=5)
        
        self.log_text = scrolledtext.ScrolledText(log_frame, height=10, state="disabled")
        self.log_text.pack(fill="both", expand=True)
        
        ttk.Button(log_frame, text="Copy to Clipboard", command=self.copy_to_clipboard).pack(pady=5)
        
    def refresh_controllers(self):
        pygame.joystick.quit()
        pygame.joystick.init()
        
        self.controllers = []
        controller_names = []
        
        num_joysticks = pygame.joystick.get_count()
        
        for i in range(num_joysticks):
            joy = pygame.joystick.Joystick(i)
            joy.init()
            
            name = joy.get_name()
            guid = joy.get_guid()
            
            # Detect twin controller setups (same name, sequential indices)
            if i > 0 and i < num_joysticks - 1:
                prev_joy = pygame.joystick.Joystick(i - 1)
                if prev_joy.get_name() == name:
                    display_name = f"{name} - Controller B (Twin Setup)"
                    is_twin = True
                else:
                    display_name = name
                    is_twin = False
            elif i < num_joysticks - 1:
                next_joy = pygame.joystick.Joystick(i + 1)
                if next_joy.get_name() == name:
                    display_name = f"{name} - Controller A (Twin Setup)"
                    is_twin = True
                else:
                    display_name = name
                    is_twin = False
            else:
                display_name = name
                is_twin = False
            
            self.controllers.append({
                'index': i,
                'joystick': joy,
                'name': name,
                'display_name': display_name,
                'guid': guid,
                'is_twin': is_twin
            })
            
            controller_names.append(f"{i}: {display_name}")
        
        self.controller_dropdown['values'] = controller_names
        if controller_names:
            self.controller_dropdown.current(0)
            self.on_controller_selected(None)
    
    def on_controller_selected(self, event):
        selection = self.controller_dropdown.current()
        if selection >= 0:
            self.selected_controller = self.controllers[selection]
            self.start_button.config(state="normal")
            
            info = f"Selected: {self.selected_controller['display_name']}\n"
            info += f"GUID: {self.selected_controller['guid']}\n"
            info += f"Twin Setup: {'Yes' if self.selected_controller['is_twin'] else 'No'}\n"
            
            self.log_text.config(state="normal")
            self.log_text.delete(1.0, tk.END)
            self.log_text.insert(tk.END, info)
            self.log_text.config(state="disabled")
    
    def start_mapping(self):
        self.mapping_active = True
        self.current_button_index = 0
        self.mapping_data = {
            'name': self.selected_controller['name'],
            'guid': self.selected_controller['guid'],
            'is_twin': self.selected_controller['is_twin'],
            'buttons': {},
            'axes': {},
            'hats': {}
        }
        
        self.start_button.config(state="disabled")
        self.stop_button.config(state="normal")
        self.restart_button.config(state="normal")
        self.controller_dropdown.config(state="disabled")
        
        self.update_status()
        
        # Start input polling thread
        self.poll_thread = threading.Thread(target=self.poll_inputs, daemon=True)
        self.poll_thread.start()
    
    def stop_mapping(self):
        self.mapping_active = False
        self.start_button.config(state="normal")
        self.stop_button.config(state="disabled")
        self.restart_button.config(state="disabled")
        self.controller_dropdown.config(state="normal")
        self.status_label.config(text="Mapping stopped")
        self.generate_final_log()
    
    def restart_mapping(self):
        self.mapping_active = False
        time.sleep(0.1)  # Brief pause to ensure poll thread stops
        self.log_text.config(state="normal")
        self.log_text.delete(1.0, tk.END)
        self.log_text.config(state="disabled")
        self.viz_text.config(state="normal")
        self.viz_text.delete(1.0, tk.END)
        self.viz_text.config(state="disabled")
        self.start_mapping()
    
    def update_status(self):
        if self.current_button_index < len(self.button_sequence):
            button_name, button_type = self.button_sequence[self.current_button_index]
            if button_type == "axis":
                self.status_label.config(text=f"Move/Press: {button_name} (move fully in both directions)")
            else:
                self.status_label.config(text=f"Press: {button_name}")
        else:
            self.status_label.config(text="Mapping complete!")
            self.stop_mapping()
    
    def poll_inputs(self):
        last_button_states = {}
        last_axis_values = {}
        last_hat_states = {}
        
        while self.mapping_active:
            pygame.event.pump()
            
            if self.current_button_index >= len(self.button_sequence):
                self.root.after(0, self.stop_mapping)
                break
            
            joy = self.selected_controller['joystick']
            button_name, button_type = self.button_sequence[self.current_button_index]
            
            # Visualize current inputs
            viz_text = ""
            
            # Check buttons
            for i in range(joy.get_numbuttons()):
                if joy.get_button(i):
                    viz_text += f"Button {i}: PRESSED\n"
                    if button_type == "button" and i not in last_button_states:
                        # New button press detected
                        self.mapping_data['buttons'][button_name] = i
                        self.root.after(0, self.advance_mapping, f"Button {i} -> {button_name}")
                        last_button_states[i] = True
                else:
                    last_button_states.pop(i, None)
            
            # Check hats (D-pad) - check before axes since D-pad buttons might be what we're mapping
            for i in range(joy.get_numhats()):
                hat = joy.get_hat(i)
                if hat != (0, 0):
                    viz_text += f"Hat {i}: {hat}\n"
                    
                    # Map hat directions to button names
                    if button_type == "button":
                        hat_key = (i, hat)
                        if hat_key not in last_hat_states:
                            # Determine which direction
                            direction = None
                            if hat[1] == 1:  # Up
                                direction = "up"
                            elif hat[1] == -1:  # Down
                                direction = "down"
                            elif hat[0] == -1:  # Left
                                direction = "left"
                            elif hat[0] == 1:  # Right
                                direction = "right"
                            
                            if direction:
                                self.mapping_data['hats'][button_name] = {'hat': i, 'direction': direction}
                                self.root.after(0, self.advance_mapping, f"Hat {i} {direction} -> {button_name}")
                                last_hat_states[hat_key] = True
                else:
                    # Clear all hat states when centered
                    last_hat_states.clear()
            
            # Check axes
            for i in range(joy.get_numaxes()):
                value = joy.get_axis(i)
                if abs(value) > 0.5:
                    viz_text += f"Axis {i}: {value:.2f}\n"
                    if button_type == "axis" and abs(value) > 0.8:
                        if i not in last_axis_values or abs(last_axis_values[i] - value) > 0.5:
                            # Determine direction (positive or negative)
                            direction = "+" if value > 0 else "-"
                            self.mapping_data['axes'][button_name] = {'axis': i, 'direction': direction, 'value': value}
                            self.root.after(0, self.advance_mapping, f"Axis {i} {direction} ({value:.2f}) -> {button_name}")
                            last_axis_values[i] = value
            
            self.root.after(0, self.update_viz, viz_text)
            time.sleep(0.05)
    
    def update_viz(self, text):
        self.viz_text.config(state="normal")
        self.viz_text.delete(1.0, tk.END)
        self.viz_text.insert(tk.END, text)
        self.viz_text.config(state="disabled")
    
    def advance_mapping(self, log_entry):
        self.log_text.config(state="normal")
        self.log_text.insert(tk.END, f"{log_entry}\n")
        self.log_text.see(tk.END)
        self.log_text.config(state="disabled")
        
        self.current_button_index += 1
        self.update_status()
        time.sleep(0.5)  # Debounce
    
    def generate_final_log(self):
        log = "=" * 50 + "\n"
        log += "CONTROLLER MAPPING COMPLETE\n"
        log += "=" * 50 + "\n\n"
        log += f"Controller Name: {self.mapping_data['name']}\n"
        log += f"GUID: {self.mapping_data['guid']}\n"
        log += f"Twin Controller Setup: {'Yes' if self.mapping_data['is_twin'] else 'No'}\n\n"
        
        log += "BUTTONS:\n"
        for btn_name, btn_id in self.mapping_data['buttons'].items():
            log += f"  {btn_name}: {btn_id}\n"
        
        log += "\nAXES:\n"
        for axis_name, axis_data in self.mapping_data['axes'].items():
            log += f"  {axis_name}: Axis {axis_data['axis']} {axis_data['direction']} (value: {axis_data['value']:.2f})\n"
        
        log += "\nHATS (D-PAD):\n"
        for hat_name, hat_data in self.mapping_data['hats'].items():
            # Determine axis and direction for hat
            hat_num = hat_data['hat']
            direction = hat_data['direction']
            if direction in ['up', 'down']:
                axis_name = 'Y'
                sign = '+' if direction == 'up' else '-'
            else:  # left or right
                axis_name = 'X'
                sign = '+' if direction == 'right' else '-'
            log += f"  {hat_name}: Hat {hat_num} {axis_name} {sign} ({direction})\n"
        
        log += "\n" + "=" * 50 + "\n"
        log += "JSON FORMAT:\n"
        log += "=" * 50 + "\n"
        log += json.dumps(self.mapping_data, indent=2)
        
        self.log_text.config(state="normal")
        self.log_text.delete(1.0, tk.END)
        self.log_text.insert(tk.END, log)
        self.log_text.config(state="disabled")
    
    def copy_to_clipboard(self):
        self.root.clipboard_clear()
        self.root.clipboard_append(self.log_text.get(1.0, tk.END))
        self.status_label.config(text="Copied to clipboard!")
        self.root.after(2000, lambda: self.status_label.config(text="Ready"))

if __name__ == "__main__":
    root = tk.Tk()
    app = ControllerMapper(root)
    root.mainloop()
    pygame.quit()
