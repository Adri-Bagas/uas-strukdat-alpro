#include <iostream>
#include <vector>
#include <memory>

int main() {
    std::vector<std::unique_ptr<int>> v;
    v.push_back(std::make_unique<int>(1));
    int* p1 = v.back().get();
    
    for (int i = 0; i < 100; ++i) {
        v.push_back(std::make_unique<int>(i));
    }
    
    std::cout << "p1 val: " << *p1 << std::endl;
    return 0;
}
