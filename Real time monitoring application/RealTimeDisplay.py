import tkinter as tk
from tkinter import ttk, messagebox
import speech_recognition as sr
import threading
import requests
import time

# ESP32 IP address
esp32_ip = "http://192.168.25.165"


def fetch_sensor_data():
    try:
        url = f"{esp32_ip}/sensor_data"  # Endpoint to get sensor data
        response = requests.get(url, timeout=5)
        if response.status_code == 200:
            data = response.json()
            # Update sensor values in the GUI
            temperature_var.set(f"Temperature: {data['temperature']} °C")
            gas_var.set(f"Gas Value: {data['gasValue']}")
            light_var.set(f"Light Value: {data['lightValue']}")
            humidity_var.set(f"Humidity: {data['humidity']} %")
        else:
            print("Failed to fetch sensor data. Response:", response.status_code)
    except requests.exceptions.RequestException as e:
        print("Error fetching sensor data:", e)
    # Schedule the function to run every 5 seconds
    app.after(5000, fetch_sensor_data)


def send_command(state):
    url = f"{esp32_ip}/control?state={state}"
    try:
        response = requests.get(url, timeout=5)
        if response.status_code == 200:
            add_message(f"System: Wall Lights turned {state.upper()}", "system")
        else:
            add_message(f"System: Failed to control Wall Lights. Response: {response.status_code}", "error")
    except requests.exceptions.RequestException as e:
        add_message(f"System: Error in sending request: {e}", "error")


def continuous_voice_recognition():
    recognizer = sr.Recognizer()
    with sr.Microphone() as source:
        add_message("System: Voice command feature activated.", "system")
        while True:
            try:
                add_message("System: Listening for commands...", "info")
                audio = recognizer.listen(source, timeout=5, phrase_time_limit=5)
                command = recognizer.recognize_google(audio).lower()
                add_message(f"{command}", "user")

                if "turn on" in command:
                    send_command("on")
                elif "turn off" in command:
                    send_command("off")
                else:
                    add_message("System: Command not recognized. Try 'turn on' or 'turn off'.", "error")
            except sr.UnknownValueError:
                add_message("System: Sorry, I didn't catch that.", "error")
            except sr.RequestError:
                add_message("System: Could not connect to the speech recognition service.", "error")
            except Exception as e:
                add_message(f"System: Unexpected error: {e}", "error")

# Start voice recognition in a thread
def start_voice_recognition():
    threading.Thread(target=continuous_voice_recognition, daemon=True).start()

# Add messages
def add_message(message, msg_type="info"):
    if msg_type == "user":
        chat_text.insert(tk.END, f"You: {message}\n", "user")
    elif msg_type == "system":
        chat_text.insert(tk.END, f"{message}\n", "system")
    elif msg_type == "error":
        chat_text.insert(tk.END, f"Error: {message}\n", "error")
    elif msg_type == "info":
        chat_text.insert(tk.END, f"{message}\n", "info")
    chat_text.see(tk.END)

# GUI setup
app = tk.Tk()
app.title("Real-Time ESP32 Controller")
app.geometry("600x750")
app.configure(bg="#1e1e1e")


style = ttk.Style()
style.theme_use("clam")

# Real-time data display section
sensor_frame = tk.Frame(app, bg="#1e1e1e")
sensor_frame.pack(fill="x", padx=10, pady=10)

temperature_var = tk.StringVar(value="Temperature: -- °C")
gas_var = tk.StringVar(value="Gas Value: --")
light_var = tk.StringVar(value="Light Value: --")
humidity_var = tk.StringVar(value="Humidity: --")

tk.Label(sensor_frame, textvariable=temperature_var, bg="#1e1e1e", fg="#e0e0e0", font=("Arial", 14)).pack(anchor="w")
tk.Label(sensor_frame, textvariable=gas_var, bg="#1e1e1e", fg="#e0e0e0", font=("Arial", 14)).pack(anchor="w")
tk.Label(sensor_frame, textvariable=light_var, bg="#1e1e1e", fg="#e0e0e0", font=("Arial", 14)).pack(anchor="w")
tk.Label(sensor_frame, textvariable=humidity_var, bg="#1e1e1e", fg="#e0e0e0", font=("Arial", 14)).pack(anchor="w")

# Chat window
chat_frame = tk.Frame(app, bg="#1e1e1e")
chat_frame.pack(fill="both", expand=True, padx=10, pady=10)

chat_text = tk.Text(chat_frame, wrap="word", state="normal", height=30, bg="#2b2b2b", fg="#e0e0e0", font=("Arial", 12), insertbackground="white")
chat_text.pack(fill="both", expand=True, side="left")

# Scrollbar for chat
scrollbar = tk.Scrollbar(chat_frame, command=chat_text.yview, bg="#1e1e1e", troughcolor="#333")
scrollbar.pack(side="right", fill="y")
chat_text.config(yscrollcommand=scrollbar.set)

# Tag configurations
chat_text.tag_config("user", foreground="#76c7c0", font=("Arial", 12, "bold"))
chat_text.tag_config("system", foreground="#c0c776", font=("Arial", 12))
chat_text.tag_config("error", foreground="#f7768e", font=("Arial", 12, "italic"))
chat_text.tag_config("info", foreground="#8f8f8f", font=("Arial", 12))

# Buttons
button_frame = tk.Frame(app, bg="#1e1e1e")
button_frame.pack(pady=10, fill="x")

voice_command_btn = tk.Button(button_frame, text="Start Voice Command", command=start_voice_recognition, bg="#444444", fg="white", font=("Arial", 12), padx=10, pady=10, relief="flat")
voice_command_btn.pack(side="left", padx=10, expand=True, fill="x")

turn_on_btn = tk.Button(button_frame, text="Turn On", command=lambda: send_command("on"), bg="#444444", fg="white", font=("Arial", 12), padx=10, pady=10, relief="flat")
turn_on_btn.pack(side="left", padx=10, expand=True, fill="x")

turn_off_btn = tk.Button(button_frame, text="Turn Off", command=lambda: send_command("off"), bg="#444444", fg="white", font=("Arial", 12), padx=10, pady=10, relief="flat")
turn_off_btn.pack(side="left", padx=10, expand=True, fill="x")


fetch_sensor_data()  
start_voice_recognition()  
app.mainloop()
