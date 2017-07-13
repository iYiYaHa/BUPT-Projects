#include "DNSRelay.h"

int main(int argc, char * argv[]) {
    dnsNS::dnsRelayer * relayer = nullptr;
    
    try {  
        relayer = dnsNS::getRelayer(argc, argv);
        relayer->relay();
    }
    catch (std::exception &e) {
        std::cout << e.what() << std::endl;
        return -1;
    }

    delete relayer;
    return 0;

}
