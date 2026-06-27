# 🏰 FTDD — RPG Terminal Adventure

> Sebuah game RPG terminal bertema misteri pembunuhan di kota Nirva.  
> Dibangun dengan C++17 dan ncurses.

## 📖 Cerita

Kamu adalah seorang **pahlawan yang dijebak** atas pembunuhan Walikota Nirva.  
Lord Inquisitor Vane memberikan waktu **14 hari** untuk membersihkan namamu.  

Jelajahi kota, kumpulkan bukti, hadapi monster di dungeon, dan bongkar konspirasi  
antara High Priest Kael dan Guildmaster Valerius sebelum waktu habis.

## 🎮 Fitur

- **Exploration** — Jelajahi 19 lokasi di kota Nirva dengan sistem navigasi TAB + Map
- **Turn-based Battle** — Sistem pertarungan dengan party, magic, special moves, dan tactics
- **Dungeon Maze** — Dungeon procedural dengan algoritma Randomized Prim's
- **Quest System** — 10+ quest dengan branching dialogue dan data-driven conditions
- **Magic System** — 6 elemen (Fire, Water, Wind, Earth, Light, None) dengan 14+ spell
- **NPC Schedules** — NPC bergerak dinamis berdasarkan waktu (Pagi/Siang/Sore/Malam)
- **Shop System** — 5 toko dengan restock schedule harian
- **Fast Travel** — BFS pathfinding dengan random encounters di perjalanan
- **Interactive Inventory** — Kelola equipment, item, dan magic book
- **Data-Driven** — Semua konten (quest, dialog, item, monster) dari JSON

## 🚀 Cara Build & Run

### Pakai Nix (Recommended)

```bash
nix-shell
cmake -B build -S .
cmake --build build
./build/ftdd
```

### Manual (Linux)

```bash
# Dependencies
sudo apt install cmake g++ libncursesw5-dev libpulse-dev libasound2-dev nlohmann-json3-dev

cmake -B build -S .
cmake --build build
./build/ftdd
```

> Panduan lengkap untuk berbagai OS: [BUILD.md](./BUILD.md)

## 🎮 Controls

### Town State (Eksplorasi)
| Key | Aksi |
|---|---|
| `W` / `↑` | Navigasi menu ke atas |
| `S` / `↓` | Navigasi menu ke bawah |
| `Enter` | Pilih / Interaksi |
| `TAB` | Ganti tab (Orang / Aktivitas / Jalan Keluar / Peta) |
| `Q` | Keluar (dengan konfirmasi) |
| `C` | Lihat status karakter |
| `I` | Buka inventory |
| `L` | Buka log / histori |
| `E` | Lihat equipment |
| `U` | Use item |

### Battle State
| Key | Aksi |
|---|---|
| `W` / `↑` | Pilih menu ke atas |
| `S` / `↓` | Pilih menu ke bawah |
| `Enter` | Konfirmasi pilihan |

### Dungeon State
| Key | Aksi |
|---|---|
| `WASD` / `↑↓←→` | Gerak |
| `Enter` / `E` | Interaksi |
| `TAB` / `T` | Toggle map |
| `B` | Kembali |
| `1` / `2` | Gunakan skill |

## 🗺️ World Map

```
         [Biara Cahaya]
              |
    [Balai Kota] — [Alun-Alun] — [Pasar Kota]
              |
       [Jalanan Utara]
              |
  [Barat] — [Jalanan Kota] — [Timur]
              |
       [Jalanan Selatan]
              |
         [Kedai Usang]
              |
        [Kamar Loteng]
```

Juga: Permukiman Kumuh, Menara Tua, Gua Tambang, dan banyak lagi.

## ⚔️ Magic System

| Element | Tier 1 (DIRECT) | Tier 2 (REACH/AOE) |
|---|---|---|
| 🔥 Fire | Fire Bolt (20) | Fire Wall (18), Inferno Blast (40/AOE) |
| 💧 Water | Water Splash (20) | Water Wave (18) |
| 🌪️ Wind | Wind Blade (20) | Wind Gust (18) |
| 🪨 Earth | Earth Spike (20) | Earth Tremor (18) |
| ✨ Light | Light Strike (18) | Light Burst (32) |
| ❤️ Heal | Minor Heal (25) | Greater Heal (50) |
| ☄️ Special | — | Meteor Strike (80/AOE, 1/hr) |

## 📁 Struktur Proyek

```
src/
├── actions/      # Action dispatcher (data-driven commands)
├── db/           # JSON loading & data storage
├── managers/     # Quest, Dialog, Place, Player, Time managers
├── models/       # Entity, Item, Monster, NPC, Quest, etc.
├── states/       # Town, Dungeon, Battle, Shop, Inventory, GameOver
├── utils/        # UI components (Popup, ChoicePopup), data structures
└── views/        # ncurses UI layouts (MainPage, BattlePage)

data/
├── dialogs/      # Dialog scenes dengan branching choices
├── items/        # Item definitions (equipment, consumable, magic books)
├── monsters/     # Monster stats, loot, magic
├── npcs/         # NPC schedules, stats, magic
├── places/       # Location definitions, connections, activities
├── quests/       # Quest conditions, rewards
└── shops/        # Shop inventories & restock schedules
```

## 🧪 Testing

> Panduan lengkap: [WALKTHROUGH.md](./WALKTHROUGH.md)

## 🛠️ Tech Stack

- **C++17** dengan CMake
- **ncurses** (wide) untuk terminal UI
- **nlohmann_json** untuk parsing data
- **PulseAudio / ALSA** untuk audio
- **Nix** untuk reproducible builds
