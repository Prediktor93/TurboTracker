#include "Screen.h"
#include "font8x8_basic.h"
#include "freertos/task.h"
#include <cstring>

static const char *TAG = "Screen";

#define I2CAddress 0x3C

Screen::Screen()
{
}

void Screen::Init()
{

    printf("Screen start\n");
    dev._address = I2CAddress;
	dev._flip = false;

	top = 2;
	center = 3;
	bottom = 8;

    ssd1306_init(&dev, 128, 64);
    ssd1306_contrast(&dev, 0xff);
    ssd1306_clear_screen(&dev, false);

}

void Screen::PrintImuGyro(float pitch, float roll)
{
	char lineChar[17] = "                ";
	char emptylineChar[17] = "                ";
	static int pos_last = 0;

	//NewValue = (((OldValue - OldMin) * (NewMax - NewMin)) / (OldMax - OldMin)) + NewMin
	
	//Acotar valores a la mitad
	if(pitch > 1.55/2)  pitch = 1.55/2;
	if(pitch < -1.55/2) pitch = -1.55/2;
	if(roll > 1.55/2)   roll = 1.55/2;
	if(roll < -1.55/2)  roll = -1.55/2;
	
	//Pitch
		// [-1.55, 1.55] - [0-16]
		// (((-pitch + 1.55) * (15 - 0)) / (1.55+1.55)) + 0
	//int pos_char = (( (-pitch + 1.55) * 15) / 3.1); //Sin acotar
	int pos_char = (( (-pitch + 1.55/2) * 16) / 1.55); //Acotado
	lineChar[pos_char] = 'o';
	
	//Roll
		// [-3, 3] - [1-7] (acotado a [-1.55, 1.55])
		// (((-roll + 3) * (7 - 0)) / (3+3)) + 0
	//int pos_line = (( (-roll + 3) * 7) / 6); //Sin acotar
	int pos_line = (( (-roll + 1.55/2) * 8) / 1.55); //Acotado


	if (pos_line < 0)  pos_line = 0;
	if (pos_line > 7)  pos_line = 7;
	if (pos_char < 0)  pos_char = 0;
	if (pos_char > 16) pos_char = 16;

	ssd1306_display_text(&dev, pos_line, lineChar, strlen(lineChar), false);
	//Borra la linea anterior
	if(pos_last != pos_line) ssd1306_display_text(&dev, pos_last, emptylineChar, strlen(emptylineChar), false);

	pos_last = pos_line;
}

void Screen::PrintImuAccel(float acX, float acY)
{
	char lineChar[17] = "                ";
	char emptylineChar[17] = "                ";
	static int pos_last = 0;

	//NewValue = (((OldValue - OldMin) * (NewMax - NewMin)) / (OldMax - OldMin)) + NewMin

	//X axis [-10, 10] - [0-16]
	int pos_char = (( (acX + 10) * 16) / 20);
	lineChar[pos_char] = 'o';
	
	//Y axis [-10, 10] - [0-16]
	int pos_line = (( (-acY + 10) * 8) / 20);


	if (pos_line < 0)  pos_line = 0;
	if (pos_line > 7)  pos_line = 7;
	if (pos_char < 0)  pos_char = 0;
	if (pos_char > 16) pos_char = 16;

	ssd1306_display_text(&dev, pos_line, lineChar, strlen(lineChar), false);
	//Borra la linea anterior
	if(pos_last != pos_line) ssd1306_display_text(&dev, pos_last, emptylineChar, strlen(emptylineChar), false);

	pos_last = pos_line;
}

void Screen::Test()
{
    
    char lineChar[20];

    //Line from 0 - 8
    ssd1306_display_text(&dev, 2, "0 0 1 0 0 0 0 0", 15, false);
    vTaskDelay(3000 / portTICK_PERIOD_MS);

    // Display Count Down
    ssd1306_clear_screen(&dev, false);
	uint8_t image[24];
	std::memset(image, 0, sizeof(image));
	ssd1306_display_image(&dev, top, (6*8-1), image, sizeof(image));
	ssd1306_display_image(&dev, top+1, (6*8-1), image, sizeof(image));
	ssd1306_display_image(&dev, top+2, (6*8-1), image, sizeof(image));
	for(int font=0x39;font>0x30;font--) {
		std::memset(image, 0, sizeof(image));
		ssd1306_display_image(&dev, top+1, (7*8-1), image, 8);
		std::memcpy(image, font8x8_basic_tr[font], 8);
		if (dev._flip) ssd1306_flip(image, 8);
		ssd1306_display_image(&dev, top+1, (7*8-1), image, 8);
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}

    // Scroll Up
	ssd1306_clear_screen(&dev, false);
	ssd1306_contrast(&dev, 0xff);
	ssd1306_display_text(&dev, 0, "---Scroll  UP---", 16, true);
	//ssd1306_software_scroll(&dev, 7, 1);
	ssd1306_software_scroll(&dev, (dev._pages - 1), 1);
	for (int line=0;line<bottom+10;line++) {
		lineChar[0] = 0x01;
		sprintf(&lineChar[1], " Line %02d", line);
		ssd1306_scroll_text(&dev, lineChar, strlen(lineChar), false);
		vTaskDelay(500 / portTICK_PERIOD_MS);
	}
	vTaskDelay(3000 / portTICK_PERIOD_MS);
	
	// Scroll Down
	ssd1306_clear_screen(&dev, false);
	ssd1306_contrast(&dev, 0xff);
	ssd1306_display_text(&dev, 0, "--Scroll  DOWN--", 16, true);
	//ssd1306_software_scroll(&dev, 1, 7);
	ssd1306_software_scroll(&dev, 1, (dev._pages - 1) );
	for (int line=0;line<bottom+10;line++) {
		lineChar[0] = 0x02;
		sprintf(&lineChar[1], " Line %02d", line);
		ssd1306_scroll_text(&dev, lineChar, strlen(lineChar), false);
		vTaskDelay(500 / portTICK_PERIOD_MS);
	}
	vTaskDelay(3000 / portTICK_PERIOD_MS);

	// Page Down
	ssd1306_clear_screen(&dev, false);
	ssd1306_contrast(&dev, 0xff);
	ssd1306_display_text(&dev, 0, "---Page	DOWN---", 16, true);
	ssd1306_software_scroll(&dev, 1, (dev._pages-1) );
	for (int line=0;line<bottom+10;line++) {
		//if ( (line % 7) == 0) ssd1306_scroll_clear(&dev);
		if ( (line % (dev._pages-1)) == 0) ssd1306_scroll_clear(&dev);
		lineChar[0] = 0x02;
		sprintf(&lineChar[1], " Line %02d", line);
		ssd1306_scroll_text(&dev, lineChar, strlen(lineChar), false);
		vTaskDelay(500 / portTICK_PERIOD_MS);
	}
	vTaskDelay(3000 / portTICK_PERIOD_MS);

	// Horizontal Scroll
	ssd1306_clear_screen(&dev, false);
	ssd1306_contrast(&dev, 0xff);
	ssd1306_display_text(&dev, center, "Horizontal", 10, false);
	ssd1306_hardware_scroll(&dev, SCROLL_RIGHT);
	vTaskDelay(5000 / portTICK_PERIOD_MS);
	ssd1306_hardware_scroll(&dev, SCROLL_LEFT);
	vTaskDelay(5000 / portTICK_PERIOD_MS);
	ssd1306_hardware_scroll(&dev, SCROLL_STOP);
	
	// Vertical Scroll
	ssd1306_clear_screen(&dev, false);
	ssd1306_contrast(&dev, 0xff);
	ssd1306_display_text(&dev, center, "Vertical", 8, false);
	ssd1306_hardware_scroll(&dev, SCROLL_DOWN);
	vTaskDelay(5000 / portTICK_PERIOD_MS);
	ssd1306_hardware_scroll(&dev, SCROLL_UP);
	vTaskDelay(5000 / portTICK_PERIOD_MS);
	ssd1306_hardware_scroll(&dev, SCROLL_STOP);
	
	// Invert
	ssd1306_clear_screen(&dev, true);
	ssd1306_contrast(&dev, 0xff);
	ssd1306_display_text(&dev, center, "  Good Bye!!", 12, true);
	vTaskDelay(5000 / portTICK_PERIOD_MS);


	// Fade Out
	ssd1306_fadeout(&dev);

}

void Screen::TestText()
{
	ssd1306_display_text(&dev, 0, "------TEST------", 16, true);
	ssd1306_display_text(&dev, 1, "------TEST------", 16, true);
	ssd1306_display_text(&dev, 2, "------TEST------", 16, true);
	ssd1306_display_text(&dev, 3, "------TEST------", 16, true);
	ssd1306_display_text(&dev, 4, "------TEST------", 16, true);
	ssd1306_display_text(&dev, 5, "------TEST------", 16, true);
	ssd1306_display_text(&dev, 6, "------TEST------", 16, true);
	ssd1306_display_text(&dev, 7, "------TEST------", 16, true);
}

//Pos[0-7] Text[0-16]
void Screen::PrintText(char *text, int pos)
{
	ssd1306_display_text(&dev, pos, text,  strlen(text), false);
}

void Screen::PrintTextColor(char *text, int pos, bool color)
{
	ssd1306_display_text(&dev, pos, text,  strlen(text), color);
}

void Screen::ClearScreen(){
	ssd1306_clear_screen(&dev, false);
}

void Screen::Countdown(){
	ssd1306_clear_screen(&dev, false);
	uint8_t image[24];
	std::memset(image, 0, sizeof(image));
	ssd1306_display_image(&dev, top, (6*8-1), image, sizeof(image));
	ssd1306_display_image(&dev, top+1, (6*8-1), image, sizeof(image));
	ssd1306_display_image(&dev, top+2, (6*8-1), image, sizeof(image));
	for(int font=0x35;font>0x30;font--) {
		std::memset(image, 0, sizeof(image));
		ssd1306_display_image(&dev, top+1, (7*8-1), image, 8);
		std::memcpy(image, font8x8_basic_tr[font], 8);
		if (dev._flip) ssd1306_flip(image, 8);
		ssd1306_display_image(&dev, top+1, (7*8-1), image, 8);
		vTaskDelay(950 / portTICK_PERIOD_MS);
	}
}