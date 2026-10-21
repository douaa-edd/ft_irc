#include "../include/Server.hpp"


volatile sig_atomic_t g_running = 1;

static void signalHandler(int signum)
{
    (void)signum; 
    g_running = 0;
}


Server::Server() : port(-1), pass(""), listening_socket(-1) {}

Server::Server(int port, std::string pass, int listening_socket)
    : port(port), pass(pass), listening_socket(listening_socket) {}

Server::~Server()
{
    for (std::map<int, Client>::iterator it = clients.begin();
         it != clients.end(); ++it)
        close(it->first);
    if (listening_socket != -1)
        close(listening_socket);
    std::cout << "\nServer shut down cleanly." << std::endl;
}


static void setupSignals()
{
    struct sigaction sa;
    sa.sa_handler = signalHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    sigaction(SIGINT,  &sa, NULL);
    sigaction(SIGTERM, &sa, NULL); 

    struct sigaction sa_pipe;
    sa_pipe.sa_handler = SIG_IGN;
    sigemptyset(&sa_pipe.sa_mask);
    sa_pipe.sa_flags = 0;
    sigaction(SIGPIPE, &sa_pipe, NULL);
}


void Server::run()
{
    setupSignals();

    listening_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (listening_socket == -1)
    {
        std::cerr << "socket() failed" << std::endl;
        return;
    }

    int yes = 1;
    if (setsockopt(listening_socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1)
    {
        std::cerr << "setsockopt() failed" << std::endl;
        close(listening_socket);
        return;
    }

    if (fcntl(listening_socket, F_SETFL, O_NONBLOCK) == -1)
    {
        std::cerr << "fcntl() failed" << std::endl;
        close(listening_socket);
        return;
    }

    sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family      = AF_INET;
    server_addr.sin_port        = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(listening_socket, (sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        std::cerr << "bind() failed" << std::endl;
        close(listening_socket);
        return;
    }

    if (listen(listening_socket, SOMAXCONN) == -1)
    {
        std::cerr << "listen() failed" << std::endl;
        close(listening_socket);
        return;
    }

    pollfd p;
    p.fd      = listening_socket;
    p.events  = POLLIN;
    p.revents = 0;
    poll_fds.push_back(p);

    std::cout << "IRC server started on port " << port << std::endl;
    std::cout << "Bot 'ircbot' ready. " << port << std::endl;
    std::cout << "Press Ctrl+C to stop." << std::endl;

    while (g_running)
    {
        int ready = poll(&poll_fds[0], poll_fds.size(), -1);

        if (ready == -1)
        {
            if (errno == EINTR)
                break;
            std::cerr << "poll() error" << std::endl;
            continue;
        }

        for (size_t i = 0; i < poll_fds.size(); i++)
        {
            if (!(poll_fds[i].revents & POLLIN))
                continue;

            if (poll_fds[i].fd == listening_socket)
                acceptNewClient();
            else
                receiveData(i);
        }
    }
}


void Server::acceptNewClient()
{
    sockaddr_in client_addr;
    socklen_t   addrlen = sizeof(client_addr);
    int client_fd = accept(listening_socket, (sockaddr *)&client_addr, &addrlen);
    if (client_fd == -1)
    {
        std::cerr << "accept() error" << std::endl;
        return;
    }

    if (fcntl(client_fd, F_SETFL, O_NONBLOCK) == -1)
    {
        std::cerr << "fcntl() on client failed" << std::endl;
        close(client_fd);
        return;
    }

    pollfd c;
    c.fd      = client_fd;
    c.events  = POLLIN;
    c.revents = 0;
    poll_fds.push_back(c);

    clients[client_fd] = Client(client_fd);

    char hostBuf[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr.sin_addr, hostBuf, sizeof(hostBuf));
    clients[client_fd].setHostname(std::string(hostBuf));

    std::cout << "New connection from " << hostBuf
              << " (fd=" << client_fd << ")" << std::endl;
}


void Server::receiveData(size_t pollIndex)
{
    int clientFd = poll_fds[pollIndex].fd;
    char buffer[512];
    ssize_t n = recv(clientFd, buffer, sizeof(buffer) - 1, 0);

    if (n > 0)
    {
        buffer[n] = '\0';
        clients[clientFd].appendBuffer(std::string(buffer, n));


        while (true)
        {
            std::string &liveBuf = clients[clientFd].getBuffer();
            size_t newline = liveBuf.find('\n');
            if (newline == std::string::npos)
                break;

            std::string line = liveBuf.substr(0, newline);
            liveBuf.erase(0, newline + 1);

            if (!line.empty() && line[line.size() - 1] == '\r')
                line.erase(line.size() - 1);

            if (line.empty())
                continue;

            IRCMessage msg = parseMessage(line);
            handleCommand(clientFd, msg);


            if (clients.find(clientFd) == clients.end())
            {
                poll_fds.erase(poll_fds.begin() + pollIndex);
                break;
            }
        }
    }
    else if (n == 0)
    {
        std::cout << "Client fd=" << clientFd << " disconnected" << std::endl;
        removeClient(clientFd);
        poll_fds.erase(poll_fds.begin() + pollIndex);
    }
    else
    {

        if (errno != EAGAIN && errno != EWOULDBLOCK)
        {
            std::cerr << "recv() error on fd=" << clientFd << std::endl;
            removeClient(clientFd);
            poll_fds.erase(poll_fds.begin() + pollIndex);
        }
    }
}
