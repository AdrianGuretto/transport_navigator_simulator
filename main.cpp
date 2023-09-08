#include "src/headers/request_handler.h"

int main() {
    Transportation::TransportCatalogue transp_cat;
    RequestHandler req_handler(transp_cat);
    req_handler.ReadInput(std::cin);
    req_handler.ProcessInput(std::cout);
}
