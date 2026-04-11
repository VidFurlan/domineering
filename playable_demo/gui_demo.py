import json
import subprocess
import sys
from dataclasses import dataclass
from typing import Optional, Tuple

from PySide6.QtCore import Qt, QSize
from PySide6.QtGui import QColor, QPainter, QPen, QBrush, QFont
from PySide6.QtWidgets import (
    QApplication, QWidget, QMainWindow, QVBoxLayout, QHBoxLayout,
    QPushButton, QLabel, QSpinBox, QMessageBox, QFileDialog
)


@dataclass
class BestMove:
    r: int
    c: int
    dir: str  # "H" or "V"


@dataclass
class SolveResult:
    win: bool
    best_move: Optional[BestMove]
    nodes: int
    hits: int
    ms: int


class DomineeringBoardWidget(QWidget):
    def __init__(self, parent=None):
        super().__init__(parent)

        self.w = 6
        self.h = 6
        self.cell_size = 56
        self.margin = 12

        self.occupied = 0
        self.vertical_bb = 0   # first player
        self.horizontal_bb = 0 # second player
        self.turn = 0          # 0 = vertical, 1 = horizontal

        self.selected: Optional[Tuple[int, int]] = None
        self.hint_move: Optional[BestMove] = None
        self.history = []

        self.setMinimumSize(QSize(
            self.margin * 2 + self.cell_size * self.w,
            self.margin * 2 + self.cell_size * self.h + 30
        ))

    def set_size(self, w: int, h: int):
        self.w = w
        self.h = h
        self.reset()

    def reset(self):
        self.occupied = 0
        self.vertical_bb = 0
        self.horizontal_bb = 0
        self.turn = 0
        self.selected = None
        self.hint_move = None
        self.history.clear()
        self.updateGeometry()
        self.update()

    def undo(self):
        if not self.history:
            return
        self.occupied, self.vertical_bb, self.horizontal_bb, self.turn = self.history.pop()
        self.selected = None
        self.hint_move = None
        self.update()

    def bit_index(self, r: int, c: int) -> int:
        return r * self.w + c

    def get_bit(self, r: int, c: int) -> int:
        return (self.occupied >> self.bit_index(r, c)) & 1

    def in_bounds(self, r: int, c: int) -> bool:
        return 0 <= r < self.h and 0 <= c < self.w

    def can_place(self, r: int, c: int, dir_: str, turn: Optional[int] = None) -> bool:
        if turn is None:
            turn = self.turn

        if not self.in_bounds(r, c):
            return False
        if self.get_bit(r, c):
            return False

        if dir_ == "V":
            if turn != 0:
                return False
            r2, c2 = r + 1, c
        else:
            if turn != 1:
                return False
            r2, c2 = r, c + 1

        if not self.in_bounds(r2, c2):
            return False
        if self.get_bit(r2, c2):
            return False

        return True

    def place(self, r: int, c: int, dir_: str) -> bool:
        if not self.can_place(r, c, dir_):
            return False

        self.history.append((self.occupied, self.vertical_bb, self.horizontal_bb, self.turn))

        idx1 = self.bit_index(r, c)
        if dir_ == "V":
            idx2 = self.bit_index(r + 1, c)
        else:
            idx2 = self.bit_index(r, c + 1)

        mask = (1 << idx1) | (1 << idx2)
        self.occupied |= mask

        if self.turn == 0:
            self.vertical_bb |= mask
        else:
            self.horizontal_bb |= mask

        self.turn ^= 1
        self.selected = None
        self.hint_move = None
        self.update()
        return True

    def pixel_to_cell(self, x: int, y: int) -> Optional[Tuple[int, int]]:
        x -= self.margin
        y -= self.margin
        if x < 0 or y < 0:
            return None
        c = int(x // self.cell_size)
        r = int(y // self.cell_size)
        if not self.in_bounds(r, c):
            return None
        return r, c

    def mousePressEvent(self, event):
        cell = self.pixel_to_cell(event.position().x(), event.position().y())
        if cell is None:
            return

        r, c = cell
        mods = event.modifiers()

        # quick-place with modifiers
        if mods & Qt.ShiftModifier:
            self.place(r, c, "H")
            return
        if mods & Qt.ControlModifier:
            self.place(r, c, "V")
            return

        # two-click placement
        if self.selected is None:
            self.selected = (r, c)
            self.update()
            return

        r0, c0 = self.selected
        self.selected = None

        # infer direction from adjacency
        if r == r0 and abs(c - c0) == 1:
            cc = min(c, c0)
            self.place(r0, cc, "H")
        elif c == c0 and abs(r - r0) == 1:
            rr = min(r, r0)
            self.place(rr, c0, "V")
        else:
            self.selected = (r, c)

        self.update()

    def paintEvent(self, event):
        painter = QPainter(self)
        painter.setRenderHint(QPainter.Antialiasing, True)

        painter.fillRect(self.rect(), QColor(22, 22, 26))

        board_x = self.margin
        board_y = self.margin
        board_w = self.w * self.cell_size
        board_h = self.h * self.cell_size

        painter.setBrush(QBrush(QColor(38, 38, 46)))
        painter.setPen(QPen(QColor(70, 70, 80), 2))
        painter.drawRoundedRect(board_x, board_y, board_w, board_h, 10, 10)

        for r in range(self.h):
            for c in range(self.w):
                x = board_x + c * self.cell_size
                y = board_y + r * self.cell_size
                idx = self.bit_index(r, c)

                is_v = ((self.vertical_bb >> idx) & 1) == 1
                is_h = ((self.horizontal_bb >> idx) & 1) == 1
                is_sel = self.selected == (r, c)

                hint_here = False
                if self.hint_move is not None:
                    hm = self.hint_move
                    if hm.dir == "V":
                        hint_here = (r, c) in [(hm.r, hm.c), (hm.r + 1, hm.c)]
                    else:
                        hint_here = (r, c) in [(hm.r, hm.c), (hm.r, hm.c + 1)]

                if is_v:
                    painter.setBrush(QBrush(QColor(80, 140, 255)))   # blue
                elif is_h:
                    painter.setBrush(QBrush(QColor(230, 90, 90)))    # red
                else:
                    painter.setBrush(QBrush(QColor(60, 60, 72)))

                if is_sel:
                    painter.setPen(QPen(QColor(245, 210, 90), 3))
                else:
                    painter.setPen(QPen(QColor(78, 78, 90), 2))

                painter.drawRoundedRect(x + 5, y + 5, self.cell_size - 10, self.cell_size - 10, 10, 10)

                if hint_here and not is_v and not is_h:
                    painter.setBrush(QBrush(QColor(90, 220, 120, 120)))
                    painter.setPen(Qt.NoPen)
                    painter.drawRoundedRect(x + 8, y + 8, self.cell_size - 16, self.cell_size - 16, 10, 10)

        painter.setPen(QPen(QColor(225, 225, 235)))
        painter.setFont(QFont("Sans", 10, QFont.Bold))
        turn_txt = "First / Vertical" if self.turn == 0 else "Second / Horizontal"
        painter.drawText(12, self.height() - 8, f"Turn: {turn_txt} | occupied = 0x{self.occupied:x}")


class MainWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Domineering Demo")

        self.board = DomineeringBoardWidget()
        self.solver_path = "./Generator"

        self.size_w = QSpinBox()
        self.size_w.setRange(1, 6)
        self.size_w.setValue(6)

        self.size_h = QSpinBox()
        self.size_h.setRange(1, 6)
        self.size_h.setValue(6)

        self.btn_reset = QPushButton("Reset")
        self.btn_undo = QPushButton("Undo")
        self.btn_solve = QPushButton("Solve")
        self.btn_hint = QPushButton("Hint")
        self.btn_ai = QPushButton("AI Move")

        self.status = QLabel("Ready.")
        self.status.setStyleSheet("color: #d8d8e8; padding: 6px;")

        self.size_w.valueChanged.connect(self.on_resize)
        self.size_h.valueChanged.connect(self.on_resize)
        self.btn_reset.clicked.connect(self.on_reset)
        self.btn_undo.clicked.connect(self.on_undo)
        self.btn_solve.clicked.connect(self.on_solve)
        self.btn_hint.clicked.connect(self.on_hint)
        self.btn_ai.clicked.connect(self.on_ai)

        self.btn_pick_solver = QPushButton("Pick Generator")
        self.solver_label = QLabel(self.solver_path)
        self.solver_label.setStyleSheet("color: #c8c8d8; padding: 4px;")
        self.solver_label.setTextInteractionFlags(Qt.TextSelectableByMouse)
        self.btn_pick_solver.clicked.connect(self.on_pick_solver)

        top = QHBoxLayout()
        top.addWidget(QLabel("W:"))
        top.addWidget(self.size_w)
        top.addWidget(QLabel("H:"))
        top.addWidget(self.size_h)
        top.addStretch(1)
        top.addWidget(self.btn_undo)
        top.addWidget(self.btn_reset)

        actions = QHBoxLayout()
        actions.addWidget(self.btn_solve)
        actions.addWidget(self.btn_hint)
        actions.addWidget(self.btn_ai)

        solver_row = QHBoxLayout()
        solver_row.addWidget(self.btn_pick_solver)
        solver_row.addWidget(self.solver_label, 1)

        layout = QVBoxLayout()
        layout.addLayout(top)
        layout.addLayout(solver_row)
        layout.addWidget(self.board)
        layout.addLayout(actions)
        layout.addWidget(self.status)

        root = QWidget()
        root.setLayout(layout)
        self.setCentralWidget(root)

        self.on_resize()

    def on_resize(self):
        self.board.set_size(self.size_w.value(), self.size_h.value())

    def on_reset(self):
        self.board.reset()
        self.status.setText("Reset.")

    def on_undo(self):
        self.board.undo()
        self.status.setText("Undo.")

    def call_solver(self, want_best_move: bool) -> SolveResult:
        args = [
            self.solver_path,
            "--w", str(self.board.w),
            "--h", str(self.board.h),
            "--algo", "ttmonc",
            "--board", hex(self.board.occupied),
            "--turn", str(self.board.turn),
            "--json",
        ]
        if want_best_move:
            args.append("--best-move")

        try:
            cp = subprocess.run(args, capture_output=True, text=True, check=True)
        except FileNotFoundError:
            raise RuntimeError(f"Solver not found: {self.solver_path}")
        except subprocess.CalledProcessError as e:
            raise RuntimeError(f"Solver failed.\nstdout:\n{e.stdout}\nstderr:\n{e.stderr}")

        data = json.loads(cp.stdout.strip())

        best_move = None
        if data.get("bestMove") is not None:
            best_move = BestMove(
                r=int(data["bestMove"]["r"]),
                c=int(data["bestMove"]["c"]),
                dir=str(data["bestMove"]["dir"])
            )

        return SolveResult(
            win=bool(data["win"]),
            best_move=best_move,
            nodes=int(data["nodes"]),
            hits=int(data["hits"]),
            ms=int(data.get("ms", 0)),
        )

    def on_solve(self):
        try:
            res = self.call_solver(want_best_move=False)
        except Exception as e:
            QMessageBox.critical(self, "Solver error", str(e))
            return

        hitrate = (res.hits / res.nodes) if res.nodes else 0.0
        self.status.setText(
            f"Winning for side to move: {res.win} | nodes={res.nodes} | hits={res.hits} | hitrate={hitrate:.2%}"
        )

    def on_hint(self):
        try:
            res = self.call_solver(want_best_move=True)
        except Exception as e:
            QMessageBox.critical(self, "Solver error", str(e))
            return

        self.board.hint_move = res.best_move
        self.board.update()

        if res.best_move is None:
            self.status.setText("No legal move.")
        else:
            hitrate = (res.hits / res.nodes) if res.nodes else 0.0
            self.status.setText(
                f"Hint: ({res.best_move.r}, {res.best_move.c}, {res.best_move.dir}) | nodes={res.nodes} | hitrate={hitrate:.2%}"
            )

    def on_ai(self):
        try:
            res = self.call_solver(want_best_move=True)
        except Exception as e:
            QMessageBox.critical(self, "Solver error", str(e))
            return

        if res.best_move is None:
            self.status.setText("No legal move for AI.")
            return

        ok = self.board.place(res.best_move.r, res.best_move.c, res.best_move.dir)
        if not ok:
            self.status.setText("AI move illegal. Check CLI / state encoding.")
            return

        self.status.setText(
            f"AI played: ({res.best_move.r}, {res.best_move.c}, {res.best_move.dir})"
        )

    def on_pick_solver(self):
        path, _ = QFileDialog.getOpenFileName(
            self,
            "Select Generator executable",
            self.solver_path if self.solver_path else "."
        )
        if not path:
            return

        self.solver_path = path
        self.solver_label.setText(path)
        self.status.setText(f"Using solver: {path}")

def main():
    app = QApplication(sys.argv)
    win = MainWindow()
    win.resize(520, 650)
    win.show()
    sys.exit(app.exec())


if __name__ == "__main__":
    main()
