#include <stdlib.h>
#include <connectionHandler.h>
#include <input.h>
#include <output.h>

/**
* This code assumes that the server replies the exact text the client sent it (as opposed to the practical session example)
*/
int main (int argc, char *argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " host port" << std::endl << std::endl;
        return -1;
    }
    std::string host = argv[1];
    short port = atoi(argv[2]);
    
    ConnectionHandler connectionHandler(host, port);
    if (!connectionHandler.connect()) {
        std::cerr << "Cannot connect to " << host << ":" << port << std::endl;
        return 1;
    }
	Input inp(1,connectionHandler);
    output opt(2,connectionHandler);
    std::thread th1(&Input::run, &inp);
    std::thread th2(&output::run, &opt);
	//From here we will see the rest of the ehco client implementation:
    th1.join();
    th2.join();
    return 0;
}
