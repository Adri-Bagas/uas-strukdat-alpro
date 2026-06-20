import pty
import os
import time

pid, fd = pty.fork()
if pid == 0:
    os.execvp("nix-shell", ["nix-shell", "--run", "./build/ftdd"])
else:
    time.sleep(1)
    os.write(fd, b'\033[B')
    time.sleep(0.5)
    os.write(fd, b'\033[B')
    time.sleep(0.5)
    os.write(fd, b'\n')
    time.sleep(1)
    # Give it some time to see if it crashes
    os.write(fd, b'q')
    time.sleep(1)
    
    # Read output
    try:
        while True:
            data = os.read(fd, 1024)
            if not data:
                break
    except Exception as e:
        pass
    
    _, status = os.waitpid(pid, 0)
    print("Exit status:", status)
    if os.WIFSIGNALED(status):
        print("Terminated by signal:", os.WTERMSIG(status))
