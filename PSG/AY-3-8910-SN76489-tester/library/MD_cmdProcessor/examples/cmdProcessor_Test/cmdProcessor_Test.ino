
#include <MD_cmdProcessor.h>

#define ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))

// handler function prototypes
void handlerHelp(char* param);

// handler functions
void handlerA1(char *param)
{
  Serial.print("\nA1 param:");
  Serial.print(param);
}

void handlerZ9(char *param)
{
  Serial.print("\nZ9 param:");
  Serial.print(param);
}

const MD_cmdProcessor::cmdItem_t PROGMEM cmdTable[] =
{
  { "?",  handlerHelp, "",    "Help", 0 },
  { "h",  handlerHelp, "",    "Help", 0 },
  { "a1", handlerA1,   "123", "Command A1", 1 },
  { "Z9", handlerZ9,   "456", "Command Z9", 1 },
};

MD_cmdProcessor CP(Serial, cmdTable, ARRAY_SIZE(cmdTable));

void handlerHelp(char* param)
{
  Serial.print(F("\nHelp\n----"));
  CP.help();
  Serial.print(F("\n"));
}

void setup(void) 
{
  Serial.begin(57600);
  Serial.print(F("\nMD_cmdProcessor Example\n-----------------------"));
  Serial.print(F("\nEnter command. Ensure line ending set to newline.\n"));
  CP.begin();
  CP.help();
}

void loop(void) 
{
  CP.run();
}
