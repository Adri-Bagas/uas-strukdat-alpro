#pragma once

#include <string>
#include <vector>
#include <functional>

namespace Utils {

struct EncyclopediaEntry {
    std::string id;
    std::string name;
    std::string category;
    std::string description;
    bool is_discovered;

    EncyclopediaEntry(std::string id, std::string name, std::string category, 
                      std::string description, bool is_discovered = false)
        : id(std::move(id)), name(std::move(name)), category(std::move(category)), 
          description(std::move(description)), is_discovered(is_discovered) {}
};

class EncyclopediaBST {
public:
    struct Node {
        EncyclopediaEntry entry;
        Node* left;
        Node* right;

        Node(EncyclopediaEntry entry)
            : entry(std::move(entry)), left(nullptr), right(nullptr) {}
    };

private:
    Node* root;

    Node* insert(Node* node, EncyclopediaEntry entry) {
        if (node == nullptr) {
            return new Node(std::move(entry));
        }

        if (entry.name < node->entry.name) {
            node->left = insert(node->left, std::move(entry));
        } else if (entry.name > node->entry.name) {
            node->right = insert(node->right, std::move(entry));
        } else {
            if (entry.id < node->entry.id) {
                node->left = insert(node->left, std::move(entry));
            } else {
                node->right = insert(node->right, std::move(entry));
            }
        }
        return node;
    }

    Node* find(Node* node, const std::string& id) const {
        if (node == nullptr) return nullptr;
        if (node->entry.id == id) return node;

        Node* left_res = find(node->left, id);
        if (left_res != nullptr) return left_res;

        return find(node->right, id);
    }

    void in_order(Node* node, std::vector<const EncyclopediaEntry*>& entries) const {
        if (node != nullptr) {
            in_order(node->left, entries);
            entries.push_back(&node->entry);
            in_order(node->right, entries);
        }
    }

    void clear(Node* node) {
        if (node != nullptr) {
            clear(node->left);
            clear(node->right);
            delete node;
        }
    }

public:
    EncyclopediaBST() : root(nullptr) {}
    
    ~EncyclopediaBST() {
        clear(root);
    }

    EncyclopediaBST(const EncyclopediaBST&) = delete;
    EncyclopediaBST& operator=(const EncyclopediaBST&) = delete;

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

    void insert_entry(std::string id, std::string name, std::string category, 
                      std::string description, bool is_discovered = false) {
        root = insert(root, EncyclopediaEntry(std::move(id), std::move(name), 
                                              std::move(category), std::move(description), 
                                              is_discovered));
    }

    bool discover_entry(const std::string& id) {
        Node* node = find(root, id);
        if (node != nullptr) {
            node->entry.is_discovered = true;
            return true;
        }
        return false;
    }

    bool is_discovered(const std::string& id) const {
        Node* node = find(root, id);
        return node ? node->entry.is_discovered : false;
    }

    void set_discovered(const std::string& id, bool val) {
        Node* node = find(root, id);
        if (node) node->entry.is_discovered = val;
    }

    void set_discovered(const std::vector<std::string>& ids) {
        for (const auto& id : ids) {
            set_discovered(id, true);
        }
    }

    std::vector<std::string> get_discovered_ids() const {
        std::vector<const EncyclopediaEntry*> all = get_all_entries();
        std::vector<std::string> ids;
        for (const auto* e : all) {
            if (e->is_discovered) ids.push_back(e->id);
        }
        return ids;
    }

    std::vector<const EncyclopediaEntry*> get_all_entries() const {
        std::vector<const EncyclopediaEntry*> entries;
        in_order(root, entries);
        return entries;
    }

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
