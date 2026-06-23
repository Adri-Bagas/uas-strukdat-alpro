# **Panduan UAS Terintegrasi** 

# **Project: Algoritma Pemrograman & Struktur Data**

## **Deskripsi Tugas**

Tugas UTS ini adalah proyek terintegrasi yang akan menjadi komponen penilaian untuk **4 Mata Kuliah**:

1. Teori Algoritma dan Pemrograman  
2. Praktikum Algoritma dan Pemrograman  
3. Teori Struktur Data  
4. Praktikum Struktur Data

Anda diminta membuat program C++ dengan **topik bebas** (contoh: *Game Text-Based*, Sistem Rumah Sakit, Simulasi Antrean Bank, dsb.). Program tersebut harus memuat logika dasar C++ tingkat lanjut sekaligus mengimplementasikan berbagai jenis Struktur Data. Proyek ini merupakan fondasi yang akan dilanjutkan ke Proyek Akhir di akhir semester.

## **Bagian 1: Desain Algoritma & Konsep (Mewakili: Teori Alpro & Teori Struktur Data)**

Sebelum menulis kode, Anda wajib memodelkan logika dan arsitektur data program.

1. **Flowchart Program Utama (Teori Alpro)**  
   * Buatlah flowchart representasi visual alur program secara keseluruhan (Terminal, Input/Output, Proses, Percabangan).  
2. **Pseudocode (Teori Alpro)**  
   * Tuliskan logika dari flowchart ke dalam bentuk pseudocode yang terstruktur.  
3. **Ilustrasi Konsep Struktur Data (Teori Struktur Data)**  
   * Gambarkan secara visual (bisa menggunakan *node-diagram* sederhana) bagaimana data dalam program Anda disimpan dan berpindah menggunakan memori dinamis. Anda harus menjelaskan *mengapa* struktur data tertentu dipilih untuk fitur tertentu di dalam program Anda.

## **Bagian 2: Implementasi Pemrograman C++ (Mewakili: Praktikum Alpro)**

Program C++ Anda wajib mengimplementasikan 11 elemen teknis fundamental berikut:

1. **struct**: Mendefinisikan entitas objek (Sekaligus akan digunakan sebagai *Node* pada struktur data Anda).  
2. **References (&)**: Digunakan pada parameter fungsi untuk menghindari penyalinan data dan meningkatkan efisiensi.  
3. **Pointer (\*)**: Wajib digunakan untuk manipulasi alamat memori, alokasi dinamis, dan menghubungkan data antar simpul (*node*).  
4. **namespace**: Bungkus fungsi-fungsi logika utama Anda ke dalam sebuah *namespace* khusus.  
5. **Callback Function**: Minimal satu fungsi yang menerima fungsi lain sebagai parameter.  
6. **Default Argument / Inline Function**: Gunakan untuk memberikan nilai awal standar dan mengoptimalkan fungsi-fungsi pendek.  
7. **Function Overloading / Function Template**: Buat fungsi dengan nama sama namun beda parameter, atau fungsi dinamis untuk berbagai tipe data.  
8. **Exception Handling**: Digunakan untuk menangani error saat program berjalan, agar program tidak langsung berhenti secara tiba-tiba. Gunakan try, catch, dan throw.  
9. **Standard Template Library (STL) Vector or List, Iterator, Sort, Find, Count:** Gunakan fitur bawaan C++ seperti vector atau list untuk menyimpan data, serta algoritma seperti sort, find, dan count. Gunakan juga **iterator** untuk menelusuri elemen di dalam container.  
10. **File Handling:** Digunakan untuk membaca atau menulis data ke file, misalnya menyimpan hasil program ke file .txt menggunakan ofstream, atau membaca data dari file menggunakan ifstream.  
11. **Lambda Expression:** Digunakan untuk membuat fungsi singkat tanpa nama, biasanya dipakai langsung di dalam proses seperti sort, find\_if, atau callback sederhana.

## **Bagian 3: Implementasi Struktur Data (Mewakili: Praktikum Struktur Data)**

Di dalam program yang sama, Anda **wajib** mengimplementasikan 8 struktur data berikut agar program dapat berjalan:

1. **Linked List (Single atau Double)**  
   * *Saran Penggunaan:* Gunakan sebagai penyimpanan data utama/master data yang dinamis (misal: Daftar Barang, Daftar Pasien).  
2. **Circular Linked List**  
   * *Saran Penggunaan:* Gunakan untuk fitur yang sifatnya berputar/berulang tanpa ujung (misal: Sistem giliran pemain dalam *game*, atau rotasi jadwal/shift).  
3. **Stack (LIFO \- Array/Linked List)**  
   * *Saran Penggunaan:* Gunakan untuk fitur *History* atau *Undo* (membatalkan aksi terakhir), atau riwayat navigasi menu.  
4. **Queue (FIFO \- Biasa / Circular / Priority / Deque)**  
* *Saran Penggunaan:* Gunakan untuk sistem antrian (misal: Antrian pemesanan, *waiting list* pasien, antrean *rendering*). Jika ingin nilai lebih tinggi, implementasikan *Priority Queue* (antrian berdasarkan prioritas/VIP).  
    
5. **Binary Tree / AVL Tree**  
* Digunakan untuk menyimpan data dalam bentuk hierarki/pohon.  
  * Binary Tree: setiap node memiliki maksimal dua child (left dan right).  
  * AVL Tree: versi Binary Tree yang otomatis menjaga keseimbangan tinggi pohon agar proses pencarian, penambahan, dan penghapusan data tetap efisien.  
6. **Graph dengan BFS dan DFS**  
   * Gunakan untuk merepresentasikan hubungan antar node/simpul, misalnya jaringan jalan, pertemanan, atau rute.  
7. **Hashing dan Hash Table:**   
   * Digunakan untuk menyimpan dan mencari data dengan sangat cepat menggunakan key. Hashing mengubah key menjadi index penyimpanan melalui hash function. Contoh implementasi di C++ adalah unordered\_map.  
8. **Sorting:**   
   * Digunakan untuk mengurutkan data secara ascending atau descending agar lebih mudah dicari dan diproses. Bisa menggunakan algoritma seperti Bubble Sort, Selection Sort, Merge Sort, atau fungsi STL seperti sort().

## **Bagian 4: Format Pengumpulan Laporan**

Dikumpulkan dalam bentuk **Laporan berformat PDF** dengan struktur berikut:

* **Bab 1: Pendahuluan** Deskripsi program, topik yang dipilih, dan daftar fitur utama.  
* **Bab 2: Desain Algoritma (Teori Alpro & Strukdat)**  
  * Gambar Flowchart Program.  
  * Teks Pseudocode.  
  * Ilustrasi visual *node* dan penjelasan konseptual mengenai cara kerja Linked list, Stack, dan Queue pada program Anda.  
* **Bab 3: Implementasi C++ & Struktur Data (Praktikum Alpro & Strukdat)**  
  * *Snippet* (potongan) kode bukti implementasi 11 syarat Praktikum Alpro.  
  * *Snippet* kode bukti implementasi 8 syarat Struktur Data (Operasi dasar seperti *Insert, Delete, Display, Push, Pop, Enqueue, Dequeue* harus ada).  
* **Bab 4: Hasil Pengujian (*Screenshots*)** Tangkapan layar terminal saat program dijalankan dan didemonstrasikan.  
* **Lampiran** Tautan (link) repositori (GitHub/Google Drive) yang berisi *source code* (file .cpp) lengkap.

## **Kriteria Penilaian Terintegrasi**

Karena ini adalah proyek untuk 4 mata kuliah, nilai akan diekstrak berdasarkan komponen berikut ke masing-masing mata kuliah (Setiap mata kuliah mendapat nilai maksimal 100):

**1\. Nilai Teori Algoritma & Pemrograman**

* **Kelengkapan (50%):** Flowchart dan pseudocode merepresentasikan alur program dari awal hingga akhir.  
* **Logika (50%):** Percabangan (IF/SWITCH) dan perulangan (FOR/WHILE) dalam desain logis dan bebas dari *deadlock*.

**2\. Nilai Teori Struktur Data**

* **Ketepatan Konsep (60%):** Penjelasan di dalam laporan mengenai cara kerja Stack, Queue, dan Linked List sangat jelas dan sesuai dengan teori.  
* **Justifikasi Pemilihan (40%):** Alasan penggunaan struktur data tertentu untuk fitur tertentu masuk akal (contoh: Mengapa menggunakan Stack untuk *Undo*, bukan Queue).

**3\. Nilai Praktikum Algoritma & Pemrograman**

* **Implementasi Syarat (70%):** Ke-7 syarat (Struct, Pointer, Reference, Namespace, Callback, Inline/Default, Overloading/Template) terimplementasi dengan sintaks yang benar.  
* **Kerapihan Kode (30%):** Kode dapat dikompilasi tanpa *error*, memiliki indentasi yang baik, dan terdapat komentar (*documentation*).

**4\. Nilai Praktikum Struktur Data**

* **Keberhasilan Implementasi (80%):** Kode berjalan dengan baik, dan tidak menyebabkan *Memory Leak*.  
* **Integrasi Program (20%):** kedelapan struktur data tersebut tidak berdiri sendiri secara terpisah, melainkan terhubung menjadi satu kesatuan program yang utuh.

