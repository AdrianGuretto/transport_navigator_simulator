#include "headers/request_handler.h"

RequestHandler::RequestHandler(Transportation::TransportCatalogue& db) : data_builder_(db) {}

void RequestHandler::ReadInput(std::istream& in){
    data_builder_.ReadData(in);
}
void RequestHandler::ProcessInput(std::ostream& out){
    data_builder_.BuildData(out);
}