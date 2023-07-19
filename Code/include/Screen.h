#ifndef __SCREEN_H__
#define __SCREEN_H__

#include "ssd1306.h"

class Screen{
public:
    Screen();

    void Init();
    void Test();
    void PrintImuGyro(float pitch, float roll);

private:

    SSD1306_t dev;
    int center, top, bottom;

};


#endif //__SCREEN_H__