#ifndef WEBUI_H
#define WEBUI_H

#include <Arduino.h>
#include <WebServer.h>
#include <Preferences.h>
#include "SPIFFS.h"
#include "Oric.h"

typedef enum {
    WEB_ACT_NONE = 0,
    WEB_ACT_CLOAD,
    WEB_ACT_CSAVE
} WebPendingAction;

class WebUi {
public:
    WebUi(Oric *oric);
    void begin(bool apMode, const String &ipStr);
    void run();
    void setNetworkInfo(bool apMode, const String &ipStr);
    bool takePendingAction(WebPendingAction &act, String &fileName);

private:
    static void webTask(void *param);
    void handleClientLoop();

    void handleRoot();
    void handleStatus();
    void handleFiles();
    void handleUploadData();
    void handleUploadDone();
    void handleDownload();
    void handleDelete();
    void handleCload();
    void handleCsave();
    void handleBasic();
    void handleHex();
    void handleDisasm();
    void handleConvert();
    void handleGetConfig();
    void handlePostConfig();
    void handleFormat();
    void handleReboot();

    static String cleanName(String name);
    static String jsonEscape(const String &s);
    static const char *stateToText(K7State st);

    Oric        *loric;
    WebServer    server;
    TaskHandle_t taskHandle;
    bool         isApMode;
    String       ipAddress;

    volatile WebPendingAction pendingAction;
    String                    pendingFile;
    volatile bool             rebootPending;
    uint32_t                  rebootAtMs;
    File                      uploadFile;
    bool                      uploadError;
};

#endif /* WEBUI_H */
