/* 
 * File:   Menu.cpp
 * Author: ale
 * 
 * Created on 22 avril 2023
 */

#include "Menu.h"

Menu::Menu(Oric *p) :
con(new Console()),
configuration(new Preferences()),
loric(p)
{
    anchor = this;
    
}

Menu::Menu(const Menu& orig) {
}


Menu::~Menu() {

    delete configuration;
    anchor = NULL;
}

void Menu::run() {
   con->run();
}

void Menu::setup() {

    con->onCmd("help", _help_);
    con->onCmd("ssid", _ssid_);
    con->onCmd("pass", _pass_);
    con->onCmd("raz", _raz_);
        
    con->onCmd("serial", _serial_);
    con->onCmd("ls", _ls_);
    con->onCmd("rm", _rm_);
    con->onCmd("cload", _load_);
    con->onCmd("csave", _save_);
    con->onCmd("free", _free_);
    con->onCmd("internet", _internet_);
    con->onCmd("show", _config_);
    con->onCmd("reboot", _reboot_);
    con->onCmd("format", _format_);
    con->onCmd("view", _view_);
    con->onCmd("basic", _basic_);
    con->onCmd("free", _free_);
    con->onCmd("conv", _conv_);
    con->onCmd("des", _des_);   
    con->onUnknown(_unknown);
    

    con->start();

    anchor->configuration->begin("oric", false);

    if (anchor->configuration->getBool("config", false) == false) {
        anchor->configuration->putString("ssid", "nossid");
        anchor->configuration->putString("pass", "nopassword");
        anchor->configuration->putBool("serial", false);
        anchor->configuration->putBool("internet", false);
        anchor->configuration->putBool("config", true);
    }
    anchor->configuration->end();
    this->run();
    con->setPrompt("Oric> ");
    Serial.println(F("console start"));
}


void Menu::_help_(ArgList& L, Stream& S) {
    S.println(F("Available commands"));
    S.println(F("Set ssid                        : ssid mywifi"));
    S.println(F("Set password                    : pass toto"));
    S.println(F("ls                              : list tap files in SPIFFS"));  //telnet
    S.println(F("info                            : SPIFFS info space size"));    //telnet
    S.println(F("rm filename.tap                 : remove tap files in SPIFFS"));  //telnet
    S.println(F("cload filename.tap              : load fsk from esp32 to oric"));   //telnet
    S.println(F("csave filename.tap              : save fsk from oric to esp32"));   //telnet
    S.println(F("view filename.tap               : view file content in HEX/ASCII format")); //telnet
    S.println(F("basic filename.tap              : view file content in BASIC")); //telnet
    S.println(F("conv source.bmp dest.tap        : convert bmp monochrome image to tap file")); //telnet non
    S.println(F("des source.tap dest.txt         : dessasemble tap file to txt file")); //telnet non
    S.println(F("Enable serial for telnet        : serial 1")); //if enable telnet cmd is disable
    S.println(F("Enable wifi                     : internet 1"));
    S.println(F("Enable local wifi Access Point  : internet 0"));
    S.println(F("Show configuration              : show"));
    S.println(F("Reset default configuration     : raz"));
    S.println(F("Format SPIFFS                   : format"));
    S.println(F("Reboot ESP32                    : reboot"));
    S.println(F("help                            : this menu")); 
    S.println(F(VERSION)); 
}

void Menu::_free_(ArgList& L, Stream& S) {
    // Affiche les tailles totale et utilisée
    unsigned int totalBytes = SPIFFS.totalBytes();
    unsigned int usedBytes = SPIFFS.usedBytes();
    Serial.println();
    Serial.println("===== Flash File System Info =====");
    Serial.print("Total space:      ");
    Serial.print(totalBytes);
    Serial.println(" octets");
    Serial.print("Total space used: ");
    Serial.print(usedBytes);
    Serial.println(" octets");
    Serial.println();
}



bool Menu::acceptCmd(String cmd, int longMin, int longMax) {

    if (cmd.length() >= longMin && cmd.length() <= longMax) {
        return true;
    } else {
        Serial.println("Erreur");
        return false;
    }
}

void Menu::_unknown(String& L, Stream& S) {
    S.print(L);
    S.println(" : command not found");
}


void Menu::_ssid_(ArgList& L, Stream& S) {
    String arg;
    String p;
    bool ret;
    while (!(arg = L.getNextArg()).isEmpty()) {
        p = p + arg + " ";
    }
    p=p.substring(0,p.length()-1);
    ret = anchor->acceptCmd(p, 3, 32);
    if (ret == true) {
        anchor->configuration->begin("oric", false);
        S.printf("ssid is %s\n\r", p.c_str());
        anchor->configuration->putString("ssid", p);
        anchor->configuration->end();
    }
}



void Menu::_pass_(ArgList& L, Stream& S) {
    String p;
    bool ret;
    p = L.getNextArg();
    ret = anchor->acceptCmd(p, 3, 63);
    if (ret == true) {
        anchor->configuration->begin("oric", false);
        S.printf("password is %s\n\r", p.c_str());
        anchor->configuration->putString("pass", p);
        anchor->configuration->end();
    }
}

void Menu::_load_(ArgList& L, Stream& S) {

    String p;

    if (!(p = L.getNextArg()).isEmpty()) {        
       anchor->loric->sendFile(p);
    } else {
        S.printf("Usage cload filename.tap\r\n");
    }
}

void Menu::_save_(ArgList& L, Stream& S) {

    String p;

    if (!(p = L.getNextArg()).isEmpty()) {
        Serial.println("En Attente du CSAVE...");
        anchor->loric->saveFile(p);
        //Serial.println("fichier ecrit");
    } else {
        S.printf("Usage csave filename.tap\r\n");
    }
}


void Menu::_view_(ArgList& L, Stream& S) {
    String p;
    if (!(p = L.getNextArg()).isEmpty()) {
        anchor->loric->viewFile(p);
    } else {
        S.printf("Usage view filename.tap\r\n");
    }
}

void Menu::_basic_(ArgList& L, Stream& S) {
    String p;
    if (!(p = L.getNextArg()).isEmpty()) {        
       anchor->loric->tap2Bas(p);
    } else {
        S.printf("Usage basic filename.tap\r\n");
    }
}

void Menu::_conv_(ArgList& L, Stream& S) {
    String p,fileSource,fileDest;
    if (!(p = L.getNextArg()).isEmpty()) {        
        fileSource=p;
    } if (!(p = L.getNextArg()).isEmpty()) {        
        fileDest=p;
        anchor->loric->conversion(fileSource,fileDest);
    }else {
        S.printf("Usage conv source.bmp dest.tap\r\n");
        S.printf("Must be 240x200 in monochome BMP\r\n");
    }
}

void Menu::_des_(ArgList& L, Stream& S) {
    String p,fileSource,fileDest;
    if (!(p = L.getNextArg()).isEmpty()) {        
        fileSource=p;
    } if (!(p = L.getNextArg()).isEmpty()) {        
        fileDest=p;
        anchor->loric->desassemble(fileSource,fileDest);
    }else {
        S.printf("Usage des source.tap dest.txt\r\n");
    }
}

void Menu::_ls_(ArgList& L, Stream& S) {
    anchor->loric->listFile();
}

void Menu::_rm_(ArgList& L, Stream& S) {
   String p,filename;
    if (!(p = L.getNextArg()).isEmpty()) {
        anchor->loric->removeFile(p);
    } else {
        S.printf("Usage rm filename.tap\r\n");
    }
}


void Menu::_config_(ArgList& L, Stream& S) {
    anchor->configuration->begin("oric", false);
    Serial.printf("Ssid is             : %s\n\r", anchor->configuration->getString("ssid").c_str());
    Serial.printf("password is         : %s\n\r", anchor->configuration->getString("pass").c_str());
    Serial.printf("Serial is           : %s\n\r", anchor->configuration->getBool("serial") ? "Enable" : "Disable");
    Serial.printf("Internet is         : %s\n\r", anchor->configuration->getBool("internet") ? "Enable" : "Disable");
    Serial.printf("TCP Access point is : %s\n\r", anchor->configuration->getBool("internet") ? "Disable" : "Enable");
    anchor->configuration->end();
}

void Menu::_raz_(ArgList& L, Stream& S) {
    anchor->configuration->begin("oric", false);
    anchor->configuration->putBool("config", false);
    //anchor->configuration->clear();  
    ESP.restart();
}

void Menu::_serial_(ArgList& L, Stream& S) {
    String p;
    bool ret;
    p = L.getNextArg();
    ret = anchor->acceptCmd(p, 1, 1);
    if (ret == true) {
        anchor->configuration->begin("oric", false);
        anchor->configuration->putBool("serial", (int8_t) p.toInt());
        Serial.printf("Serial is : %s\n\r", anchor->configuration->getBool("serial") ? "Enable" : "Disable");        
        anchor->configuration->end();
    }
}

void Menu::_internet_(ArgList& L, Stream& S) {
    String p;
    bool ret;
    p = L.getNextArg();
    ret = anchor->acceptCmd(p, 1, 1);
    if (ret == true) {
        anchor->configuration->begin("oric", false);
        anchor->configuration->putBool("internet", (int8_t) p.toInt());
        Serial.printf("Internet is : %s\n\r", anchor->configuration->getBool("internet") ? "Enable" : "Disable");
        Serial.printf("TCP Access point is : %s\n\r", anchor->configuration->getBool("internet") ? "Disable" : "Enable");
        anchor->configuration->end();
    }
}

void Menu::_reboot_(ArgList& L, Stream& S) {
     ESP.restart();
}

void Menu::_format_(ArgList& L, Stream& S) {
    anchor->loric->formatSpiffs();
}


Menu* Menu::anchor = NULL;


