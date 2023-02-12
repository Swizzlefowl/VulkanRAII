#include "Renderer.h"
#include "PresentationEngine.h"
int main() {
    Renderer app{};
    PresentationEngine engine{app};
    try {
        app.run(&engine);
    } catch (const std::exception& except) {
        std::cout << except.what();
        return 1;
    }
    return 0;
}
