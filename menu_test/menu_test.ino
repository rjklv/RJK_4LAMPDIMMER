/*
 * serial_nav.ino - Example code using the menu system library
 *
 * This example shows the menu system being controlled over the serial port.
 *
 * Copyright (c) 2015, 2016 arduino-menusystem
 * Licensed under the MIT license (see LICENSE)
 */

#include <MenuSystem.h>
#include "CustomNumericMenuItem.h"
#include "MyRenderer.h"

// forward declarations

//void on_root_selected(MenuItem* p_menu_item);
//void on_main_selected(MenuItem* p_menu_item);

void on_menu_address_selected(MenuItem* p_menu_item);

//const String format_float(const float value);
const String format_int(const float value);
//const String format_color(const float value);

//void on_item2_selected(MenuItem* p_menu_item);
//void on_item3_selected(MenuItem* p_menu_item);
void on_back_item_selected(MenuItem* p_menu_item);
void on_menu_mode_dmx_selected(MenuItem* p_menu_item);
void on_menu_mode_standalone_selected(MenuItem* p_menu_item);

void on_menu_exit(MenuItem* p_menu_item);
// Menu variables

MyRenderer my_renderer;
MenuSystem ms(my_renderer);

Menu menu_root("ROOT (_006)");
  Menu menu_main("Setup");
    Menu menu_address("Address Setup");
       //NumericMenuItem menu_item_address("addr: )", &on_menu_address_selected);
       NumericMenuItem menu_item_address("Addr value:", nullptr, 6, 1, 512, 1, format_int);
    Menu menu_mode("Mode");
      MenuItem menu_mode_dmx("Mode DMX", &on_menu_mode_dmx_selected);
      MenuItem menu_mode_standalone("Mode Standalone", &on_menu_mode_standalone_selected);
      //MenuItem menu_main.add_item(&menu_mode_standalone);
    Menu menu_color("Color menu");
      Menu menu_color_red("RED Color menu");
        NumericMenuItem menu_item_color_red("RED Color value", nullptr, 0, 0, 255, 1, format_int);
      Menu menu_color_green("GREEN Color menu");
        NumericMenuItem menu_item_color_green("GREEN Color value", nullptr, 0, 0, 255, 1, format_int);
      Menu menu_color_blue("BLUE Color menu");
        NumericMenuItem menu_item_color_blue("BLUE Color value", nullptr, 0, 0, 255, 1, format_int);
      Menu menu_color_white("WHITE Color menu");
        NumericMenuItem menu_item_color_white("WHITE Color value", nullptr, 0, 0, 255, 1, format_int);
      MenuItem menu_item_exit("Exit", &on_menu_exit);

// Menu callback function

// In this example all menu items use the same callback.

void on_menu_address_selected(MenuItem* p_menu_item){
  Serial.println("address changed");
}

void on_menu_mode_dmx_selected(MenuItem* p_menu_item){
  Serial.println("Mode set to DMX");
}
void on_menu_mode_standalone_selected(MenuItem* p_menu_item){
  Serial.println("Mode set to STANDALONE");
}

void on_menu_exit(MenuItem* p_menu_item){
  Serial.println("EXIT complete");
}
// writes the (int) value of a float into a char buffer.
const String format_int(const float value)
{
    return String((int) value);
}


void display_help() {
    Serial.println();
    Serial.println("***************");
    Serial.println("w: go to previus item (up)");
    Serial.println("s: go to next item (down)");
    Serial.println("a: go back (right)");
    Serial.println("d: select \"selected\" item");
    Serial.println("?: print this help");
    Serial.println("h: print this help");
    Serial.println("***************");
}

void serial_handler()
{
    char inChar;
    if ((inChar = Serial.read()) > 0)
    {
        switch (inChar)
        {
            case 'w': // Previus item
                ms.prev();
                ms.display();
                Serial.println("");
                break;
            case 's': // Next item
                ms.next();
                ms.display();
                Serial.println("");
                break;
            case 'a': // Back presed
                ms.back();
                ms.display();
                Serial.println("");
                break;
            case 'd': // Select presed
                ms.select();
                ms.display();
                Serial.println("");
                break;
            case '?':
            case 'h': // Display help
                ms.display();
                Serial.println("");
                break;
            default:
                break;
        }
    }
}

// Standard arduino functions

void setup()
{
    Serial.begin(9600);

/*
    ms.get_root_menu().add_item(&mm_mi1);
    ms.get_root_menu().add_item(&mm_mi2);
    ms.get_root_menu().add_menu(&mu1);
    mu1.add_item(&mu1_mi0);
    mu1.add_item(&mu1_mi1);
    mu1.add_item(&mu1_mi2);
    mu1.add_item(&mu1_mi3);
    ms.get_root_menu().add_item(&mm_mi4);
    ms.get_root_menu().add_item(&mm_mi5);
*/

    ms.get_root_menu().add_menu(&menu_root);
    
    menu_root.add_menu(&menu_main);    
      menu_main.add_menu(&menu_address);
        menu_address.add_item(&menu_item_address);
      menu_main.add_menu(&menu_mode);
        menu_mode.add_item(&menu_mode_dmx);
        menu_mode.add_item(&menu_mode_standalone);
      menu_main.add_menu(&menu_color);
        menu_color.add_menu(&menu_color_red);
          menu_color_red.add_item(&menu_item_color_red);
        menu_color.add_menu(&menu_color_green);
          menu_color_green.add_item(&menu_item_color_green);
        menu_color.add_menu(&menu_color_blue);
          menu_color_blue.add_item(&menu_item_color_blue);
        menu_color.add_menu(&menu_color_white );
          menu_color_white.add_item(&menu_item_color_white);
      menu_main.add_item(&menu_item_exit);
       
    display_help();
    ms.display();
    Serial.println("");
}

void loop()
{
    serial_handler();
}
