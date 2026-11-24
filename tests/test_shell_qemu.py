#!/usr/bin/env python3
"""Exercise the real GOS shell through its PS/2 keyboard and VGA text buffer.

Run `make build-mac` first, then `python3 tests/test_shell_qemu.py`.
Uses a temporary filesystem with permission fixtures and snapshot disks;
neither img/gos.img nor img/fs.img is modified.
"""
import argparse
import json
from pathlib import Path
import re
import socket
import struct
import subprocess
import tempfile
import time


class QemuShell:
    def __init__(self, repo):
        self.repo = Path(repo).resolve()
        self.logs = Path(tempfile.mkdtemp(prefix="gos-shell-tests-", dir="/private/tmp"))
        self.socket_path = self.logs / "qmp.sock"
        self.process = None
        self.stream = None
        self.connection = None
        self.cases = 0

    def __enter__(self):
        # Build a fresh disposable volume, including directories for permission errors.
        # Match the production 128-byte inode format and filesystem block layout.
        filesystem = self.logs / "fs.img"
        size = (self.repo / "img/fs.img").stat().st_size
        total_blocks = size // 512
        remaining = total_blocks - 6 - 5120
        bitmap_blocks = remaining // 4097
        data_start = 6 + 5120 + bitmap_blocks
        with filesystem.open("w+b") as disk:
            disk.truncate(size)
            disk.write(struct.pack("<6I", 0x20001209, total_blocks, 5, 5120,
                                   bitmap_blocks, remaining - bitmap_blocks))
            disk.seek(512)
            disk.write(bytes([15]))  # root and three permission fixture inodes
            disk.seek((6 + 5120) * 512)
            disk.write(bytes([1]))  # root directory data block
            for inode, mode in enumerate((0o755, 0o000, 0o500, 0o300)):
                blocks = [data_start] + [0] * 26 if inode == 0 else [0] * 27
                disk.seek(6 * 512 + inode * 128)
                disk.write(struct.pack("<IIHH29I", 2, 96 if inode == 0 else 0,
                                       1, mode, *blocks, 0, 0))
            disk.seek(data_start * 512)
            for inode, name in enumerate((b"denied", b"readonly", b"writeonly"), start=1):
                disk.write(struct.pack("<I28s", inode, name))
        self.process = subprocess.Popen([
            "qemu-system-i386", "-m", "32M", "-display", "none", "-nic", "none",
            "-no-reboot", "-no-shutdown", "-monitor", "none",
            "-qmp", f"unix:{self.socket_path},server=on,wait=off", "-snapshot",
            "-drive", f"file={self.repo / 'img/gos.img'},if=ide,index=0,media=disk,format=raw",
            "-drive", f"file={filesystem},if=ide,index=1,media=disk,format=raw",
        ], stdout=(self.logs / "qemu.stdout").open("w"),
            stderr=(self.logs / "qemu.stderr").open("w"))
        try:
            deadline = time.monotonic() + 10
            while not self.socket_path.exists():
                if self.process.poll() is not None:
                    raise RuntimeError((self.logs / "qemu.stderr").read_text())
                if time.monotonic() > deadline:
                    raise TimeoutError("QEMU QMP socket did not appear")
                time.sleep(0.05)
            self.connection = socket.socket(socket.AF_UNIX)
            self.connection.settimeout(10)
            self.connection.connect(str(self.socket_path))
            self.stream = self.connection.makefile("rwb", buffering=0)
            json.loads(self.stream.readline())
            self.qmp("qmp_capabilities")
            self.wait_prompt(timeout=20)
            return self
        except BaseException:
            self.__exit__(None, None, None)
            raise

    def __exit__(self, *unused):
        if self.process and self.process.poll() is None:
            self.process.terminate()
            try:
                self.process.wait(timeout=5)
            except subprocess.TimeoutExpired:
                self.process.kill()
                self.process.wait()
        if self.stream:
            self.stream.close()
        if self.connection:
            self.connection.close()
        print(f"QEMU logs: {self.logs}")

    def qmp(self, name, **arguments):
        self.stream.write((json.dumps({"execute": name, "arguments": arguments}) + "\n").encode())
        while True:
            reply = json.loads(self.stream.readline())
            if "error" in reply:
                raise RuntimeError(reply["error"])
            if "return" in reply:
                return reply["return"]

    def monitor(self, command):
        return self.qmp("human-monitor-command", **{"command-line": command})

    def memory(self):
        filename = self.logs / "vga.bin"
        result = self.monitor(f'pmemsave 0xb8000 0x4000 "{filename}"')
        if result:
            raise RuntimeError(result)
        return filename.read_bytes()

    def screen(self):
        return self.memory()[::2].decode("ascii", errors="replace")

    @staticmethod
    def prompt(text):
        matches = list(re.finditer(r"JackHu@JackHu-PC:([^$\x00]*?)\$ ", text))
        return matches[-1] if matches else None

    def wait_prompt(self, timeout=5):
        deadline = time.monotonic() + timeout
        while time.monotonic() < deadline:
            text = self.screen()
            match = self.prompt(text)
            if match and not text[match.end():].strip(" \x00"):
                return text, match.group(1)
            time.sleep(0.05)
        self.save_screen("timeout", text)
        raise AssertionError(f"shell prompt timed out, see {self.logs}")

    def key(self, qcode):
        self.monitor(f"sendkey {qcode} 1")
        time.sleep(0.012)

    def type(self, text):
        codes = {" ": "spc", "/": "slash", ".": "dot", "-": "minus", "_": "shift-minus",
                 "~": "shift-grave_accent", ">": "shift-dot", "\t": "tab", "\b": "backspace",
                 '"': "shift-apostrophe", "'": "apostrophe"}
        for ch in text:
            if "A" <= ch <= "Z":
                code = "shift-" + ch.lower()
            else:
                code = codes.get(ch, ch)
            self.key(code)

    def save_screen(self, name, text=None):
        if text is None:
            text = self.screen()
        formatted = "\n".join(text[i:i + 80].rstrip(" \x00") for i in range(0, len(text), 80))
        (self.logs / f"{self.cases:03d}-{name}.txt").write_text(formatted.rstrip() + "\n")

    def command(self, line, contains=(), absent=(), cwd=None, clear=True):
        if clear:
            self.type("clear")
            self.key("ret")
            time.sleep(0.05)
            self.wait_prompt()
        self.type(line)
        self.key("ret")
        time.sleep(0.06)
        text, actual_cwd = self.wait_prompt()
        self.cases += 1
        self.save_screen("command", text)
        match = self.prompt(text)
        body = text[:match.start()]
        # Exclude the command echo, including screen-wrapped long input.
        echo = body.find(line)
        output = body[echo + len(line):] if echo >= 0 else body
        output = re.sub(r"JackHu@JackHu-PC:([^$\x00]*?)\$ ", "", output)
        for expected in contains:
            assert expected in output, f"{line!r}: missing {expected!r}; screen: {self.logs}"
        for unexpected in absent:
            assert unexpected not in output, f"{line!r}: unexpected {unexpected!r}; screen: {self.logs}"
        if cwd is not None:
            assert actual_cwd == cwd, f"{line!r}: cwd {actual_cwd!r}, expected {cwd!r}"
        print(f"PASS {self.cases:02d} {line!r}", flush=True)
        return output


def run_suite(shell):
    run = shell.command
    run("pwd", contains=("/",), cwd="/")
    run("cd /denied", contains=("Permission denied",), cwd="/")
    run("cd /denied/..", contains=("Permission denied",), cwd="/")
    run("mkdir /denied/new", contains=("Permission denied",), cwd="/")
    run("ls /denied", contains=("Permission denied",), cwd="/")
    run("cd /readonly", cwd="/readonly")
    run("mkdir child", contains=("Permission denied",), cwd="/readonly")
    run("touch file", contains=("Permission denied",), cwd="/readonly")
    run("cd /writeonly", cwd="/writeonly")
    run("ls", contains=("Permission denied",), cwd="/writeonly")
    run("cd /", cwd="/")
    run("mkdir qa", cwd="/")
    run("mkdir qa", contains=("File exists",), cwd="/")
    run("mkdir /qa/one", cwd="/")
    run("mkdir /qa/one/two", cwd="/")
    run("cd qa/one/two", cwd="/qa/one/two")
    run("pwd", contains=("/qa/one/two",), cwd="/qa/one/two")
    run("cd ..", cwd="/qa/one")
    run("cd ./two/.././", cwd="/qa/one")
    run("cd /qa//one/two/", cwd="/qa/one/two")
    run("mkdir ../sibling", cwd="/qa/one/two")
    run("cd ../sibling", cwd="/qa/one/sibling")
    run("cd missing", contains=("No such file or directory",), cwd="/qa/one/sibling")
    run("cd ../../missing", contains=("No such file or directory",), cwd="/qa/one/sibling")
    run("touch note", cwd="/qa/one/sibling")
    run("echo hello > note", cwd="/qa/one/sibling")
    run("cat note", contains=("hello",))
    run('echo "hello world" > note')
    run("cat ../sibling/note", contains=("hello world",))
    run("cd note", contains=("Not a directory",), cwd="/qa/one/sibling")
    run("cd note/..", contains=("Not a directory",), cwd="/qa/one/sibling")
    run("mkdir note/child", contains=("Not a directory",))
    run("ls note", contains=("note",))
    run("cd /qa", cwd="/qa")
    run("touch plain")
    run("touch .hidden")
    output = run("ls", contains=("one/", "plain"), absent=(".hidden",))
    memory = shell.memory()
    assert any(memory[i + 1] == 9 for i in range(0, len(memory) - 1, 2) if memory[i] == ord('/')), "directories must use LIGHT_BLUE"
    run("ls -a", contains=("./", "../", ".hidden", "one/", "plain"))
    run("ls -a /qa", contains=(".hidden", "one/", "plain"))
    run("ls one", contains=("two/", "sibling/"))
    run("ls /qa/one/../", contains=("one/", "plain"))
    run("ls missing", contains=("No such file or directory",))
    run("mkdir /missing/child", contains=("No such file or directory",))
    run("mkdir -p /qa/new/deep", cwd="/qa")
    run("mkdir -p /qa/new/deep", absent=("File exists",))
    run("mkdir " + "x" * 28, contains=("too long",))
    run('mkdir ""', contains=("Invalid",))
    run("mkdir", contains=("usage",))
    run("cd /", cwd="/")
    run("cd ..", cwd="/")
    run("cd ~", cwd="/home/JackHu")
    run("mkdir docs")
    run("cd ~/docs", cwd="/home/JackHu/docs")
    run("cd", cwd="/home/JackHu")
    run("cd ~other", contains=("Invalid path",), cwd="/home/JackHu")
    run('mkdir "two words"')
    run('cd "two words"', cwd="/home/JackHu/two words")
    run("cd ..", cwd="/home/JackHu")
    run("  pwd  ", contains=("/home/JackHu",), cwd="/home/JackHu")
    run("", absent=("Unknown command",))
    run("   \t ", absent=("Unknown command",))
    run("pwx\bd", contains=("/home/JackHu",))
    run("\t\bpwd", contains=("/home/JackHu",))
    prompt = "JackHu@JackHu-PC:/home/JackHu$ "
    run(" " * (79 - len(prompt)) + "pwd" + "\b" * 3 + "pwd",
        contains=("/home/JackHu",), cwd="/home/JackHu")
    run("pwd extra", contains=("usage",))
    run("cd a b", contains=("usage",), cwd="/home/JackHu")
    run("clear extra", contains=("usage",))
    run("ls -z", contains=("invalid option",))
    run("ls a b", contains=("usage",))
    run("mkdir -z", contains=("invalid option",))
    run("mkdir -p", contains=("usage",))
    run("mkdir alpha beta")
    run("ls", contains=("alpha/", "beta/"))
    run("pwd " + "x " * 16, contains=("too many arguments",))
    run("mkdir many")
    run("cd many", cwd="/home/JackHu/many")
    for i in range(20):
        run(f"mkdir d{i}")
    run("touch end")
    longest_name = "n" * 27
    run("mkdir " + longest_name)
    run("cd " + longest_name, cwd="/home/JackHu/many/" + longest_name)
    run("cd ..", cwd="/home/JackHu/many")
    run("ls", contains=("d0/", "d15/", "d16/", "d19/", "end"))
    run("pwd", contains=("/home/JackHu/many",))
    # Exceed path and line bounds without executing a truncated command.
    run("cd /" + "a/" * 130, contains=("too long",), cwd="/home/JackHu/many")
    run("z" * 520, contains=("input too long",), absent=("Unknown command",))
    # Scroll the display, then verify clear resets all memory and the next prompt.
    for _ in range(15):
        run("pwd", contains=("/home/JackHu/many",), clear=False)
    run("clear", cwd="/home/JackHu/many", clear=False)
    raw = shell.screen()
    expected = "JackHu@JackHu-PC:/home/JackHu/many$ "
    assert raw.startswith(expected), "clear must move the prompt to the top left"
    assert not raw[len(expected):].strip(" \x00"), "clear must remove all prior VGA contents"
    run("pwd", contains=("/home/JackHu/many",), clear=False)
    print(f"All {shell.cases} shell scenarios passed.")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--repo", type=Path, default=Path(__file__).resolve().parents[1])
    parser.add_argument("--probe", action="store_true", help="only boot and save the VGA text")
    args = parser.parse_args()
    with QemuShell(args.repo) as shell:
        if args.probe:
            shell.save_screen("boot")
            print(shell.screen().rstrip(" \x00"))
        else:
            run_suite(shell)


if __name__ == "__main__":
    main()
