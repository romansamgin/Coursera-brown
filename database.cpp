#include "database.h"

using namespace std;

DataBase::DataBase(const Json::Node& base_requests, const Json::Dict& routing_settings) {
    bus_wait_time_ = routing_settings.at("bus_wait_time").AsInt();
    bus_velocity_ = routing_settings.at("bus_velocity").AsDouble();
    for (const Json::Node& node : base_requests.AsArray()) {
        switch (STR_TO_QType.at(node.AsMap().at("type").AsString())) {
        case QueryType::STOP: {
            string stop_name = node.AsMap().at("name").AsString();
            stop_buses_[stop_name];
            double latitude = node.AsMap().at("latitude").AsDouble();
            double longitude = node.AsMap().at("longitude").AsDouble();
            stops_[stop_name] = Geometry::Coordinates(latitude, longitude);
            for (const auto& pair : node.AsMap().at("road_distances").AsMap()) {
                int distance = pair.second.AsInt();
                string stop_name_rhs = pair.first;
                distances_[{stop_name, stop_name_rhs}] = distance;
                if (distances_.count({ stop_name_rhs, stop_name }) == 0) {
                    distances_[{stop_name_rhs, stop_name}] = distance;
                }
            }
            break;
        }
        case QueryType::BUS: {
            string bus_name = node.AsMap().at("name").AsString();
            is_roundtrip_[bus_name] = true;
            vector<string> stops;
            for (const auto stop_name : node.AsMap().at("stops").AsArray()) {
                stop_buses_[stop_name.AsString()].insert(bus_name);
                stops.push_back(stop_name.AsString());
            }
            if (!node.AsMap().at("is_roundtrip").AsBool()) {
                is_roundtrip_[bus_name] = false;
                for (size_t i = stops.size() - 2; i > 0; --i) {
                    stops.push_back(stops[i]);
                }
                stops.push_back(stops.front());
            }
            bus_path_[bus_name] = move(stops);
            break;
        }
        default:
            break;
        }
    }

    graph = Graph::DirectedWeightedGraph<double>(2 * stops_.size());
    Graph::VertexId cur_id = 0;
    for (const auto& [stop_name, crds] : stops_) {
        id_to_stop_[cur_id] = stop_name;
        stop_to_id_[stop_name].first = cur_id++;
        id_to_stop_[cur_id] = stop_name;
        stop_to_id_[stop_name].second = cur_id++;

        Graph::VertexId from = stop_to_id_[stop_name].first;
        Graph::VertexId to = stop_to_id_[stop_name].second;
        double weight = static_cast<double>(bus_wait_time_);
        Graph::Edge<double> e(Graph::EdgeType::WAIT, from, to, weight);
        graph.AddEdge(e);
    }

    for (const auto& [bus_name, stops] : bus_path_) {
        string bus = bus_name;
        for (size_t i = 0; i < stops.size(); ++i) {
            Graph::VertexId from = stop_to_id_[stops[i]].second;
            double weight = 0;
            size_t span_count = 0;
            double distance = 0;
            for (size_t j = i + 1; j < stops.size(); ++j) {
                Graph::VertexId to = stop_to_id_[stops[j]].first;
                weight += ((distances_[{stops[j - 1], stops[j]}] / 1000.0) / bus_velocity_) * 60;
                span_count++;
                Graph::Edge<double> e(Graph::EdgeType::BUS, from, to, weight, bus, span_count);
                graph.AddEdge(e);
            }
        }
    }
}

vector<Json::Node> DataBase::Response(const vector<Json::Node>& requests) {
    vector<Json::Node> result_nodes;
    result_nodes.reserve(requests.size());
    Graph::Router<double> router(graph);

    for (const Json::Node& request_node : requests) {
        Json::Dict dict;
        dict["request_id"] = Json::Node(request_node.AsMap().at("id").AsInt());
        switch (STR_TO_QType.at(request_node.AsMap().at("type").AsString())) {
        case QueryType::STOP: {
            string stop_name = request_node.AsMap().at("name").AsString();
            if (stop_buses_.count(stop_name) == 0) {
                dict["error_message"] = Json::Node("not found"s);
            }
            else {
                vector<Json::Node> bus_nodes;
                bus_nodes.reserve(stop_buses_[stop_name].size());
                for (const string& bus_name : stop_buses_[stop_name]) {
                    bus_nodes.emplace_back(bus_name);
                }
                dict["buses"] = Json::Node(move(bus_nodes));
            }
            break;
        }

        case QueryType::BUS: {
            string bus_name = request_node.AsMap().at("name").AsString();
            size_t stops_count = bus_path_[bus_name].size();
            if (stops_count == 0) {
                dict["error_message"] = Json::Node("not found"s);
            }
            else {
                unordered_set<string_view>unique_stops;
                for (string_view stop : bus_path_[bus_name]) {
                    unique_stops.insert(stop);
                }
                double length_roads = 0;
                double length_georg = 0;
                for (size_t i = 1; i < bus_path_[bus_name].size(); ++i) {
                    length_georg += Distanse(stops_.at(bus_path_[bus_name][i - 1]), stops_.at(bus_path_[bus_name][i]));
                    const auto lhs = stops_.at(bus_path_[bus_name][i - 1]);
                    const auto rhs = stops_.at(bus_path_[bus_name][i]);

                    string stop1 = bus_path_[bus_name][i - 1];
                    string stop2 = bus_path_[bus_name][i];
                    if (distances_.count({ stop1, stop2 }) == 0) {
                        length_roads += distances_.at({ stop2, stop1 });
                    }
                    else {
                        length_roads += distances_.at({ stop1, stop2 });
                    }
                }
                double curvature = length_roads / length_georg;
                dict = {
                          {"stop_count", Json::Node(static_cast<int>(stops_count))},
                          {"unique_stop_count", Json::Node(static_cast<int>(unique_stops.size()))},
                          {"route_length", Json::Node(length_roads)},
                          {"curvature", Json::Node(curvature)},
                          {"request_id", Json::Node(request_node.AsMap().at("id").AsInt())}
                };
            }
            break;
        }
        case QueryType::ROUTE: {
            string from = request_node.AsMap().at("from").AsString();
            string to = request_node.AsMap().at("to").AsString();
            const auto route_found = router.BuildRoute(stop_to_id_[from].first, stop_to_id_[to].first);
            if (!route_found.has_value()) {
                dict["error_message"] = Json::Node("not found"s);
            }
            else {
                double total_time = route_found.value().weight;
                size_t route_id = route_found.value().id;
                size_t edges_count = route_found.value().edge_count;
                dict["total_time"] = Json::Node(total_time);
                vector<Json::Node> items;
                items.reserve(edges_count);
                Json::Dict item_dict;
                for (size_t idx = 0; idx < edges_count; ++idx) {
                    Graph::EdgeId edge_id = router.GetRouteEdge(route_id, idx);
                    Graph::Edge e = graph.GetEdge(edge_id);
                    string item_time = to_string(e.weight);

                    switch (e.type) {
                    case Graph::EdgeType::WAIT: {
                        item_dict = {
                          {"type", Json::Node("Wait"s)},
                          {"stop_name", Json::Node(id_to_stop_[e.from])},
                          {"time", Json::Node(e.weight)},
                        };
                        break;
                    }
                    case Graph::EdgeType::BUS: {
                        item_dict = {
                          {"type", Json::Node("Bus"s)},
                          {"bus", Json::Node(e.bus)},
                          {"span_count", Json::Node(static_cast<int>(e.span_count))},
                          {"time", Json::Node(e.weight)},
                        };
                        break;
                    }
                    default: break;
                    }
                    items.emplace_back(item_dict);
                }
                dict["items"] = move(items);
            }
            break;
        }
        default: break;
        }
        result_nodes.push_back(Json::Node(dict));
    }
    return result_nodes;
}
