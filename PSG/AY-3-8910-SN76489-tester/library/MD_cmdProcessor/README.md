# Library for simple Command Line Interface

[![arduino-library-badge](https://www.ardu-badge.com/badge/MD_cmdProcessor.svg?)](https://www.ardu-badge.com/MD_cmdProcessor)

Table driven command line interface with basic syntax useful for simple requirements like testing applications. Reads input from Stream passed as a parameter, and writes to Stream output.

A command is defined as 

    <command_line> = <cmd>|<cmd><space><parameters>[<separator><command_line>]<eoln>
    <cmd> = <string>
    <parameters> = <string>
    <string> = <character><string>
    <space> = ' '
    <separator> = ';'
    <eoln> = '\n'
    <character> = any ASCII except for <space>, <separator> or <newline>

Valid commands are defined in a table the calling application. The table should be located in flash memory (PROGMEM). Commands are case insensitive (ie, "A" and "a" are the same). Handlers (callback functions in user code) are also defined to handle each command. This class will manage the user input and call the relevant handler with the parameters passed for that command.

If you like and use this library please consider making a small donation using [PayPal](https://paypal.me/MajicDesigns/4USD)
