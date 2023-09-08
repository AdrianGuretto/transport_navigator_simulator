#pragma once

#include "transport_catalogue.h"
#include "json.h"
#include "json_reader.h"

// A class for reading and processing user input.
class RequestHandler{
public:
    RequestHandler(Transportation::TransportCatalogue& db);

public: // --------- METHODS ---------
    void ReadInput(std::istream& in);
    void ProcessInput(std::ostream& out);

private:
    JSON_TC_Builder data_builder_;
};