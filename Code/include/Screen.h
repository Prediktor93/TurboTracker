#ifndef __SCREEN_H__
#define __SCREEN_H__

#include "ssd1306.h"

class Screen{
public:
    Screen();

    void Init();
    void Test();
    void TestText();
    void PrintImuGyro (float pitch, float roll);
    void PrintImuAccel(float acX, float acY);
    void PrintText(char *text, int pos);
    void ClearScreen();
    void Countdown();

private:

    SSD1306_t dev;
    int center, top, bottom;

};


#endif //__SCREEN_H__