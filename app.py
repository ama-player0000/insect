import threading
from fastapi import FastAPI
from pydantic import BaseModel
import uvicorn
import tkinter as tk
from tkinter import messagebox

# =============== FastAPI Backend ===============
portRun = 44555

app = FastAPI()
current_time = {"hour": 0, "minute": 0}

class TimeInput(BaseModel):
    hour: int
    minute: int

@app.post("/set-time")
def set_time(time: TimeInput):
    current_time["hour"] = time.hour
    current_time["minute"] = time.minute
    return {"message": "Time updated successfully"}

@app.get("/time")
def get_time():
    return current_time

def start_api():
    uvicorn.run(app, host="0.0.0.0", port=portRun)

# =============== Tkinter GUI ===============
def start_gui():
    def submit_time():
        try:
            h = int(hour_entry.get())
            m = int(minute_entry.get())
            if not (0 <= h <= 23 and 0 <= m <= 59):
                raise ValueError
            current_time["hour"] = h
            current_time["minute"] = m
            messagebox.showinfo("موفق", f"زمان به {h:02}:{m:02} تنظیم شد.")
        except ValueError:
            messagebox.showerror("خطا", "لطفاً ساعت و دقیقه معتبر وارد کنید.")

    window = tk.Tk()
    window.title("Setting Operating Parameters")
    window.geometry("300x180")
    # window.minsize("300x180")
    # window.maxsize("300x180")

    tk.Label(window, text=f"0.0.0.0:{portRun}/time").pack(pady=5)
    tk.Label(window, text="Hour (0-23 OR 0-12):").pack(pady=5)
    hour_entry = tk.Entry(window)
    hour_entry.pack()

    tk.Label(window, text="Minites (0-59):").pack(pady=5)
    minute_entry = tk.Entry(window)
    minute_entry.pack()

    submit_btn = tk.Button(window, text="set", command=submit_time)
    submit_btn.pack(pady=15)

    window.mainloop()

# =============== Start both ==================
if __name__ == "__main__":
    api_thread = threading.Thread(target=start_api, daemon=True)
    api_thread.start()
    start_gui()
