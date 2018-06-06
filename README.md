# CSE 392 - Spring 2018 - Homework 1

## **Client Due in Lecture on Thursday 02/22/18**
## **Server Due on Sunday 03/04/18 @ 11:59PM**

# Introduction

In this assignment you will be creating a chat service much like Google's Hangouts, called **ME2U Chat**.
The goal is to learn about network programming in C.
You will implement the service so that it adheres to the **ME2U Protocol**.

The concept of a protocol is an important one to understand.
When implementing the protocol you need to follow the requirements and description **TO THE LETTER**.
The idea is to create a standard so that anyone implementing the `ME2U` Protocol will be able to connect and operate with any other program implementing the same protocol.
Any client and chat should work with any server if the protocol is used correctly.

You may also want to test your client by connecting to another group’s server, and your server by having other group’s clients connect to it.
This is a good way to ensure that the protocol is implemented correctly in your client/server programs.
While we encourage testing this way, THIS IS NOT an open invitation to share code with other groups.
You should not be showing or sharing code with anyone else.
To perform these tests you should simply be telling your client programs to connect to the IP address and port number at which these servers are running at.
You have been warned!

# Helpful Resources

The relevant sections from __UNIX Network Programming__ are 2.2, 2.4, 2.9-10, and chapters 3-5.

A pretty good reference on sockets which summarizes the man pages can be [found here](https://web.archive.org/web/20161015220642/http://www.python4science.eu/sockets.html).

Additionally, you may also want to read [Beej’s Guide to Network Programming](http://beej.us/guide/bgnet/).
It has a good mix of tutorials, explanations, and descriptions of the functions you need to use for this assignment.

## Using netcat as a faux server

You can use **netcat** as a faux server to test your client program.
First create a basic server listening on port **PORT** on localhost.

```
$ nc -C -l -p PORT
```

That's all you need to do.
The **netcat** program is now waiting to accept connections on port **PORT**, and it will write anything it receives to **stdout**.
So now if you have your client program to connect to localhost **PORT**, you should see the protocol displayed plainly.

Netcat will also allow you to type back to the connected socket if you type in it.
You can type out a protocol response, and then **press enter twice** to send `\r\n\r\n`.
To end the netcat program, press **ctrl-c** to terminate.

## Seeing raw bytes over the network interface

You can use [tcpdump(8)](http://linux.die.net/man/8/tcpdump) or [ngrep(8)](http://linux.die.net/man/8/ngrep) to watch all network communications over a particular device.

The below instructions will focus on using **ngrep** as a way to monitor both the hex and ASCII values of all network communications over a device on your machine over a particular port.

```sh
sudo apt-get install ngrep
```

Use **ngrep** with the following configuration to see all the raw traffic going between the server and client programs.

```
sudo ngrep -qx -d DEVICE  port PORT
```

You need to replace **DEVICE** and **PORT** with the appropriate values. You can get device by running the **ip addr show** program.

```
$ ip addr show
1: lo: <LOOPBACK,UP,LOWER_UP> mtu 65536 qdisc noqueue state UNKNOWN group default qlen 1000
    link/loopback 00:00:00:00:00:00 brd 00:00:00:00:00:00
    inet 127.0.0.1/8 scope host lo
       valid_lft forever preferred_lft forever
    inet6 ::1/128 scope host
       valid_lft forever preferred_lft forever
2: wlp2s0: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc pfifo_fast state UP group default qlen 1000
    link/ether xx:xx:xx:xx:xx:xx brd ff:ff:ff:ff:ff:ff
    inet xxx.xxx.xxx.xxx/22 brd xxx.xxx.xxx.255 scope global dynamic wlp2s0
       valid_lft 10450sec preferred_lft 10450sec
    inet6 xxxx::xxxx:xxx:xxxx:xxxx/64 scope link
       valid_lft forever preferred_lft forever

```

On the machine that `ip addr show`  was run on, one could replace **\<device\>** with **lo** or **wlp2s0**.
If you wanted to listen on localhost over port 1234, you would run **ngrep** with the following arguments.
Then try to connect a client and watch the communications come pass through the wire. (It requires root privileges to sniff the traffic)

```
sudo ngrep -qx -d lo port 1234
```

To quit **ngrep** press **ctrl-c**.

## Colored Messages

To make things easier to read, you should use the following ansii escape codes to help color your messages on the server screen.

| Verbose    | Errors     | Default |
|------------|------------|---------|
| \x1B[1;34m | \x1B[1;31m | \x1B[0m |

> :nerd: If you don't remember what the ansii escape codes are/do take a look [here](http://ascii-table.com/ansi-escape-sequences.php).

## IPC

Later in this assignment you will need to use `IPC` to communicate between the client and chat windows.

On a Linux enviornment there are many forms of `IPC` that you can use such as [pipes and FIFOs](https://linux.die.net/man/7/pipe), [unix domain sockets](https://linux.die.net/man/7/unix), and [shared memory](https://linux.die.net/man/7/shm_overview).

## I/O Multiplexing

The client and server will need to use some form of I/O multiplexing in order to complete their tasks.

I/O multiplexing is available through the following interfaces - [select](http://linux.die.net/man/2/select), [poll](http://linux.die.net/man/2/poll), and [epoll](http://linux.die.net/man/2/epoll_create).
They all have their pros and cons, but it is up to you to decide which one(s) your client and server will use.
If you need help making a decision about which one to use, take a look at [epoll vs kqueue](http://people.eecs.berkeley.edu/~sangjin/2012/12/21/epoll-vs-kqueue.html) and [select/poll/epoll](https://www.ulduzsoft.com/2014/01/select-poll-epoll-practical-difference-for-system-architects/).

# ME2U Protocol

## Notes

The diagrams below represent exchanges in the `ME2U` protocol, which the client and server will use for communication.
- The protocol is in plain text
- `\r\n\r\n` is the "terminating sequence" that denotes the end of a protocol message
    - This is a carriage return and line feed repeated twice
    - In hex this is `0x0D0A0D0A`
- Any components contained in `<` and `>` are meant to be dynamically filled in by your program and are **NOT** to be bracketed by `<` and `>`
    - Ex. `TO <name> <message>\r\n\r\n` could be `TO neal hello, friend!\r\n\r\n`
- Almost every protocol message has a reversed string reply that does **NOT** include the payload. Pay close attention to the format / required response
- Usernames are limited to 10 characters and cannot contain spaces

## Login Procedure

The login procedure starts when a client initiates a connection with the server.
The client and server will perform a handshake and the client will send the server a username.
If the client's username is available, the server will acknowledge it and send it the message of the day.

<!--
Title Login Protocol
Client->Server: ME2U\\r\\n\\r\\n
Server->Client: U2EM\\r\\n\\r\\n
Client->Server: IAM <name>\\r\\n\\r\\n
alt Username Taken
Server->Client: ETAKEN\\r\\n\\r\\n
Note left of Client: Client prints "Username Taken" and quits
else Username Not Taken
Server->Client: MAI\\r\\n\\r\\n
Server->Client: MOTD <message>\\r\\n\\r\\n
Note left of Client: Client prints MOTD message and prompts user for input
-->

![Login Protocol](imgs/loginProtocol.png)

## List User Procedure

The list user procedure starts when the client sends the server a request for a list of all users.
The server should respond with a list of all connected users.


<!--
Title List Users Protocol

Client->Server: LISTU\\r\\n\\r\\n
Server->Client: UTSIL <name1> <name2> <name3> ... <nameN>\\r\\n\\r\\n
-->

![List Protocol](imgs/listProtocol.png)

> :nerd: When implementing this protocol, you should think about how you can receive an arbitrarily sized list of users.

## Typical Messaging Procedure

Below is a simple exchange between two logged in clients.
When one of the clients tries to contact a client that does not exist, the server responds with an `EDNE` message. **NOTE THE ORDER OF THE MESSAGES**.
The `TO` is sent to the server, `FROM` is sent to the destination client, destination client responds with `MORF` **THEN** the server replies to the orignal client with an `OT` message.
**Note** the usernames being filled in to each corresponding message.

<!--
Title Message Protocol
Bob->Server: TO Alice <message>\\r\\n\\r\\n
Server->Alice: FROM Bob <message>\\r\\n\\r\\n
Alice->Server: MORF Bob\\r\\n\\r\\n
Server->Bob: OT Alice\\r\\n\\r\\n
Alice->Server: TO Bob <message>\\r\\n\\r\\n
Server->Bob: FROM Alice <message>\\r\\n\\r\\n
Bob->Server: MORF Alice\\r\\n\\r\\n
Server->Alice: OT Bob\\r\\n\\r\\n

Title User Does Not Exist Protocol
Bob->Server: TO Jim <message>\\r\\n\\r\\n
note right of Server: Jim does not exist
Server->Bob: EDNE Jim\\r\\n\\r\\n
-->

![Message Protocol](imgs/messageProtocol.png)

If the user does not exist you should implement the following protocol.

![User DNE Protocol](imgs/userDoesNotExistProtocol.png)

## Logout Procedure

When a client wants to log out it will send the server a `BYE` message.
After acknowledging the client, the server will inform all other connected clients that they have logged out by broadcasting a `UOFF` message.

<!--
Title List Users Protocol

ClientA->Server: BYE\\r\\n\\r\\n
Server->Client: EYB\\r\\n\\r\\n

Server->Client1: UOFF ClientA\\r\\n\\r\\n
Server->Client2: UOFF ClientA\\r\\n\\r\\n
Server->Client3: UOFF ClientA\\r\\n\\r\\n
Server->Client...: UOFF ClientA\\r\\n\\r\\n
note right of Client...: UOFF is broadcast to all logged in clients
-->

![Logout Protocol](imgs/logoutProtocol.png)

## Handling Errors

If the client or server receives a poorly formated message from their peer, they should immediately terminate the connection and print an error message.

![Error case](imgs/errorProtocol.png)

# ME2U Client

This part of the homework document outlines the expected functionality and operation of a `ME2U` chat client.
This part of the homeework assignent must be implemented in the `C` programming language.

## Usage

Below is a help menu that describes the arguments needed to run the client.
Remember that options in **[  ]** are optional, and arguments which are not assigned a flag are positional.

```sh
./client [-hv] NAME SERVER_IP SERVER_PORT
-h                         Displays this help menu, and returns EXIT_SUCCESS.
-v                         Verbose print all incoming and outgoing protocol verbs & content.
NAME                       This is the username to display when chatting.
SERVER_IP                  The ip address of the server to connect to.
SERVER_PORT                The port to connect to.
```

> :nerd: To make things easier to read, use ansi escape sequences to color the messages your client prints.

## Overview

The ME2U Chat program will allow users logged into a ME2U Chat server to send messages to each other.
Chats between users will be displayed on the client machine in independent windows, just as Google Hangouts does.

![Client Figure](imgs/HW1_ClientFigure.png)

The diagram above shows the internal control flow of the client.
On startup, the client will connect to the server using the specified `SERVER_IP` ip address and `SERVER_PORT` port number.
After establishing a connection with the server, the client will attempt to log in via the `ME2U` protocol.
If the client is able to successfully log in, it will listen for communication from the server while handling commands from `stdin`.

## Client Commands

In order to fulfill user requests, the client will handle input from `stdin`.
These commands are transparent to the server as it only involves the user and client program.

#### `/help`
* Prints all the commands which the client accepts, at minimum these four, to `stdout`.

#### `/logout`
* Initiates the logout procedure with the server.
* This command also closes all chat windows and open file descriptors prior to exiting.

#### `/listu`
* Requests a list of logged in users from the server, and prints the list to `stdout`.

#### `/chat <to> <msg>`
* This command requires the name of the user to send the message to, and the message to be sent.
* More information about this command can be found in the section below.
* If the user exists, this command should also spawn a chat window for further communication. **Note** the client should block until it recieves confirmation that the other user has gotten the message. Only after this confirmation should a new window be spawned.
* If the user does not exist, then this command should print an error message.

## Chat Program

In order to simulate the functionality of Google Hangouts, which spawns new windows for every chat, you will have to implement a `chat` program that communicates with your client program.

When the user wants to begin a chat session with another user, they will run the `/chat` command.
If the recipient exists, the client should spawn an [xterm](https://linux.die.net/man/1/xterm) window running an instance of the `chat` program.
If the recipient does not exist, the client should print an error message.
A chat window should also be opened when the recipient's client receives the message.

Once both users have an `xterm` chat window open, they can freely send messages to one another.
Incoming messages should be proceeded by an `>` symbol, and outgoing messages by an `<` symbol.
It may also help to color these messages to make them easier to identify.
After a message is entered in the `xterm` window's `stdin`, the chat program should use some form of `IPC` to relay the message to the client.
The client then sends the message to the server for routing.

A user can close a chat window at anytime using either the close button on the `xterm` window, or by typing `/close`.
When the window is closed on one client, it should have no impact on the client who was chatting with the user, i.e., the client should not communicate with the server when this command is used.
If a user closes a chat window they must use the `/chat` command in the client to initiate the chat again and reopen the window.
The user who did not close their chat window should receive this message normally.

If implemented correctly, it doesn’t matter if the `xterm` window is closed.
Every time a message is received by the client it should check to see if a window already exists for that user, and if not, it will spawn one.
If the window does exist, then the message is passed through IPC and displayed appropriately on the chat window.
If one of the user's disconnects from the server or does not exist, but a message is sent to this user, the server will respond with an `EDNE` message.

# ME2U Server

This part of the homework document outlines the expected functionality and operation of a `ME2U` chat server.
This part of the homework assignment can be implemented in the `C` or `Python` programming languages.

## Usage

Below is a help menu that describes the arguments needed to run the server.
Remember that options in **[  ]** are considered optional, and arguments which are not assigned a flag are positional.

```sh
./server [-hv] PORT_NUMBER NUM_WORKERS MOTD
-h            Displays help menu & returns EXIT_SUCCESS.
-v            Verbose print all incoming and outgoing protocol verbs & content.
PORT_NUMBER   Port number to listen on.
NUM_WORKERS   Number of workers to spawn.
MOTD          Message to display to the client when they connect.
```
> :nerd: To make things easier to read, use ansi escape sequences to color the messages your server prints.

## Overview

![Server Figure](imgs/HW1_ME2UServerFigure.png)

The diagram above shows the internal control flow of the server.
On startup, the main thread will create `NUM_WORKERS` `workers` which will remain active for the lifetime of the server.
The main/multiplexer thread will also create a socket, bind it to a specified port, and begin listening for incoming connections.
Additionally, the `main/multiplexer` thread will also parse commands from `stdin` and pass them to the workers to be run.

Clients will attempt to establish a connection with the server, which will be accepted by the `main/multiplexer` thread.
For every connection that the main thread accepts, the login job will be sent to the worker queue and be handled by the next available worker.
The server will recieve messages from logged in users on their respective client sockets in the `main/multiplexer` thread. The `main/multiplexer` thread will simply pass these messages to workers to be processed appropriately.

## Server Commands

Aside from handling connections from various clients, the server also has to handle input from `stdin`.
These commands are transparent to the connected clients, and are mainly used as a way to gather information by the server operator.

#### `/users`
* Dumps a list of currently logged in users to `stdout`. (**NOTE:** this does not send anything over the network, this is only on the server side)

#### `/help`
* Prints all the commands which the server accepts, at minimum these three, to `stdout`.

#### `/shutdown`
* Cleanly shuts the server down by disconnecting all connected users, closing all open file descriptors, and freeing any allocated memory.

## How to submit your assignment

To submit your assignment for grading, tag the commit you intend to submit, with `hw1-client` for the client and `hw1-server` for the server, before the deadline.
This can be done through the Gitlab user interface, or through the git command line using the following command.

```sh
git tag -a hw1-XXXXXX [commit]
git push origin --tags
```

> :warning: You only get one change to tag.
> Make sure all your files are committed and push to the gitlab server before you tag.
