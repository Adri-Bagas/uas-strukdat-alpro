#pragma once

#include <stdexcept>

namespace Utils {

template <typename T>
class Queue {
private:
    struct Node {
        T data;
        Node* next;
        Node(T val) : data(val), next(nullptr) {}
    };

    Node* head;
    Node* tail;
    int _size;

public:
    Queue() : head(nullptr), tail(nullptr), _size(0) {}

    ~Queue() {
        clear();
    }

    void enqueue(T val) {
        Node* new_node = new Node(val);
        if (tail) {
            tail->next = new_node;
        } else {
            head = new_node;
        }
        tail = new_node;
        _size++;
    }

    void dequeue() {
        if (!head) return;
        Node* temp = head;
        head = head->next;
        if (!head) {
            tail = nullptr;
        }
        delete temp;
        _size--;
    }

    T front() const {
        if (!head) throw std::runtime_error("Queue is empty");
        return head->data;
    }

    bool is_empty() const {
        return head == nullptr;
    }

    int size() const {
        return _size;
    }

    void clear() {
        while (!is_empty()) {
            dequeue();
        }
    }
};

} // namespace Utils
