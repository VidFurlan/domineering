import sys
import shlex
import time

from PySide6.QtCore import QProcess, QTimer
from PySide6.QtWidgets import (
    QApplication,
    QWidget,
    QLabel,
    QLineEdit,
    QPushButton,
    QFileDialog,
    QHBoxLayout,
    QVBoxLayout,
    QPlainTextEdit,
    QSpinBox,
    QFormLayout,
)


class DomineeringRunner(QWidget):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Domineering Runner")
        self.resize(900, 600)

        # File select
        self.exe_edit = QLineEdit()
        self.exe_btn = QPushButton("Browse")

        exe_layout = QHBoxLayout()
        exe_layout.addWidget(QLabel("Executable:"))
        exe_layout.addWidget(self.exe_edit, 1)
        exe_layout.addWidget(self.exe_btn)

        # Args
        self.args_input = QLineEdit("--w 3 --h 3 --runs 1 --save-graph graph.json --format json")
        self.args_input.setPlaceholderText("--w 3 --h 3 --runs 1 --save-graph graph.json --format json")
        self.timout_input = QLineEdit("0")
        self.timout_input.setPlaceholderText("0 = no timeout")
        self.runs_input = QSpinBox()
        self.runs_input.setRange(1, 100)
        self.runs_input.setValue(1)

        args_layout = QFormLayout()
        args_layout.addRow("Args:", self.args_input)
        args_layout.addRow("Runs:", self.runs_input)
        args_layout.addRow("Timeout (ms):", self.timout_input)

        # Run
        self.run_btn = QPushButton("Run")
        self.stop_btn = QPushButton("Stop")
        self.stop_btn.setEnabled(False)
        self.status_label = QLabel("Idle")

        btn_layout = QHBoxLayout()
        btn_layout.addWidget(self.run_btn)
        btn_layout.addWidget(self.stop_btn)
        btn_layout.addStretch(1)
        btn_layout.addWidget(self.status_label)

        # Log
        self.log = QPlainTextEdit()
        self.log.setReadOnly(True)

        # Root
        root = QVBoxLayout(self)
        root.addLayout(exe_layout)
        root.addLayout(args_layout)
        root.addLayout(btn_layout)
        root.addWidget(self.log, 1)

        # Functionality
        self.proc = QProcess(self)
        self.proc.setProcessChannelMode(QProcess.SeparateChannels)

        self.proc.readyReadStandardOutput.connect(self.on_stdout)
        self.proc.readyReadStandardError.connect(self.on_stderr)
        self.proc.finished.connect(self.on_finished)

        self.exe_btn.clicked.connect(self.browse_exe)
        self.run_btn.clicked.connect(self.start_run)
        self.stop_btn.clicked.connect(self.stop_run)

        # Vars
        self._run_id = 0
        self._times_ms = []
        self._time_0 = None
        self._timeout_timer = QTimer(self)
        self._timeout_timer.setSingleShot(True)
        self._timeout_timer.timeout.connect(self.on_timeout)

    def output(self, msg: str):
        self.log.appendPlainText(f"{msg}\n")

    def output_error(self, msg: str):
        self.log.appendPlainText(f"❗️ {msg}")

    def browse_exe(self):
        path, _ = QFileDialog.getOpenFileName(self, "Select executable")
        if path:
            self.exe_edit.setText(path)

    def start_run(self):
        exe = self.exe_edit.text().strip()
        if not exe:
            self.output_error("No executable selected")
            return

        if self.proc.state() != QProcess.NotRunning:
            self.output_error("Process already running")
            return

        self._run_id = 0
        self._time_ms = []
        self.log.clear()
        self.run_btn.setEnabled(False)
        self.stop_btn.setEnabled(True)

        self.output("=== Run Started ===")
        self.run_once()

    def run_once(self):
        exe = self.exe_edit.text().strip()
        args_str = self.args_input.text().strip()
        args = shlex.split(args_str) if args_str else []

        self._run_id += 1

        timeout_ms = int(self.timout_input.text().strip() or "0")
        if timeout_ms > 0:
            self._timeout_timer.start(timeout_ms)
        else:
            self._timeout_timer.stop()

        self.status_label.setText(f"Running {self._run_id}/{self.runs_input.value()}")

        self.output(f"--- Run {self._run_id} ---")
        self.output(f"Exec command: {exe} {' '.join(args)}")
        self._time_0 = time.perf_counter()
        self.proc.start(exe, args)

    def stop_run(self):
        if self.proc.state() == QProcess.NotRunning:
            return
        self.output("Stopping...")
        self.proc.terminate()

    def on_timeout(self):
        self.output_error("Timeout reached. Killing process.")
        self.proc.kill()

    def on_stdout(self):
        data = self.proc.readAllStandardOutput().data().decode(errors="replace")
        if data:
            self.output(data)

    def on_stderr(self):
        data = self.proc.readAllStandardError().data().decode(errors="replace")
        if data:
            self.output_error(data)

    def on_finished(self, exit_code, exit_status):
        self._timeout_timer.stop()
        time_1 = time.perf_counter()
        time_delta = (time_1 - (self._time_0 or time_1)) * 1000.0

        self._times_ms.append(time_delta)
        self.output(
            f"Process finished with exit code {exit_code} in {time_delta:.3f} ms"
        )

        if self._run_id < self.runs_input.value():
            self.run_once()
            return

        self.run_btn.setEnabled(True)
        self.stop_btn.setEnabled(False)
        self.status_label.setText("Done")

        times = self._times_ms
        times = sorted(times)
        median = times[len(times) // 2]
        avg = sum(times) / len(times)

        self.output("\n=== Run Summary ===")
        self.output(f"Total runs: {len(times)}")
        self.output(f"Min time: {times[0]:.3f} ms")
        self.output(f"Max time: {times[-1]:.3f} ms")
        self.output(f"Median time: {median:.3f} ms")
        self.output(f"Average time: {avg:.3f} ms")
        self.output("")

        self._times_ms = []
        self._run_id = 0

if __name__ == "__main__":
    app = QApplication(sys.argv)
    w = DomineeringRunner()
    w.show()
    sys.exit(app.exec())
