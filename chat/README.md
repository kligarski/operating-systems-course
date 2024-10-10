# TCP/IP Socket Chat
## Task description
Write a simple client-server chat application where communication between 
chat participants/clients and the server is carried out via sockets using the stream protocol.

The server address/port is provided as an argument when the server is launched.

The client takes the following arguments:
- the client's name/identifier (a string with a predetermined maximum length)
- the server address (IPv4 address and port number)


The communication protocol should support the following operations:

### LIST:
Retrieve and display all active clients from the server.

### 2ALL string:
Send a message to all other clients. The client sends a string to the server, 
and the server distributes this string along with the sender's identifier 
and the current date to all other clients.

### ONE client_id string:
Send a message to a specific client. The client sends a string to the server, 
specifying the recipient as a client with a particular identifier from 
the list of active clients. The server sends this string along with 
the sender's identifier and the current date to the specified client.

### STOP:
Notify the server of the client's exit from the chat. This should result in 
the client being removed from the list of clients maintained by the server.

### ALIVE:
The server should periodically "ping" registered clients to verify that they 
are still responsive, and if not, remove them from the list of clients.

When the client is closed using Ctrl+C, it should deregister itself 
from the server.

For simplicity, assume that the server stores information about clients 
in a static array (the size of the array limits the number of clients 
that can simultaneously participate in the chat).

## Usage
Run a server and then run as many clients as you want.
```bash
make
./server <IPv4_address> <port_number>
./client <username> <IPv4_address> <port_number>
```

### Example
```bash
./server 127.0.0.1 65432
./client Foo 127.0.0.1 65432
./client Bar 127.0.0.1 65432
```