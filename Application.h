#include <iostream>
#include <vector>
#include "Render.h"

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