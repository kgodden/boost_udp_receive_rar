# boost_udp_receive_rar
The boost::asio classes provide a cross platform & flexible way to receive and send UDP data - however they can be confusing and hard to use, and for some simpler applications they may feel like overkill.  Sometimes I want to just receive a string via UDP by providing and IP address and Port without having to worry about all the millions of other details that can be involved in network programming!

Thats where **boost_udp_receive_rar** comes in, this class just has 4 functions that allow you to receive a UDP datagram as a **std::string** or s**td::vector<unsigned char>** in a synchronous or asynchronous manner.
  
The boost::asio lib will throw excpetions if anything goes wrong (which it often does with network programming) so it is adviable to wrap any code that uses **boost_udp_receive_rar** in a try/catch block that catches **boost::system::system_error**.
  
The 4 functions are:
- ```std::string receive_sync()``` - Receives a datagram as a string (blocking)
- ```std::vector<unsigned char> receive_binary_sync()``` - Receives a datagram as binary (blocking)
- ```std::string receive_async()``` - Receives a datagram as a string (non-blocking)
- ```std::vector<unsigned char> receive_binary_async()``` - Receives a datagram as binary (non-blocking)

Note that this class deals with receiving UDP data only, for an easy way to send UDP data see: [boost_udp_send_faf](https://github.com/kgodden/boost_udp_send_faf)
  
RAR stands for "Receive and Rejoice!"
  
To use **boost_udp_receive_rar** just include the header file:
  
```
#include "boost_udp_receive_rar.h"
```
The boost libraries must also be included in your build.
  
## To receive a UDP datagram as a string

```  
// Setup a receiver, specifying IP address and port
// Note the IP address is that of the receiving network
// interface.  As always with UDP and non-standard ports
// check that a firewall isn't blocking data transfer.
boost_udp_receive_rar rar("127.0.0.1", 8861);

// Receive a datagram synchronosuly as a string
//
std::string datagram = rar.receive_sync();

cout << "Sync received string: " << datagram << endl;
```

## To receive a UDP datagram as binary
  
```
// Receive some binary data synchronously
//
std::vector<unsigned char> data = rar.receive_binary_sync();

cout << "Sync, received a datagram of size " << data.size() << endl;  
```
  
## To Asynchronously receive a UDP datagram as a string
```
// Asyncrounsly receive a datagram as a string
//
// We loop calling receive_async() until it returns
// a non-empty string. receive_async() returns quickly
// and we can do other work (or other async reads on different
// ports) while we wait for somethign to arrive.
//
std::string datagram;
  
do {
  datagram = rar.receive_async();

  // Do other stuff
  //
  // Better do a little sleep to be nice to the
  // CPU
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

} while (datagram.empty());

cout << "Async, received string: " << datagram << endl;

```
## To Asynchronously receive a UDP datagram as binary
  
```
std::vector<unsigned char> data;

do {
  data = rar.receive_binary_async();

  // Do other stuff
  //
  // Better do a little sleep to be nice to the
  // CPU
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
} while (data.empty());

```
# To build the tests for Ubuntu:
  
- Make sure boost is installed (e.g. To install: ```sudo apt-get install libboost-all-dev```)
- In terminal, ```cd``` to the test directory and run ```make```
- Run the tests bu executing: ```./test_boost_udp_receive_rar```
  
## Building the tests with Visual Studio
  There is a VS2017 based solution to build the tests in the test directrory, you will have to change the include and library directories for boost in the project settings to match your system's configuration.  Buld the x86 Configuration.
