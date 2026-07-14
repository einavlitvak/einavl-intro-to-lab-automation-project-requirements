import queue
import threading
from typing import Optional

import serial
import serial.tools.list_ports

try:
    import FreeSimpleGUI as sg
except ImportError:
    import PySimpleGUI as sg

MIN_DURATION_MS = 1
MAX_DURATION_MS = 600000
DEFAULT_BAUD = 9600

STATE_MESSAGES = {
    "0": "LED off",
    "1": "Button pressed, LED on",
    "2": "Button released",
}


class SerialReader(threading.Thread):
    def __init__(self, ser: serial.Serial, out_queue: queue.Queue, stop_event: threading.Event):
        super().__init__(daemon=True)
        self.ser = ser
        self.out_queue = out_queue
        self.stop_event = stop_event

    def run(self) -> None:
        while not self.stop_event.is_set():
            try:
                raw = self.ser.readline()
                if not raw:
                    continue
                line = raw.decode(errors="replace").strip()
                if line:
                    self.out_queue.put(("rx", line))
            except Exception as exc:
                self.out_queue.put(("error", f"Serial read error: {exc}"))
                break


def available_ports() -> list[str]:
    return [port.device for port in serial.tools.list_ports.comports()]


def build_window() -> sg.Window:
    ports = available_ports()
    layout = [
        [sg.Text("COM Port"), sg.Combo(ports, default_value=ports[0] if ports else "", key="-PORT-", size=(16, 1)), sg.Button("Refresh Ports")],
        [sg.Text("Baud"), sg.Combo(["9600", "115200"], default_value=str(DEFAULT_BAUD), key="-BAUD-", size=(16, 1)), sg.Button("Connect"), sg.Button("Disconnect", disabled=True)],
        [sg.Text("LED ON time (ms)"), sg.Input("1000", key="-DURATION-", size=(18, 1)), sg.Button("Send", disabled=True)],
        [sg.Multiline("", key="-LOG-", size=(70, 20), autoscroll=True, disabled=True)],
        [sg.Button("Exit")],
    ]
    return sg.Window("Project 8 Mini Project 1", layout, finalize=True)


def append_log(window: sg.Window, text: str) -> None:
    current = window["-LOG-"].get()
    window["-LOG-"].update(current + text + "\n")


def parse_duration(value: str) -> Optional[int]:
    try:
        parsed = int(value.strip())
    except ValueError:
        return None
    if parsed < MIN_DURATION_MS or parsed > MAX_DURATION_MS:
        return None
    return parsed


def main() -> None:
    window = build_window()
    message_queue: queue.Queue = queue.Queue()

    ser: Optional[serial.Serial] = None
    reader_thread: Optional[SerialReader] = None
    stop_event = threading.Event()

    while True:
        event, values = window.read(timeout=100)

        if event in (sg.WIN_CLOSED, "Exit"):
            break

        if event == "Refresh Ports":
            ports = available_ports()
            window["-PORT-"].update(values=ports)
            append_log(window, "RX: Port list refreshed")

        if event == "Connect":
            if ser and ser.is_open:
                append_log(window, "RX: Already connected")
                continue

            port = (values.get("-PORT-") or "").strip()
            baud_text = (values.get("-BAUD-") or str(DEFAULT_BAUD)).strip()

            if not port:
                append_log(window, "RX: Please choose a COM port")
                continue

            try:
                baud = int(baud_text)
                ser = serial.Serial(port=port, baudrate=baud, timeout=0.2)
                stop_event.clear()
                reader_thread = SerialReader(ser, message_queue, stop_event)
                reader_thread.start()

                window["Send"].update(disabled=False)
                window["Disconnect"].update(disabled=False)
                window["Connect"].update(disabled=True)
                append_log(window, f"RX: Connected to {port} at {baud}")
            except Exception as exc:
                append_log(window, f"RX: Connection failed: {exc}")

        if event == "Disconnect":
            if ser and ser.is_open:
                stop_event.set()
                if reader_thread:
                    reader_thread.join(timeout=1.0)
                ser.close()
                append_log(window, "RX: Disconnected")

            ser = None
            reader_thread = None
            window["Send"].update(disabled=True)
            window["Disconnect"].update(disabled=True)
            window["Connect"].update(disabled=False)

        if event == "Send":
            if not ser or not ser.is_open:
                append_log(window, "RX: Not connected")
                continue

            duration_text = values.get("-DURATION-", "")
            duration_ms = parse_duration(duration_text)
            if duration_ms is None:
                append_log(window, f"RX: Invalid duration. Enter {MIN_DURATION_MS}..{MAX_DURATION_MS} ms")
                continue

            command = f"{duration_ms}\n"
            try:
                ser.write(command.encode("ascii"))
                append_log(window, f"TX: {duration_ms}")
            except Exception as exc:
                append_log(window, f"RX: Send failed: {exc}")

        while not message_queue.empty():
            msg_type, payload = message_queue.get_nowait()
            if msg_type == "error":
                append_log(window, f"RX: {payload}")
                continue

            if payload in STATE_MESSAGES:
                append_log(window, f"RX: {payload} ({STATE_MESSAGES[payload]})")
            else:
                append_log(window, f"RX: {payload}")

    stop_event.set()
    if reader_thread:
        reader_thread.join(timeout=1.0)
    if ser and ser.is_open:
        ser.close()

    window.close()


if __name__ == "__main__":
    main()
