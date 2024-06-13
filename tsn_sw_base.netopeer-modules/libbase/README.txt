To build and install librray, test app and execute test app
    $ ./install_lib.bash
    
Just base library build:
    $ make all

If we need debug messages 
    $ make all DEBUGPRINT=-DDEBUGP

To build library and test app
    $ make all dyn_app DEBUGPRINT=-DDEBUGP
