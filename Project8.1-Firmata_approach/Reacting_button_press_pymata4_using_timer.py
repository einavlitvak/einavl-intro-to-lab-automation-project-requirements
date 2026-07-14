"""Project 8.1: react to a button press with pymata4 and threading.Timer.

The script uses FirmataExpress on the Arduino side and pymata4 on the Python
side. A GUI shows the current button state, LED state, and an action log. The
user can also change the LED-on interval from the GUI.
"""

from __future__ import annotations

import threading
import time
from typing import Optional

from pymata4 import pymata4

try:
    import FreeSimpleGUI as sg
except ImportError:  # pragma: no cover - fallback for alternate installs
    import PySimpleGUI as sg


# Hardware configuration. The button uses pin 6 and the LED uses pin 4.
BUTTON_PIN = 6
LED_PIN = 4

# Project 8.1 measurement uses 30 ms, so make that the default interval.
DEFAULT_INTERVAL_MS = 30

# Notebook example used COM4. Keep it editable from the GUI.
DEFAULT_BOARD_PORT = "COM4"


class FirmataTimerApp:
    """GUI wrapper for the FirmataExpress + pymata4 timer demo."""

    def __init__(self) -> None:
        self.window = self._build_window()
        self.board: Optional[pymata4.Pymata4] = None
        self.timer: Optional[threading.Timer] = None
        self.state_lock = threading.Lock()

        self.interval_ms = DEFAULT_INTERVAL_MS
        self.button_state = "Released"
        self.led_state = "Off"
        self.connected_port = "Disconnected"

        self._append_log("Starting Project 8.1 Firmata timer app.")
        self._update_status_labels()
        self._connect_board(DEFAULT_BOARD_PORT)

    def _build_window(self) -> sg.Window:
        """Create the GUI layout."""

        layout = [
            [
                sg.Text("Board port"),
                sg.Input(DEFAULT_BOARD_PORT, key="-PORT-", size=(12, 1)),
                sg.Button("Connect"),
                sg.Button("Disconnect"),
            ],
            [
                sg.Text("LED interval (ms)"),
                sg.Input(str(DEFAULT_INTERVAL_MS), key="-INTERVAL-", size=(12, 1)),
                sg.Button("Set Interval"),
            ],
            [sg.Text("Button state: Released", key="-BUTTON_STATE-", size=(40, 1))],
            [sg.Text("LED state: Off", key="-LED_STATE-", size=(40, 1))],
            [sg.Text("Board: Disconnected", key="-BOARD_STATE-", size=(40, 1))],
            [
                sg.Multiline(
                    "",
                    key="-LOG-",
                    size=(90, 18),
                    autoscroll=True,
                    disabled=True,
                )
            ],
            [sg.Button("Exit")],
        ]

        return sg.Window(
            "Project 8.1 - pymata4 button timer",
            layout,
            finalize=True,
        )

    def _append_log(self, message: str) -> None:
        """Append a message to the log area and mirror it to the console."""

        timestamp = time.strftime("%H:%M:%S")
        line = f"[{timestamp}] {message}"
        print(line)

        if self.window is None:
            return

        current_text = self.window["-LOG-"].get()
        self.window["-LOG-"].update(current_text + line + "\n")

    def _update_status_labels(self) -> None:
        """Refresh the GUI labels for button, LED, and board state."""

        if self.window is None:
            return

        self.window["-BUTTON_STATE-"].update(f"Button state: {self.button_state}")
        self.window["-LED_STATE-"].update(f"LED state: {self.led_state}")
        self.window["-BOARD_STATE-"].update(f"Board: {self.connected_port}")

    def _parse_interval(self, raw_value: str) -> Optional[int]:
        """Validate the LED interval typed into the GUI."""

        try:
            interval_ms = int(raw_value.strip())
        except ValueError:
            return None

        if interval_ms <= 0:
            return None

        return interval_ms

    def _connect_board(self, port: str) -> None:
        """Connect to FirmataExpress and register the button callback."""

        self._disconnect_board(silent=True)

        try:
            self._append_log(f"Connecting to Arduino on {port}...")
            board = pymata4.Pymata4(com_port=port)
            board.set_pin_mode_digital_output(LED_PIN)
            board.digital_write(LED_PIN, 0)
            board.set_pin_mode_digital_input_pullup(
                BUTTON_PIN, callback=self._button_callback
            )
        except Exception as exc:  # pragma: no cover - hardware/port specific
            self.board = None
            self.connected_port = "Disconnected"
            self._update_status_labels()
            self._append_log(f"Connection failed: {exc}")
            return

        with self.state_lock:
            self.board = board
            self.connected_port = port
            self.button_state = "Released"
            self.led_state = "Off"

        self._update_status_labels()
        self._append_log(f"Connected to Arduino on {port}.")

    def _disconnect_board(self, silent: bool = False) -> None:
        """Cancel any active timer and shut down the board cleanly."""

        with self.state_lock:
            if self.timer is not None:
                self.timer.cancel()
                self.timer = None

            board = self.board
            self.board = None
            self.connected_port = "Disconnected"
            self.led_state = "Off"

        if board is not None:
            try:
                board.digital_write(LED_PIN, 0)
            except Exception:
                pass
            try:
                board.shutdown()
            except Exception:
                pass

        self._update_status_labels()
        if not silent:
            self._append_log("Board disconnected.")

    def _start_timer_locked(self) -> None:
        """Start the timer while holding the state lock."""

        if self.timer is not None:
            self.timer.cancel()

        timer = threading.Timer(self.interval_ms / 1000.0, self._timer_expired)
        timer.daemon = True
        self.timer = timer
        timer.start()

    def _button_callback(self, data) -> None:
        """Handle digital input changes from pymata4.

        The callback receives [pin_type, pin_number, pin_value, timestamp].
        With pullup enabled, a 0 means the button is pressed and a 1 means it
        is released.
        """

        _, pin_number, pin_value, timestamp = data
        pressed = pin_value == 0

        action_message = ""
        with self.state_lock:
            if pressed:
                self.button_state = "Pressed"

                if self.board is not None and self.led_state == "Off":
                    self.board.digital_write(LED_PIN, 1)
                    self.led_state = "On"
                    self._start_timer_locked()
                    action_message = (
                        f"Button pressed on pin {pin_number}; LED turned on for "
                        f"{self.interval_ms} ms."
                    )
                else:
                    action_message = (
                        f"Button pressed on pin {pin_number}; LED already on, "
                        "press ignored."
                    )
            else:
                self.button_state = "Released"
                action_message = f"Button released on pin {pin_number}."

        self.window.write_event_value(
            "-BUTTON_EVENT-",
            {
                "pressed": pressed,
                "timestamp": timestamp,
                "message": action_message,
            },
        )

    def _timer_expired(self) -> None:
        """Turn the LED off after the configured delay."""

        with self.state_lock:
            if self.board is None:
                return

            try:
                self.board.digital_write(LED_PIN, 0)
            except Exception as exc:  # pragma: no cover - hardware specific
                self.window.write_event_value(
                    "-TIMER_EVENT-",
                    {"message": f"Timer expired, but LED off failed: {exc}"},
                )
                self.led_state = "Off"
                self.timer = None
                return

            self.led_state = "Off"
            self.timer = None

        self.window.write_event_value(
            "-TIMER_EVENT-",
            {"message": "Timer expired; LED turned off."},
        )

    def run(self) -> None:
        """Main GUI event loop."""

        while True:
            event, values = self.window.read(timeout=100)

            if event in (sg.WIN_CLOSED, "Exit"):
                break

            if event == "Connect":
                port = values.get("-PORT-", DEFAULT_BOARD_PORT).strip() or DEFAULT_BOARD_PORT
                self._connect_board(port)
                continue

            if event == "Disconnect":
                self._disconnect_board()
                continue

            if event == "Set Interval":
                interval_ms = self._parse_interval(values.get("-INTERVAL-", ""))
                if interval_ms is None:
                    self._append_log("Invalid interval. Enter a positive integer in ms.")
                else:
                    with self.state_lock:
                        self.interval_ms = interval_ms
                    self._append_log(f"Timer interval set to {interval_ms} ms.")
                continue

            if event == "-BUTTON_EVENT-":
                info = values[event]
                state_label = "Pressed" if info["pressed"] else "Released"
                self.button_state = state_label
                self._update_status_labels()
                self._append_log(info["message"])
                continue

            if event == "-TIMER_EVENT-":
                self.led_state = "Off"
                self._update_status_labels()
                self._append_log(values[event]["message"])

        self._disconnect_board(silent=True)
        self.window.close()


def main() -> None:
    """Program entry point."""

    app = FirmataTimerApp()
    app.run()


if __name__ == "__main__":
    main()