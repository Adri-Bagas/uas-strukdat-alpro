#pragma once
#include <vector>
#include <functional>

namespace Utils {

// Fungsi helper untuk menggabungkan dua sub-array
template <typename T, typename Compare>
void merge(std::vector<T>& vec, int left, int mid, int right, Compare comp) {
    int n1 = mid - left + 1;
    int n2 = right - mid;

    // Membuat array sementara
    std::vector<T> L(n1);
    std::vector<T> R(n2);

    for (int i = 0; i < n1; i++) L[i] = vec[left + i];
    for (int j = 0; j < n2; j++) R[j] = vec[mid + 1 + j];

    // Menggabungkan kembali array sementara ke vec[left..right]
    int i = 0, j = 0, k = left;
    while (i < n1 && j < n2) {
        if (comp(L[i], R[j])) {
            vec[k] = L[i];
            i++;
        } else {
            vec[k] = R[j];
            j++;
        }
        k++;
    }

    // Menyalin sisa elemen L jika ada
    while (i < n1) {
        vec[k] = L[i];
        i++;
        k++;
    }

    // Menyalin sisa elemen R jika ada
    while (j < n2) {
        vec[k] = R[j];
        j++;
        k++;
    }
}

// Fungsi rekursif pembantu Merge Sort
template <typename T, typename Compare>
void merge_sort_helper(std::vector<T>& vec, int left, int right, Compare comp) {
    if (left >= right) return;
    
    int mid = left + (right - left) / 2;
    merge_sort_helper(vec, left, mid, comp);
    merge_sort_helper(vec, mid + 1, right, comp);
    merge(vec, left, mid, right, comp);
}

// Antarmuka utama fungsi Merge Sort kustom
template <typename T, typename Compare = std::less<T>>
void merge_sort(std::vector<T>& vec, Compare comp = Compare()) {
    if (vec.size() <= 1) return;
    merge_sort_helper(vec, 0, vec.size() - 1, comp);
}

} // namespace Utils
