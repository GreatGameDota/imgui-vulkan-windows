
#include "Application.h"

Application *app;

int main()
{
    app = new Application();
    app->Init();
    app->Run();
    delete app;
    return 0;
}