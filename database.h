#include <string_view>
#include <unordered_map>
#include <vector>
#include <unordered_set>
#include <set>

#include "json.h"
#include "router.h"
#include "geometry.h"


class DataBase {
private:
    enum class QueryType {
        STOP, BUS, ROUTE
    };

    std::unordered_map<std::string, QueryType> STR_TO_QType =
    {
        {"Stop", QueryType::STOP},
        {"Bus", QueryType::BUS},
        {"Route", QueryType::ROUTE}
    };

    struct hash_pair {
        template <class T1, class T2>
        size_t operator()(const std::pair<T1, T2>& p) const {
            auto hash1 = std::hash<T1>{}(p.first);
            auto hash2 = std::hash<T2>{}(p.second);
            return hash1 ^ hash2;
        }
    };

private:
    std::unordered_map<std::string, std::vector<std::string>> bus_path_{};

    std::unordered_map<std::string, bool> is_roundtrip_;

    std::unordered_map<std::string, Geometry::Coordinates> stops_{};

    std::unordered_map<std::string, std::set<std::string>> stop_buses_{};

    std::unordered_map<std::pair<std::string, std::string>, int, hash_pair> distances_;


    size_t bus_wait_time_;
    double bus_velocity_;
    Graph::DirectedWeightedGraph<double> graph;

    //first - ожидание, second - начало движения, ребро first->second имеет вес bus_wait_time
    std::unordered_map<std::string, std::pair<Graph::VertexId, Graph::VertexId>> stop_to_id_;
    std::unordered_map<Graph::VertexId, std::string> id_to_stop_;

public:
    DataBase() = default;
    DataBase(const Json::Node& base_requests, const Json::Dict& routing_settings);

    std::vector<Json::Node> Response(const std::vector<Json::Node>& requests);
};
