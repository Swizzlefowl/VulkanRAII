#include "Renderer.h"
#include "PresentationEngine.h"
#include "Graphics.h"
#include "Resources.h"

int main(int argc, char* argv[]) {
    std::vector<std::string> args{};
    if (argc < 2)
        ;
    else {
        for (size_t index{ 1 }; index < argc; index++) {
            std::string arg{argv[index]};
            args.emplace_back(arg);
        }
    }
    Renderer app{args};
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
