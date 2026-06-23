#pragma once

#include <stdexcept>

namespace Utils {

template <typename T>
class Stack {
private:
    struct Node {
        T data;
        Node* next;
        Node(const T& val) : data(val), next(nullptr) {}
    };

    Node* _top;
    int _size;

public:
    Stack() : _top(nullptr), _size(0) {}

    ~Stack() {
        clear();
    }

    // Push element onto stack
    void push(const T& val) {
        Node* new_node = new Node(val);
        new_node->next = _top;
        _top = new_node;
        _size++;
    }

    // Pop element from stack
    void pop() {
        if (is_empty()) return;
        Node* temp = _top;
        _top = _top->next;
        delete temp;
        _size--;
    }

    // Get top element
    T& top() {
        if (is_empty()) throw std::runtime_error("Stack is empty");
        return _top->data;
    }

    // Get top element (const version)
    const T& top() const {
        if (is_empty()) throw std::runtime_error("Stack is empty");
        return _top->data;
    }

    // Check if empty
    bool is_empty() const {
        return _top == nullptr;
    }

    // Get size
    int size() const {
        return _size;
    }

    // Clear all elements
    void clear() {
        while (!is_empty()) {
            pop();
        }
    }
};

} // namespace Utils
