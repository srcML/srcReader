#include <iostream>

int helper(int x) {
    int result = x * 2;
    return result;
}

int main() {
    int first = 10;
    int second = 20;

    int total = first + second;

    if (total > 20) {
        std::cout << "large" << std::endl;
    }

    int value = helper(total);

    std::cout << value << std::endl;

    return 0;
}
