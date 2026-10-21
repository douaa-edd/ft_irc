# ft_irc — IRC Server in C++98

## Description

**ft_irc** is an IRC (Internet Relay Chat) server written in **C++98**.

The server implements the core IRC protocol and supports multiple simultaneous clients, channel management, operator privileges, messaging, and the mandatory channel modes required by the 42 subject.

The server uses a single `poll()` loop to monitor the listening socket and all connected clients. Every socket is configured in non-blocking mode to ensure that one client never blocks the entire server.

---

# Key Technical Points

| Topic | Implementation |
|-------|----------------|
| I/O Multiplexing | `poll()` with `std::vector<pollfd>` |
| Non-blocking I/O | `fcntl(fd, F_SETFL, O_NONBLOCK)` |
| TCP Handling | Per-client receive buffer with `\r\n` message reconstruction |
| IRC Parsing | RFC 2812 message parser |
| Signal Handling | `SIGINT` / `SIGTERM` for clean shutdown, `SIGPIPE` ignored |

---

# Features

- Multiple simultaneous clients
- Server password authentication (`PASS`)
- User registration (`NICK`, `USER`)
- Channel creation and management
- Private and channel messaging
- Channel operators
- Channel modes:
  - `+i` Invite Only
  - `+t` Topic Protection
  - `+k` Channel Key
  - `+l` User Limit
  - `+o` Channel Operator

### Bonus

- IRC Bot
- DCC SEND file transfer

---

# Compilation

```bash
make
```

---

# Running the Server

```bash
./ircserv <port> <password>
```

Example:

```bash
./ircserv 6667 mypassword
```

Stop the server with **Ctrl+C**.

---

# Connecting with HexChat

1. Open **HexChat**
2. Open the Network List.
3. Create a new network.
4. Set the server to:

```
127.0.0.1/6667
```

5. Set the server password.
6. Connect.

---

# Testing with Netcat

```bash
nc -C 127.0.0.1 6667
```

Example registration:

```text
PASS mypassword
NICK user
USER username 0 * :realname
JOIN #general
PRIVMSG #general :Hello World!
QUIT :Bye
```

---

# Supported Commands

## Registration

| Command | Syntax |
|---------|--------|
| PASS | `PASS <password>` |
| NICK | `NICK <nickname>` |
| USER | `USER <username> 0 * :<realname>` |

---

## Channels

| Command | Syntax |
|---------|--------|
| JOIN | `JOIN <#channel> [key]` |
| PART | `PART <#channel> [:reason]` |
| TOPIC | `TOPIC <#channel> [:topic]` |
| INVITE | `INVITE <nickname> <#channel>` |
| KICK | `KICK <#channel> <nickname> [:reason]` |
| QUIT | `QUIT [:reason]` |

---

## Messaging

| Command | Syntax |
|---------|--------|
| PRIVMSG | `PRIVMSG <nickname> :<message>` |
| PRIVMSG | `PRIVMSG <#channel> :<message>` |
| NOTICE | `NOTICE <nickname> :<message>` |
| NOTICE | `NOTICE <#channel> :<message>` |

---

## Channel Modes

| Command | Syntax |
|---------|--------|
| MODE | `MODE <#channel>` |
| MODE | `MODE <#channel> +i` |
| MODE | `MODE <#channel> -i` |
| MODE | `MODE <#channel> +t` |
| MODE | `MODE <#channel> -t` |
| MODE | `MODE <#channel> +k <key>` |
| MODE | `MODE <#channel> -k` |
| MODE | `MODE <#channel> +l <limit>` |
| MODE | `MODE <#channel> -l` |
| MODE | `MODE <#channel> +o <nickname>` |
| MODE | `MODE <#channel> -o <nickname>` |

---

## Utility

| Command | Syntax |
|---------|--------|
| PING | `PING <token>` |

---

# Bot (Bonus)

The server includes a built-in bot named **ircbot**.

Supported commands:

| Command | Description |
|---------|-------------|
| `!hello` | Greets the user |
| `!help` | Displays available commands |
| `!time` | Shows the current server time |
| `!roll` | Rolls a dice |
| `!echo <text>` | Repeats the given text |

---

# Resources

- RFC 2812 — IRC Client Protocol
- Modern IRC Client Protocol

---

