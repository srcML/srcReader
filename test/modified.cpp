#include <iostream>

int helper(int x) {
    int value = square(x + 1);
    return value;
}

int main() {
    int second = 2;
    int first = 1;
    int total = first + second + 10;

    if (total >= 3) {
        std::cout << "total=" << total << std::endl;
    }

    for (int i = 0; i < 2; i++) {
        std::cout << i << std::endl;
    }

    return 0;
}
