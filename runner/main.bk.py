import sys
import shlex
import time

from PySide6.QtCore import QProcess, QTimer
from PySide6.QtWidgets import (
    QApplication, QWidget, QLabel, QLineEdit, QPushButton,
    QFileDialog, QHBoxLayout, QVBoxLayout, QPlainTextEdit,
    QSpinBox, QFormLayout
)


class DomineeringRunner(QWidget):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Domineering Runner")
        self.resize(900, 600)

        self.exe_edit = QLineEdit()
        self.exe_btn = QPushButton("Browse…")

        self.args_edit = QLineEdit()
        self.args_edit.setPlaceholderText("--w 6 --h 6 --method exact")

        self.runs_spin = QSpinBox()
        self.runs_spin.setRange(1, 10_000)
        self.runs_spin.setValue(5)

        self.timeout_edit = QLineEdit("0")
        self.timeout_edit.setPlaceholderText("0 = no timeout, otherwise ms")

        self.start_btn = QPushButton("Run")
        self.stop_btn = QPushButton("Stop")
        self.stop_btn.setEnabled(False)

        self.status_label = QLabel("Idle")
        self.log = QPlainTextEdit()
        self.log.setReadOnly(True)

        # --- Layout ---
        top_row = QHBoxLayout()
        top_row.addWidget(QLabel("Executable:"))
        top_row.addWidget(self.exe_edit, 1)
        top_row.addWidget(self.exe_btn)

        form = QFormLayout()
        form.addRow("Args:", self.args_edit)
        form.addRow("Runs:", self.runs_spin)
        form.addRow("Timeout (ms):", self.timeout_edit)

        btn_row = QHBoxLayout()
        btn_row.addWidget(self.start_btn)
        btn_row.addWidget(self.stop_btn)
        btn_row.addStretch(1)
        btn_row.addWidget(self.status_label)

        root = QVBoxLayout(self)
        root.addLayout(top_row)
        root.addLayout(form)
        root.addLayout(btn_row)
        root.addWidget(self.log, 1)

        # --- Process runner state ---
        self.proc = QProcess(self)
        self.proc.setProcessChannelMode(QProcess.SeparateChannels)

        self.proc.readyReadStandardOutput.connect(self.on_stdout)
        self.proc.readyReadStandardError.connect(self.on_stderr)
        self.proc.finished.connect(self.on_finished)

        self.exe_btn.clicked.connect(self.pick_exe)
        self.start_btn.clicked.connect(self.start_benchmark)
        self.stop_btn.clicked.connect(self.stop_running)

        self._run_index = 0
        self._durations_ms = []
        self._t0 = None
        self._timeout_timer = QTimer(self)
        self._timeout_timer.setSingleShot(True)
        self._timeout_timer.timeout.connect(self.on_timeout)

    def append(self, text: str):
        self.log.appendPlainText(text)

    def pick_exe(self):
        path, _ = QFileDialog.getOpenFileName(self, "Select executable")
        if path:
            self.exe_edit.setText(path)

    def start_benchmark(self):
        exe = self.exe_edit.text().strip()
        if not exe:
            self.append("❌ No executable selected.")
            return

        if self.proc.state() != QProcess.NotRunning:
            self.append("⚠️ Process is already running.")
            return

        self._run_index = 0
        self._durations_ms = []
        self.log.clear()

        self.start_btn.setEnabled(False)
        self.stop_btn.setEnabled(True)

        self.append("=== Benchmark started ===")
        self.run_once()

    def run_once(self):
        exe = self.exe_edit.text().strip()
        args_str = self.args_edit.text().strip()
        args = shlex.split(args_str) if args_str else []

        self._run_index += 1
        self.status_label.setText(f"Running {self._run_index}/{self.runs_spin.value()}")

        # Setup timeout if requested
        try:
            timeout_ms = int(self.timeout_edit.text().strip() or "0")
        except ValueError:
            timeout_ms = 0

        if timeout_ms > 0:
            self._timeout_timer.start(timeout_ms)
        else:
            self._timeout_timer.stop()

        self.append(f"\n--- Run #{self._run_index} ---")
        self.append(f"$ {exe} {args_str}")

        self._t0 = time.perf_counter()
        self.proc.start(exe, args)

    def stop_running(self):
        if self.proc.state() == QProcess.NotRunning:
            return
        self.append("\n🛑 Stopping…")
        self.proc.terminate()
        # If it doesn't terminate quickly, force kill
        QTimer.singleShot(500, lambda: self.proc.kill() if self.proc.state() != QProcess.NotRunning else None)

    def on_timeout(self):
        if self.proc.state() != QProcess.NotRunning:
            self.append("\n⏱️ Timeout reached. Killing process.")
            self.proc.kill()

    def on_stdout(self):
        data = self.proc.readAllStandardOutput().data().decode(errors="replace")
        if data:
            self.append(data.rstrip("\n"))

    def on_stderr(self):
        data = self.proc.readAllStandardError().data().decode(errors="replace")
        if data:
            self.append("[stderr] " + data.rstrip("\n"))

    def on_finished(self, exit_code, exit_status):
        self._timeout_timer.stop()

        t1 = time.perf_counter()
        dt_ms = (t1 - (self._t0 or t1)) * 1000.0
        self._durations_ms.append(dt_ms)

        self.append(f"\n✅ Finished. exit_code={exit_code}, time={dt_ms:.3f} ms")

        total_runs = self.runs_spin.value()
        if self._run_index < total_runs:
            # start next run
            self.run_once()
            return

        # Done
        self.start_btn.setEnabled(True)
        self.stop_btn.setEnabled(False)
        self.status_label.setText("Done")

        # Summary stats
        d = self._durations_ms
        d_sorted = sorted(d)
        mean = sum(d) / len(d)
        median = d_sorted[len(d_sorted)//2] if len(d_sorted) % 2 == 1 else 0.5*(d_sorted[len(d_sorted)//2 - 1] + d_sorted[len(d_sorted)//2])

        self.append("\n=== Summary ===")
        self.append(f"runs: {len(d)}")
        self.append(f"min : {min(d):.3f} ms")
        self.append(f"mean: {mean:.3f} ms")
        self.append(f"med : {median:.3f} ms")
        self.append(f"max : {max(d):.3f} ms")


if __name__ == "__main__":
    app = QApplication(sys.argv)
    w = DomineeringRunner()
    w.show()
    sys.exit(app.exec())


