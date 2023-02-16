#include "Renderer.h"
#include "PresentationEngine.h"
#include "Graphics.h"
#include "Resources.h"

int main() {
    Renderer app{};
    PresentationEngine engine{app};
    Graphics graphics{app};
    Resources resources{app};
    try {
        app.run(&engine, &graphics, &resources);
    } catch (const std::exception& except) {
        std::cout << except.what();
        return 1;
    }
    return 0;
}
