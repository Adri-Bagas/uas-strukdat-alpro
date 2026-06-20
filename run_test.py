import subprocess
import time

p = subprocess.Popen(["nix-shell", "--run", "./build/ftdd"], stdin=subprocess.PIPE)
time.sleep(1)
p.stdin.write(b'\033[B\033[B\n')
p.stdin.flush()
time.sleep(1)
p.stdin.write(b'\n')
p.stdin.flush()
time.sleep(1)
p.stdin.write(b'q')
p.stdin.flush()
p.communicate()
