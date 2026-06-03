#include <iostream>

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    int n;
    std::cin >> n;
    std::cout << n << '\n';

    int x1, x2, y1, y2;
    for (int i = 0; i < n; ++i) {
        std::cin >> x1 >> x2 >> y1 >> y2;
        std::cout << x1 << ' ' << x2 << ' ' << y1 << ' ' << y2 << '\n';
    }
}
