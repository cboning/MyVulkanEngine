#include "Render.h"
#include <iostream>
#include <vector>

class Application
{
private:
    Render _renderer;
    void init();
    void mainLoop();

public:
    Application();
    void run();
};