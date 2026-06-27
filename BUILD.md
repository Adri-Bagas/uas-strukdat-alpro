# Panduan Build & Instalasi

## Tentang Crash yang Kamu Alami

Stack trace yang muncul (`clock_gettime` → corrupted stack `0xcaf00000...`) adalah **stack corruption** yang biasanya disebabkan oleh:

1. **Terminal terlalu kecil** saat masuk dungeon (minimal 110x24) — ncurses menulis di luar batas window
2. **Resize terminal** secara cepat saat render — buffer ncurses corrupt
3. **Audio driver crash** — PulseAudio latency tidak sesuai (WSL/container)

> Perbaiki dengan: resize terminal minimal 110x24, jangan resize saat render aktif, dan pastikan PulseAudio terkonfigurasi benar.

---

## Metode 1: Nix (Reproducible — Direkomendasikan)

Nix menjamin environment development yang **identik di semua OS** (Linux, macOS, WSL).

### Instalasi Nix

```bash
# Linux / WSL / macOS
curl -L https://nixos.org/nix/install | sh

# Source profile setelah instalasi
. ~/.nix-profile/etc/profile.d/nix.sh
```

Untuk **NixOS**, Nix sudah terinstal secara bawaan.

### Build dengan Nix

```bash
# Clone repositori
git clone <repo-url>
cd uas-strukdat-alpro

# Masuk ke environment development
nix-shell

# Build
cmake -B build -S .
cmake --build build

# Jalankan
./build/ftdd

# Keluar dari nix-shell
exit
```

### Build Ulang Cepat (Saat Development)

```bash
nix-shell --run "cmake --build build"
```

Atau gunakan auto-rebuild (terinstal di nix-shell):
```bash
nix-shell --run "watchexec -w src -w data -- cmake --build build"
```

---

## Metode 2: Linux (Manual tanpa Nix)

### Ubuntu / Debian

```bash
# Dependencies
sudo apt-get update
sudo apt-get install -y \
    cmake \
    g++ \
    pkg-config \
    libncursesw5-dev \
    libncurses-dev \
    libpulse-dev \
    libasound2-dev \
    nlohmann-json3-dev

# Build
cmake -B build -S .
cmake --build build

# Jalankan
./build/ftdd
```

### Fedora

```bash
sudo dnf install -y \
    cmake \
    gcc-c++ \
    pkg-config \
    ncurses-devel \
    pulseaudio-libs-devel \
    alsa-lib-devel \
    nlohmann-json-devel

cmake -B build -S .
cmake --build build
```

### Arch Linux

```bash
sudo pacman -S \
    cmake \
    gcc \
    pkg-config \
    ncurses \
    pulseaudio \
    alsa-lib \
    nlohmann-json

cmake -B build -S .
cmake --build build
```

---

## Metode 3: macOS

### Dengan Nix (Direkomendasikan)

```bash
curl -L https://nixos.org/nix/install | sh
. ~/.nix-profile/etc/profile.d/nix.sh

cd uas-strukdat-alpro
nix-shell
cmake -B build -S .
cmake --build build
```

### Dengan Homebrew (Tanpa Nix)

```bash
brew install cmake pkg-config ncurses nlohmann-json pulseaudio

# Homebrew's ncurses adalah kegunaan (kegunaan) — path perlu manual
export CURSES_INCLUDE_DIRS=$(brew --prefix ncurses)/include
export CURSES_LIBRARIES=$(brew --prefix ncurses)/lib

cmake -B build -S . \
    -DCURSES_INCLUDE_DIRS=$CURSES_INCLUDE_DIRS \
    -DCURSES_LIBRARIES=$CURSES_LIBRARIES
cmake --build build
```

---

## Metode 4: Windows (WSL2)

**Wajib:** Windows 10/11 dengan WSL2 dan Ubuntu 22.04+.

```powershell
# Di PowerShell (Admin)
wsl --install -d Ubuntu
```

```bash
# Di WSL Ubuntu
sudo apt-get update
sudo apt-get install -y \
    cmake \
    g++ \
    pkg-config \
    libncursesw5-dev \
    libpulse-dev \
    libasound2-dev \
    nlohmann-json3-dev \
    curl

# Install Nix (opsional tapi direkomendasikan)
curl -L https://nixos.org/nix/install | sh
. ~/.nix-profile/etc/profile.d/nix.sh

cd uas-strukdat-alpro
nix-shell
cmake -B build -S .
cmake --build build
```

### Fix Audio di WSL (Suara Putus-Putus / Crash)

Buat `/etc/pulse/default.pa` atau jalankan sebelum game:

```bash
export PULSE_LATENCY_MSEC=200
./build/ftdd
```

Atau matikan audio total (jika tetap crash):
- Hapus `engine->get_music_manager()` calls dari `on_enter()` states, atau
- Comment baris `#include "../managers/MusicManager.hpp"` dan panggilannya

---

## Troubleshooting

### "Ncurses library is missing!"
```bash
# Ubuntu/Debian
sudo apt-get install libncursesw5-dev libncurses-dev
# Fedora
sudo dnf install ncurses-devel
# Arch
sudo pacman -S ncurses
```

### "Ncurses menu library not found!"
```bash
# Pastikan package ncurses development sudah lengkap (bukan hanya libncurses5-dev)
# Ubuntu/Debian: sudo apt-get install libncursesw5-dev sudah include menu/panel
```

### Crash: "corrupt stack?" / Stack corruption
- Terminal minimal **110×24** untuk dungeon
- Jangan resize terminal saat animasi/render aktif
- Jika di WSL, matikan audio: `export PULSE_LATENCY_MSEC=200`
- Coba terminal berbeda (GNOME Terminal, Kitty, Alacritty — hindari Windows Terminal versi lama)

### "git: command not found" di nix-shell
```bash
nix-shell -p git
```

### Permission denied saat clone di nix-shell
```bash
git config --global --add safe.directory /path/to/uas-strukdat-alpro
```

### VLC / libvlc not found (error tapi build tetap jalan)
Ini aman diabaikan — library VLC hanya dipakai kalau ada, audio fallback ke PulseAudio/ALSA.
