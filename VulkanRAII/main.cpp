#include "Renderer.h"
#include "PresentationEngine.h"
#include "Graphics.h"
int main() {
    Renderer app{};
    PresentationEngine engine{app};
    Graphics graphics{app};
    try {
        app.run(&engine, &graphics);
    } catch (const std::exception& except) {
        std::cout << except.what();
        return 1;
    }
    return 0;
}
