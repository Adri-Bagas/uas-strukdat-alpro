# Panduan Menyelesaikan Game — Ending Gereja (Good Ending)

## Ringkasan

Kamu adalah seorang **pahlawan yang dijebak** atas pembunuhan Walikota Nirva. Lord Inquisitor Vane memberikan waktu **14 hari** untuk membersihkan namamu. Gagal = **eksekusi**. Berhasil = **bebas dan mendapatkan pengakuan**.

---

## Syarat Ending Gereja (Good Ending)

Pada Hari ke-14, game mengecek **dua variabel**:

| Variabel | Nilai yang Dibutuhkan | Cara Mendapatkan |
|---|---|---|
| `silas_intel_secured` | `1` | Selesaikan quest **`quest_gereja_silas_1`** (Informasi Berharga) dari Silas |
| `quest_gereja_4` | COMPLETED | Selesaikan quest **`quest_gereja_4`** (Relik Suci) dari Pastor Andreas |

Jika kedua terpenuhi → **Good Ending** (dibebaskan, Kael & Valerius ditangkap).
Jika salah satu/both tidak terpenuhi → **Bad Ending** (eksekusi di alun-alun).

---

## Walkthrough Lengkap (Day-by-Day)

### Hari ke-1 (Otomatis — Tidak Bisa Dilewati)

Kamu akan menjalani serangkaian cutscene:
1. Terjaga di **Kandang Kuda**
2. Ditangkap oleh **Gagak Hitam** (Valerius)
3. Dibawa ke **Balai Kota** — Valerius memberi ultimatum 14 hari
4. **Kael** memasang *Holy Prison Bracelet* (membatasi kekuatanmu)
5. Dilepaskan ke **Alun-Alun**
6. Warga bergosip tentangmu
7. Kabur ke gang samping (sore hari)
8. Menemukan kedai **"The Blind Boar"**
9. Masuk ke **Kedai Usang**
10. Bertemu **Arthur** — dia menawarkan bantuan
11. Pindah ke **kamar loteng** (safehouse) — hari berakhir

> **Catatan:** Belum ada misi yang bisa diambil di hari 1. Nikmati saja ceritanya.

---

### Hari ke-2 (Pagi — Cutscene Otomatis)

Saat keluar dari **kamar loteng** di pagi hari:
1. Cutscene Arthur dan Valerius bertengkar
2. Kamu bisa memilih: **Sembunyi** atau **Tampakkan Diri** (tidak memengaruhi akhir)
3. Quest **`quest_survei_kota`** (Investigasi Wilayah Nirva) diterima
4. Arthur memberimu perlengkapan awal

#### Segera Lakukan:
- Buka **TAB → Peta** untuk melihat peta kota
- Kunjungi **Pasar Kota** (timur dari Alun-Alun)
- Kunjungi **Biara** (utara dari Alun-Alun) — tempat semua quest penting berada

---

### Hari ke-2 sampai 13 — Rantai Quest Biara

Semua quest gereja ada di **Biara Cahaya** (utara Alun-Alun). NPC-nya:

| NPC | Lokasi di Biara | Quest |
|---|---|---|
| **Maria** | Dalam Biara | `gereja_1` — Bantuan Dapur Umum |
| **Kael** | Dalam Biara | `gereja_kael_1` — Tugas Suci (opsional) |
| **Elara** | Dalam Biara | `gereja_elara_1` — Kecurigaan Elara (opsional) |
| **Andreas** | Dalam Biara | `gereja_2`, `gereja_3`, `gereja_4` |
| **Silas** | Permukiman Kumuh | `gereja_silas_1` — Informasi Berharga (KRITIS) |

---

### Urutan Pengerjaan yang Disarankan

#### Langkah 1: Quest Maria — "Bantuan untuk Dapur Umum"
- **NPC:** Maria (Biara)
- **Tujuan:** Dapatkan 3 roti
- **Cara:** Pergi ke **Pasar Kota** → cari activity trigger `scene_pasar_gandum` → lawan 2 bandit → dapat 3 roti
- **Kembali ke Maria** → quest selesai
- **Hadiah:** 10 gold + membuka semua quest lain
- **PENTING:** Tanpa quest ini, quest lain (Silas, Andreas, Kael, Elara) **tidak akan muncul**

#### Langkah 2: Quest Silas — "Informasi Berharga" (KRITIS)
- **NPC:** Silas di **Permukiman Kumuh** (timur dari Alun-Alun, lalu ke selatan/barat)
- **Tujuan:** Dapatkan **Bukti Gagak**
- **Cara:** Setelah terima quest, pergi ke **Jalanan Barat Kota** → trigger `scene_silas_target` → lawan `boss_bfb_01` → dapat `bukti_gagak`
- **Kembali ke Silas** → quest selesai
- **PENTING:** Ini yang membuat `silas_intel_secured = 1`. **WAJIB** untuk good ending.

#### Langkah 3: Quest Andreas 2 — "Anak yang Hilang"
- **NPC:** Andreas (Biara)
- **Tujuan:** Temukan Lukas di **Dungeon Lantai 2**
- **Cara:** Masuk dungeon dari **pasar_kota** atau **alun_alun** → cari trigger `scene_dungeon_lukas` di lantai 2 → Lukas bergabung ke party
- **Kembali ke Andreas** → quest selesai
- **Hadiah:** 2 Potion HP

#### Langkah 4: Quest Andreas 3 — "Pertahankan Biara"
- **NPC:** Andreas
- **Tujuan:** Bertahan dari serangan orc di **Alun-Alun**
- **Cara:** Pergi ke **Alun-Alun** → trigger `scene_defend_biara` → lawan 2 goblin + orc boss
- **Kembali ke Andreas** → quest selesai
- **Hadiah:** Iron Sword

#### Langkah 5: Quest Andreas 4 — "Relik Suci" (KRITIS)
- **NPC:** Andreas
- **Tujuan:** Ambil **Relik Suci** dari **Gua Tambang**
- **Cara:** Pergi ke **Gua Tambang** (selatan Permukiman Kumuh) → trigger `scene_tambang_relik` → lawan Stone Golem → dapat `relik_suci`
- **Kembali ke Andreas** → quest selesai
- **PENTING:** Ini membuat `quest_gereja_4_complete = 1`. **WAJIB** untuk good ending.

#### Langkah 6: Quest Survei Kota
- **Dari Arthur** (di kamar loteng atau kedai)
- **Tujuan:** Kunjungi 4+ lokasi berbeda
- **Cara:** Cukup jelajahi peta — setiap lokasi baru yang kamu masuki pertama kali menambah counter
- **Lokasi yang disarankan:** Biara, Balai Kota, Pasar Kota, Permukiman Kumuh, Menara Tua, Gua Tambang, Kedai Usang
- **Hadiah:** Leather Boots, Nirva Sword, Leather Armor

---

### Quest Opsional (Untuk Persiapan)

#### Quest Kael — "Tugas Suci" (Jebakan)
- **NPC:** Kael (Biara) — tersedia setelah quest Maria selesai
- **Tujuan:** Pergi ke **Menara Tua**
- **Awas:** Ini jebakan! Kamu akan dihadapkan pada `scene_garrick_ambush` → lawan 2 bandit + stone golem
- **Hadiah:** 3 Potion MP (berguna untuk persiapan)
- **Catatan:** Kael adalah antagonis. Quest ini tidak wajib, tapi memberimu perlawanan bagus untuk latihan.

#### Quest Elara — "Kecurigaan Elara"
- **NPC:** Elara (Biara) — tersedia setelah quest Maria selesai
- **Tujuan:** Cari dokumen Kael di Dungeon Lantai 1
- **Catatan:** Membantumu memahami alur cerita, tapi tidak wajib untuk ending.

#### Investigasi Kandang Kuda
- **Dari Arthur** (bisa diambil kapan saja)
- **Tujuan:** Cari petunjuk di Kandang Kuda
- **Hadiah:** 30 gold + Potion HP
- **Catatan:** Petunjuk awal untuk memahami konspirasi.

---

### Tips Bertahan Hidup

1. **Naikkan Level:** Lawan musuh di dungeon atau di jalanan untuk EXP
2. **Kumpulkan Gold:** Untuk membeli perlengkapan di toko
3. **Simpan HP & MP:** Jangan lupa istirahat di kamar loteng
4. **Gunakan TAB:** Untuk navigasi cepat lewat peta
5. **Lengkapi Party:** Elara dan Lukas bisa menjadi anggota party yang membantumu di battle
6. **Cek Quest:** Tekan `L` untuk melihat log aktivitas dan progress quest

---

### Hari ke-14 — Penentuan

Di pagi hari ke-14, game otomatis akan menjalankan ending:

**Jika syarat terpenuhi** (Silas + Relik):
> Kamu bebas! Inquisitor Vane mengakui ketidakbersalahanmu. Kael dan Valerius ditangkap. Gelang penjara dibuka.

**Jika syarat tidak terpenuhi:**
> Eksekusi di Alun-Alun. Konspirasi terkubur bersamamu.

---

## Ringkasan Checklist (6 Langkah Penting)

- [ ] Hari 1: Ikuti cutscene otomatis
- [ ] Hari 2: Ambil quest survei dari Arthur
- [ ] **Maria: Selesaikan quest roti (buka akses semua quest)**
- [ ] **Silas: Selesaikan quest Informasi Berharga (WAJIB untuk good ending)**
- [ ] **Andreas 2→3→4: Selesaikan rantai quest (Relik Suci WAJIB untuk good ending)**
- [ ] Hari 14: Terima hasilnya!

## Troubleshooting

| Masalah | Solusi |
|---|---|
| Quest Silas tidak muncul | Selesaikan quest Maria dulu |
| Quest Andreas 2 tidak muncul | Selesaikan quest Maria |
| Tidak bisa masuk gua tambang | Buka peta (TAB), cari jalur dari Permukiman Kumuh |
| Kalah di battle | Grinding di dungeon, beli potion, lengkapi party |
| Waktu habis | Load save atau mulai baru — butuh sekitar 4-5 jam untuk selesaikan semua quest |
