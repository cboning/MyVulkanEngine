#include "Engine/Render/Render.h"
#include <iostream>
#include <vector>

class Application
{
private:
    std::unique_ptr<Render> _renderer;
    void init();
    void mainLoop();

public:
    Application();
    void run();
};