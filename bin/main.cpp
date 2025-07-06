#include <iostream>

#include "deque.h"

int main() {
    deque<int> d;

    d.push_back(1);
    d.push_front(0);
    d.emplace_back(2);
    d.insert(d.end(), {3, 4});
    d.insert(d.begin(), {-4, -3, -2, -1});

    std::cout << "Front: " << d.front() << "\n";
    std::cout << "Back: " << d.back() << "\n";
    std::cout << "Element at 1: " << d[1] << "\n";

    std::cout << "Data : ";
    for (auto it = d.begin(); it != d.end(); ++it) {
        std::cout << *it << "\t";
    }
    std::cout << "\n";

    d.pop_back();
    d.erase(d.begin());

    std::cout << "Data : ";
    for (auto el : d) {
        std::cout << el << "\t";
    }
    std::cout << "\n";

    return 0;
}