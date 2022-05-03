#include <vector>
#include <string>

#include "webSocketController.h"
#include "graph.hpp"

void webSocketController::handleNewMessage(const WebSocketConnectionPtr &wsConnPtr, std::string &&message,
                                           const WebSocketMessageType &type) {
    Json::Reader reader;
    Json::Value recRoot;
    reader.parse(message, recRoot, false);
    if (!recRoot.isNull()) { //Во время завершения сервера посылается пустое сообщение, чтобы не вылетал сервер
        std::vector<std::string> fromLocation = {recRoot["fromLocation"][0].asString(),
                                                 recRoot["fromLocation"][1].asString()}; //from location coords JSON -> cpp array convert
        std::vector<std::string> toLocation = {recRoot["toLocation"][0].asString(),
                                               recRoot["toLocation"][1].asString()}; //to location coords JSON -> cpp array convert

        std::cout << "[" << fromLocation[0] << ", " << fromLocation[1] << "] --> "
                  << "[" << toLocation[0] << ", " << toLocation[1] << "]" << std::endl;

        DataBase db("../controllers/shadow.db");
        Graph city(db);

        auto route = city.getRoute(fromLocation, toLocation);

//        double routeCoords[2][2] = {
//                {
//                        std::stof(recRoot["fromLocation"][0].asString()),
//                        std::stof(recRoot["fromLocation"][1].asString())},
//                {
//                        std::stof(recRoot["toLocation"][0].asString()),
//                        std::stof(recRoot["toLocation"][1].asString())}};

        Json::Value sendRoot;

        if (!route.Nodes.empty()) {
//        for (int i = 0; i < 2; i++) { //2 -> route.coords.size()
            for (int i = 0; i < route.Nodes.size(); i++) {
                Json::Value routeCoordN;
//            routeCoordN[0] = routeCoords[i][0]; //routeCoords[i][0] -> route.coords[i].lat
//            routeCoordN[1] = routeCoords[i][1]; //routeCoords[i][1] -> route.coords[i].lon
                routeCoordN[0] = DataBase::toStringWithPrecision(route.Nodes[i].x);
                routeCoordN[1] = DataBase::toStringWithPrecision(route.Nodes[i].y);
                sendRoot["routeCoords"][(int) route.Nodes.size() - 1 -
                                        i] = routeCoordN; // TODO почему в обратном порядке
            }
        } else
            sendRoot["routeCoords"] = 727;

        Json::StreamWriterBuilder builder;
        std::string resRouteCoords = Json::writeString(builder, sendRoot);

        wsConnPtr->send(resRouteCoords);
    }
}

void webSocketController::handleNewConnection(const HttpRequestPtr &req, const WebSocketConnectionPtr &wsConnPtr) {

}

void webSocketController::handleConnectionClosed(const WebSocketConnectionPtr &wsConnPtr) {

}
