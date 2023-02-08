#include "Renderer.h"

int main() {
    Renderer app{};

    try {
        app.run();
    } catch (const std::exception& except) {
        std::cout << except.what();
        return 1;
    }
    return 0;
}
