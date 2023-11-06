# Assignment 4
## Overview

## Server Component

This component of the project acts as the server-side application. It handles client connections, serves requested data, and manages the overall communication. Below, you'll find an overview of the files and their functionalities in the "server" folder.

## Files:

### 1. `server.cpp`

- **Purpose**: Main entry point for the server application.
- **Functionality**:
  - Parses command-line arguments.
  - Creates a server socket.
  - Binds the socket to a specific IP address and port.
  - Listens for incoming connections.
  - Spawns a thread for detecting exit input.
  - Accepts client connections (incomplete).

### 2. `function.h`

- **Purpose**: Header file containing utility functions used by the server.
- **Functionality**:
  - Provides functions for managing logs.
  - Reads data from files.
  - Handles command-line arguments.

### 3. `server.h`

- **Purpose**: Header file defining global variables and including necessary headers.
- **Functionality**:
  - Defines global variables for server configuration.

## Usage:

To compile and run the server application, follow these steps:

1. Traverse the directory: `cd server`
2. Compile the code: `g++ server.cpp`
3. Run the server: `./a.out <TRACKER INFO FILE> <TRACKER NUMBER>`
4. Ex:  `./a.out tracker_info.txt 1`

# Client Component

This component of the project represents the client-side application. It interacts with the server, manages file transfers, and handles various client-server communications. Below, you'll find an overview of the files and their functionalities in the "client" folder.

## Files:

### 1. `client.cpp`

- **Purpose**: Main entry point for the client application.
- **Functionality**:
  - Parses command-line arguments.
  - Creates a client socket.
  - Spawns a server thread.
  - Connects to a server.

### 2. `client.h`

- **Purpose**: Header file containing global variables and function declarations used by the client code.
- **Functionality**:
  - Defines global variables for client configuration.
  - Declares functions for handling command-line arguments, logging, and connecting to the server.

### 3. `clientFunc.h`

- **Purpose**: Header file containing utility functions used by the client.
- **Functionality**:
  - Defines a function for logging client actions.
  - Provides functions for splitting strings, reading server details from a file, clearing log data, and connecting to a server.

### 4. `clientProcess.h`

- **Purpose**: Header file containing functions for handling client-server interactions.
- **Functionality**:
  - Defines a function for sending a chunk of a file to the server.
  - Handles incoming client requests.
  - Implements a server that listens for incoming connections and spawns threads to handle client requests.

## Usage:

To compile and run the client application, follow these steps:

1. Traverse the directory: `cd client`
2. Compile the code: `g++ client.cpp`
3. Run the client: `./a.out <IP>:<PORT> <TRACKER INFO FILE>`
4. Ex: `./a.out 127.0.0.1:18000 tracker_info.txt`


## Instructions

### Tracker

1. Run Tracker:

```
cd tracker
./tracker​ <TRACKER INFO FILE> <TRACKER NUMBER>
ex: ./tracker tracker_info.txt 1
```

`<TRACKER INFO FILE>` contains the IP, Port details of all the trackers.

```
Ex:
127.0.0.1
5000
127.0.0.1
6000
```

2. Close Tracker:

```
quit
```

### Client:

1. Run Client:

```
cd client
./client​ <IP>:<PORT> <TRACKER INFO FILE>
ex: ./client 127.0.0.1:18000 tracker_info.txt
```

2. Create user account:

```
create_user​ <user_id> <password>
```

3. Login:

```
login​ <user_id> <password>
```

4. Create Group:

```
create_group​ <group_id>
```

5. Join Group:

```
join_group​ <group_id>
```

6. Leave Group:

```
leave_group​ <group_id>
```

7. List pending requests:

```
list_requests ​<group_id>
```

8. Accept Group Joining Request:

```
accept_request​ <group_id> <user_id>
```

9. List All Group In Network:

```
list_groups
```

10. Logout:​

```
logout
```

## Notes:
- The server code is work in progress. The full implementation of code is yet to be completed. For now I have managed to establish connection between client and server.
- The code also maintains logs of server and client activity seperately.
- All the neceesary errors are also handled in the code. 

