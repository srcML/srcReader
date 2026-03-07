#include <iostream>

int helper(int x) {
    int a = square(x);
    return a;
}

int main() {
    int first = 1;
    int second = 2;
    int total = first + second;

    if (total > 2) {
        std::cout << total << std::endl;
    }

    return 0;
}
