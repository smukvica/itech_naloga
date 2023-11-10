# [itech naloga](https://github.com/smukvica/itech_naloga)

Client to read packets from network device and display, write and save the data. Also a simple simulator was created to simulate the device so the device isn't needed.

## Usage

### Server

Server first reads a set of arguments to setup it's environment. If the arguments aren't provided the default values are used.

The arguments that can be provided are as follows:
* ***number_of_fields*** - sets the number of data fields in each packet (limited from **1** to **10**)
* ***size_of_field*** - sets the size of each data field in packet in bytes (limited to **1**, **2** and **4**)
* ***number_of_packets*** - sets the number of packets to be sent
* ***number_of_bpm*** - sets the number of bpm the device will simulate
* ***send_rate*** - sets the amount of packets sent each second (very high rates may not be reached - *ex. 20000*)
* ***help*** - displays the help for arguments

The server will continue to send th packets until all packets are sent or the program is otherwise terminated.

### Client

The client has a similar set of parameters as the server with an addition of a couple more arguments:

* ***queue_size*** - size of the buffer used to receive packets into (limited from **200.000** to **100.000.000**)
* ***file_entries*** - number of entries in each file when saving (limited from **500** to **1000**)
* ***ip*** - the ip address of the device to connect to
* ***port*** - the port of the device to connect to
* ***writer*** - set saving to files (**1** to save, **0** not to save)
* ***output*** - set writing to standard out (**1** to write, **00** not to write)

Final *n* arguments are meant to be the names for the data fields. The names should be given as one name one argument.

All arguments can be omitted and the default values will be used.

A separate argument can be used to setup the file path to read from:

* ***read_file*** - followed by the file name

Once the program is started the parameters are first read from a file if it exists, after they are read from the arguments. They can then be tweaked in the GUI. After the parameters are set to desired values the setup needs to be finished with the press on the button **Setup**. Once the setup is finished we can connect to the device or simulator with the button **Start** or we can read from the file with the button **Read file**.

Once the data is aquired it is displayed in the GUI. When the graph is full, we can click the button **Refresh data** which updates the data to display.

- if reading from file the next 500 values are aquired from the last aquired value
- if reading from device the values are aquired from the last received value regardles of how many samples have arrived between

The client will output at the end the time running, packages aquired, packages per second and how many errors were in the order of packages.

## Compiling

### Server

To compile the server move to the server folder and type the command:

    gcc server.c -fopenmp -lm -o server

### Client

To compile the server move to the client folder and type the command:

    gcc *.c -fopenmp -lm -lpthread -lraylib -o client

For compiling and using the client the [raylib](https://github.com/raysan5/raylib) and [raygui](https://github.com/raysan5/raygui) library are needed. Raygui is already included since it's a header only library. Raylib has to be insalled separately.