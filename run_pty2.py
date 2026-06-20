import pty
import os
import time

pid, fd = pty.fork()
if pid == 0:
    os.execvp("nix-shell", ["nix-shell", "--run", "gdb -q -ex 'run' -ex 'bt' -ex 'quit' ./build/ftdd"])
else:
    time.sleep(1)
    # Start gdb
    os.write(fd, b'r\n')
    time.sleep(1)
    os.write(fd, b'\033[B')
    time.sleep(0.5)
    os.write(fd, b'\033[B')
    time.sleep(0.5)
    os.write(fd, b'\n')
    time.sleep(2)
    os.write(fd, b'q')
    time.sleep(0.5)
    
    try:
        while True:
            data = os.read(fd, 4096)
            if not data:
                break
            print(data.decode('utf-8', errors='replace'), end='')
    except Exception as e:
        pass
    
    _, status = os.waitpid(pid, 0)
