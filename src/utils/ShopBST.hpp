// src/utils/ShopBST.hpp
#pragma once
#include <string>
#include <vector>

struct ShopNode {
    std::string item_id;
    int value; // Harga item untuk acuan sorting
    ShopNode* left;
    ShopNode* right;

    ShopNode(std::string id, int val) 
        : item_id(id), value(val), left(nullptr), right(nullptr) {}
};

class ShopBST {
private:
    ShopNode* root;

    ShopNode* insert(ShopNode* node, const std::string& id, int val) {
        if (node == nullptr) {
            return new ShopNode(id, val);
        }
        if (val < node->value) {
            node->left = insert(node->left, id, val);
        } else {
            node->right = insert(node->right, id, val);
        }
        return node;
    }

    void inOrder(ShopNode* node, std::vector<std::string>& sorted_ids) {
        if (node != nullptr) {
            inOrder(node->left, sorted_ids);
            sorted_ids.push_back(node->item_id);
            inOrder(node->right, sorted_ids);
        }
    }

    void clear(ShopNode* node) {
        if (node != nullptr) {
            clear(node->left);
            clear(node->right);
            delete node;
        }
    }

public:
    ShopBST() : root(nullptr) {}
    ~ShopBST() { clear(root); }

    void insertItem(const std::string& id, int val) {
        root = insert(root, id, val);
    }

    std::vector<std::string> getSortedItems() {
        std::vector<std::string> sorted_ids;
        inOrder(root, sorted_ids);
        return sorted_ids;
    }
};