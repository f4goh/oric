/* 
 * File:   Menu.h
 * Author: ale
 * 
 * Created on 22 avril 2023
 */

#ifndef MENU_H
#define MENU_H
#include <Arduino.h>
#include <console.h>
#include <Preferences.h>        // Classe pour écrire et lire la configuration
#include "SPIFFS.h"
#include "Oric.h"
#define VERSION "F4GOH : Version 1.0"
class Menu {
public:
    Menu(Oric *p);
    Menu(const Menu& orig);
    virtual ~Menu();

    void run();
    void setup();

    // Méthodes associées aux commandes    
    static void _ssid_(ArgList& L, Stream& S);
    static void _pass_(ArgList& L, Stream& S);
    static void _raz_(ArgList& L, Stream& S);
    static void _serial_(ArgList& L, Stream& S);

    static void _ls_(ArgList& L, Stream& S);
    static void _rm_(ArgList& L, Stream& S);
    static void _load_(ArgList& L, Stream& S);
    static void _save_(ArgList& L, Stream& S);
    static void _view_(ArgList& L, Stream& S);
    static void _free_(ArgList& L, Stream& S);    
    static void _config_(ArgList& L, Stream& S);
    static void _internet_(ArgList& L, Stream& S);
    static void _reboot_(ArgList& L, Stream& S);
    static void _format_(ArgList& L, Stream& S);
    static void _basic_(ArgList& L, Stream& S);
    static void _conv_(ArgList& L, Stream& S);


    static void _help_(ArgList& L, Stream& S);
    static void _unknown(String& L, Stream& S);




private:
    
    Console *con;
    Preferences *configuration;
    Oric *loric;
    static Menu* anchor;
    bool acceptCmd(String cmd, int longMin, int longMax);

};

#endif /* MENU_H */
