#include "database.h"

int main() {
    const Json::Document doc_input = Json::Load(std::cin);
    const Json::Dict& input_map = doc_input.GetRoot().AsMap();
    DataBase db(
        input_map.at("base_requests").AsArray(),
        input_map.at("routing_settings").AsMap()
    );
    Json::PrintValue(db.Response(input_map.at("stat_requests").AsArray()), std::cout);

    return 0;
}
