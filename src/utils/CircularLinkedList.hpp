#pragma once

#include <stdexcept>
#include <vector>

namespace Utils {

template <typename T>
class CircularLinkedList {
private:
    struct Node {
        T data;
        Node* next;
        Node* prev;
        Node(const T& d) : data(d), next(nullptr), prev(nullptr) {}
    };
    
    Node* head;
    Node* tail;
    Node* current;
    size_t _size;

public:
    CircularLinkedList() : head(nullptr), tail(nullptr), current(nullptr), _size(0) {}
    
    ~CircularLinkedList() {
        clear();
    }

    void push_back(const T& value) {
        Node* new_node = new Node(value);
        if (!head) {
            head = tail = current = new_node;
            head->next = head;
            head->prev = head;
        } else {
            tail->next = new_node;
            new_node->prev = tail;
            new_node->next = head;
            head->prev = new_node;
            tail = new_node;
        }
        _size++;
    }

    void pop_front() {
        if (!head) return;
        if (head == tail) {
            delete head;
            head = tail = current = nullptr;
        } else {
            Node* temp = head;
            head = head->next;
            tail->next = head;
            head->prev = tail;
            if (current == temp) current = head;
            delete temp;
        }
        _size--;
    }

    void remove(const T& value) {
        if (!head) return;
        Node* temp = head;
        do {
            if (temp->data == value) {
                if (temp == head) {
                    pop_front();
                    return;
                }
                if (temp == tail) {
                    tail = tail->prev;
                    tail->next = head;
                    head->prev = tail;
                    if (current == temp) current = head;
                    delete temp;
                    _size--;
                    return;
                }
                temp->prev->next = temp->next;
                temp->next->prev = temp->prev;
                if (current == temp) current = temp->next;
                delete temp;
                _size--;
                return;
            }
            temp = temp->next;
        } while (temp != head);
    }

    void advance() {
        if (current) {
            current = current->next;
        }
    }

    T& front() {
        if (!head) throw std::out_of_range("List is empty");
        return current ? current->data : head->data;
    }
    
    const T& front() const {
        if (!head) throw std::out_of_range("List is empty");
        return current ? current->data : head->data;
    }

    bool empty() const {
        return _size == 0;
    }

    size_t size() const {
        return _size;
    }

    void clear() {
        while (_size > 0) {
            pop_front();
        }
    }

    std::vector<T> to_vector() const {
        std::vector<T> vec;
        if (!head) return vec;
        
        Node* start = current ? current : head;
        Node* iter = start;
        do {
            vec.push_back(iter->data);
            iter = iter->next;
        } while (iter != start);
        
        return vec;
    }
};

} // namespace Utils
