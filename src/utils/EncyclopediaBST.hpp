// src/utils/EncyclopediaBST.hpp
#pragma once

#include <string>
#include <vector>
#include <functional>

namespace Utils {

// Struktur data untuk menyimpan informasi entitas dalam ensiklopedia
struct EncyclopediaEntry {
    std::string id;          // ID unik (misal: "npc_arthur", "mon_slime")
    std::string name;        // Nama tampilan entitas (misal: "Arthur", "Slime Lendir")
    std::string category;    // Kategori entitas ("npc" atau "monster")
    std::string description; // Deskripsi, statistik, atau lore entitas
    bool is_discovered;      // Apakah entitas ini sudah ditemui/dikalahkan?

    EncyclopediaEntry(std::string id, std::string name, std::string category, 
                      std::string description, bool is_discovered = false)
        : id(std::move(id)), name(std::move(name)), category(std::move(category)), 
          description(std::move(description)), is_discovered(is_discovered) {}
};

// Kelas utama Binary Search Tree (BST) Ensiklopedia
class EncyclopediaBST {
public:
    // Representasi Node dalam pohon biner
    struct Node {
        EncyclopediaEntry entry;
        Node* left;
        Node* right;

        Node(EncyclopediaEntry entry)
            : entry(std::move(entry)), left(nullptr), right(nullptr) {}
    };

private:
    Node* root;

    // Helper rekursif untuk memasukkan node baru
    // Pohon akan diurutkan secara alfabetis berdasarkan nama entitas (entry.name)
    Node* insert(Node* node, EncyclopediaEntry entry) {
        if (node == nullptr) {
            return new Node(std::move(entry));
        }

        if (entry.name < node->entry.name) {
            node->left = insert(node->left, std::move(entry));
        } else if (entry.name > node->entry.name) {
            node->right = insert(node->right, std::move(entry));
        } else {
            // Jika nama sama, bandingkan berdasarkan ID unik agar tidak ada duplikasi bertumpuk
            if (entry.id < node->entry.id) {
                node->left = insert(node->left, std::move(entry));
            } else {
                node->right = insert(node->right, std::move(entry));
            }
        }
        return node;
    }

    // Helper rekursif untuk mencari node berdasarkan ID unik (menggunakan penelusuran umum)
    Node* find(Node* node, const std::string& id) const {
        if (node == nullptr) return nullptr;
        if (node->entry.id == id) return node;

        // Mencari ke cabang kiri terlebih dahulu
        Node* left_res = find(node->left, id);
        if (left_res != nullptr) return left_res;

        // Jika tidak ditemukan di kiri, cari ke cabang kanan
        return find(node->right, id);
    }

    // Helper rekursif untuk In-Order Traversal (Kiri, Akar, Kanan)
    // Penelusuran ini otomatis mengembalikan entitas yang terurut secara alfabetis
    void in_order(Node* node, std::vector<const EncyclopediaEntry*>& entries) const {
        if (node != nullptr) {
            in_order(node->left, entries);
            entries.push_back(&node->entry);
            in_order(node->right, entries);
        }
    }

    // Helper rekursif untuk menghapus seluruh node (mencegah memory leak)
    void clear(Node* node) {
        if (node != nullptr) {
            clear(node->left);
            clear(node->right);
            delete node;
        }
    }

public:
    EncyclopediaBST() : root(nullptr) {}
    
    // Destruktor otomatis memanggil pembersihan memori
    ~EncyclopediaBST() {
        clear(root);
    }

    // Menonaktifkan operasi copy untuk mencegah isu double-delete memori dinamis
    EncyclopediaBST(const EncyclopediaBST&) = delete;
    EncyclopediaBST& operator=(const EncyclopediaBST&) = delete;

    // Mendukung operasi move semantics
    EncyclopediaBST(EncyclopediaBST&& other) noexcept : root(other.root) {
        other.root = nullptr;
    }
    
    EncyclopediaBST& operator=(EncyclopediaBST&& other) noexcept {
        if (this != &other) {
            clear(root);
            root = other.root;
            other.root = nullptr;
        }
        return *this;
    }

    // Memasukkan data entitas baru ke dalam BST
    void insert_entry(std::string id, std::string name, std::string category, 
                      std::string description, bool is_discovered = false) {
        root = insert(root, EncyclopediaEntry(std::move(id), std::move(name), 
                                              std::move(category), std::move(description), 
                                              is_discovered));
    }

    // Mengungkap/membuka kunci entitas (discovery) berdasarkan ID
    bool discover_entry(const std::string& id) {
        Node* node = find(root, id);
        if (node != nullptr) {
            node->entry.is_discovered = true;
            return true;
        }
        return false;
    }

    // Memeriksa status penemuan entitas
    bool is_discovered(const std::string& id) const {
        Node* node = find(root, id);
        return node ? node->entry.is_discovered : false;
    }

    // Mendapatkan semua entitas tersortir secara alfabetis
    std::vector<const EncyclopediaEntry*> get_all_entries() const {
        std::vector<const EncyclopediaEntry*> entries;
        in_order(root, entries);
        return entries;
    }

    // Mendapatkan entitas tersortir berdasarkan kategori khusus ("npc" atau "monster")
    std::vector<const EncyclopediaEntry*> get_entries_by_category(const std::string& category) const {
        std::vector<const EncyclopediaEntry*> all = get_all_entries();
        std::vector<const EncyclopediaEntry*> filtered;
        for (const auto* entry : all) {
            if (entry->category == category) {
                filtered.push_back(entry);
            }
        }
        return filtered;
    }
};

} // namespace Utils
