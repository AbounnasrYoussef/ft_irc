# ft_irc

*This project has been created as part of the 42 curriculum by nbougrin, yabounna, oelbied.*

## Description

ft_irc is an IRC (Internet Relay Chat) server implementation written in C++98. The goal of this project is to create a fully functional IRC server capable of handling multiple clients simultaneously through non-blocking I/O operations. The server supports essential IRC features including authentication, channel management, private messaging, and channel operator commands. This project demonstrates understanding of network programming, socket operations, and the IRC protocol while adhering to C++98 standards and strict compilation requirements.

## Instructions

### Compilation

The project uses a Makefile with the following rules:

```bash
make        # Compile the server
make clean  # Remove object files
make fclean # Remove object files and executable
make re     # Recompile everything
```

The server is compiled with the following flags: `-Wall -Wextra -Werror -std=c++98`

### Requirements

- C++ compiler with C++98 support
- Unix-like operating system (Linux or macOS)
- Make build system

### Execution

Start the server by providing a port number and password:

```bash
./ircserv <port> <password>
```

Example:
```bash
./ircserv 6667 mypassword
```

Arguments:
- `port`: The port number on which the server will listen (1-65535)
- `password`: Connection password required for client authentication

### Connecting to the Server

**Using netcat (for testing):**
```bash
nc -c 127.0.0.1 6667
PASS mypassword
NICK yournickname
USER username 0 * :Real Name
```

**Using LimeChat:**
1. Open LimeChat
2. Add new server: 127.0.0.1
3. Set port: 6667
4. Set password: mypassword
5. Connect

## Supported Commands

### Authentication Commands

- **PASS** - Authenticate with server password
- **NICK** - Set or change nickname
- **USER** - Set username and realname

### Channel Commands

- **JOIN** - Join a channel or create it if it does not exist
- **PART** - Leave a channel
- **TOPIC** - View or change channel topic
- **INVITE** - Invite a user to a channel

### Messaging Commands

- **PRIVMSG** - Send private message to user or channel
- **QUIT** - Disconnect from server

### Operator Commands

Channel operators have additional privileges:

- **KICK** - Remove a user from channel
- **MODE** - Change channel modes

#### Available Channel Modes

- **i** - Set/remove invite-only channel
- **t** - Set/remove restrictions on TOPIC command
- **k** - Set/remove channel key (password)
- **o** - Give/take channel operator privilege
- **l** - Set/remove user limit to channel

## Architecture

### Core Components

**Server**
- Manages client connections and channels
- Handles command processing and routing
- Implements non-blocking I/O with poll()

**Client**
- Represents connected user
- Maintains authentication state
- Buffers incomplete messages

**Channel**
- Manages channel members and operators
- Enforces channel modes
- Broadcasts messages to members

### Design Principles

- Single poll() call for all I/O operations
- Non-blocking socket operations
- No forking or threading
- Proper signal handling (SIGINT, SIGTERM, SIGQUIT)
- Partial message aggregation

## Project Structure

```
ft_irc/
├── Makefile
├── Mandatory/
│   ├── includes/
│   │   ├── Server.hpp
│   │   ├── Client.hpp
│   │   ├── Channel.hpp
│   │   └── Commands.hpp
│   ├── srcs/
│   │   ├── Server.cpp
│   │   ├── Client.cpp
│   │   └── Channel.cpp
│   ├── tools/
│   │   ├── Server_tools.cpp
│   │   ├── Client_tools.cpp
│   │   ├── privmsg.cpp
│   │   ├── kick.cpp
│   │   ├── mode.cpp
│   │   ├── Join.cpp
│   │   ├── Topic.cpp
│   │   └── Invite.cpp
│   └── main.cpp
└── Bonus/
    └── (Bot implementation)
```

## Makefile Rules

- `make` or `make all` - Compile the server
- `make clean` - Remove object files
- `make fclean` - Remove object files and executable
- `make re` - Recompile everything

## Testing

### Basic Connection Test

```bash
# Terminal 1: Start server
./ircserv 6667 password123

# Terminal 2: Connect with nc
nc -c 127.0.0.1 6667
PASS password123
NICK testuser
USER testuser 0 * :Test User
JOIN #test
PRIVMSG #test :Hello world
QUIT
```

### Multi-Client Test

Connect multiple clients to test:
- Message broadcasting in channels
- Private messaging between users
- Channel operator commands
- Partial message handling
- Client disconnection handling

## Compliance

This implementation follows:
- RFC 1459 (Internet Relay Chat Protocol)
- 42 School ft_irc subject requirements

### Mandatory Features

- Authentication with password
- Nickname and username registration
- Channel operations (join, part, topic)
- Private and channel messaging
- Channel operator privileges
- Channel modes (i, t, k, o, l)
- Operator commands (KICK, INVITE, TOPIC, MODE)

### Bonus Features

- Bot implementation

## Technical Constraints

- No external libraries except C++ standard library
- No Boost
- No forking allowed
- All I/O operations must be non-blocking
- Only one poll() (or equivalent: select, kqueue, epoll)
- fcntl() limited to: fcntl(fd, F_SETFL, O_NONBLOCK)

## Error Handling

The server handles:
- Invalid port numbers
- Empty passwords
- Partial message reception
- Client disconnections
- Invalid commands
- Permission errors
- Signal interruptions (Ctrl+C)

## Resources

### Documentation and References

- **RFC 1459: Internet Relay Chat Protocol**  
  https://www.rfc-editor.org/rfc/rfc1459.html

- **Linux poll() Documentation**  
  https://man7.org/linux/man-pages/man2/poll.2.html

- **Socket Programming in C++**  
  https://www.geeksforgeeks.org/cpp/socket-programming-in-cpp/

- **Sockets Programming Guide**  
  https://home.iitk.ac.in/~chebrolu/ee673-f06/sockets.pdf

- **Beej's Guide to Network Programming**  
  https://beej.us/guide/bgnet/

