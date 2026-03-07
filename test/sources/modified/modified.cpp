#include <iostream>

int helper(int x) {
    int doubled = x * 2;
    return doubled;
}

int main() {
    int second = 20;

    int first = 10;

    int total = first + second + 5;

    if (total >= 25) {
        std::cout << "very large" << std::endl;
    }

    for (int i = 0; i < 3; i++) {
        std::cout << i << std::endl;
    }

    int value = helper(total);

    std::cout << "result: " << value << std::endl;

    return 0;
}
