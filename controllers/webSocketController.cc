#include "webSocketController.h"
#include "graph.hpp"

void webSocketController::handleNewMessage(const WebSocketConnectionPtr &wsConnPtr, std::string &&message,
                                           const WebSocketMessageType &type) {
    Json::Reader reader;
    Json::Value recRoot;
    reader.parse(message, recRoot, false);
    if (!recRoot.isNull()) { //Во время завершения сервера посылается пустое сообщение, чтобы не вылетал сервер
        double fromLocation[2] =
                {
                        std::stof(recRoot["fromLocation"][0].asString()), std::stof(
                        recRoot["fromLocation"][1].asString())}; //from location coords JSON -> cpp array convert
        double toLocation[2] =
                {
                        std::stof(recRoot["toLocation"][0].asString()), std::stof(
                        recRoot["toLocation"][1].asString())}; //to location coords JSON -> cpp array convert

        Graph city{};
        auto route = city.getRoute(1, 1);

        double routeCoords[2][2] = {
                {
                        std::stof(recRoot["fromLocation"][0].asString()),
                        std::stof(recRoot["fromLocation"][1].asString())},
                {
                        std::stof(recRoot["toLocation"][0].asString()),
                        std::stof(recRoot["toLocation"][1].asString())}};

        Json::Value sendRoot; //ПРИМЕР ОТПРАВКИ
        for (int i = 0; i < 2; i++) { //2 -> route.coords.size()
//        for (int i = 0; i < route.Nodes.size(); i++) {
            Json::Value routeCoordN;
            routeCoordN[0] = routeCoords[i][0]; //routeCoords[i][0] -> route.coords[i].lat
            routeCoordN[1] = routeCoords[i][1]; //routeCoords[i][1] -> route.coords[i].lon
//            routeCoordN[0] = route.Nodes[i].x;
//            routeCoordN[1] = route.Nodes[i].y;
            sendRoot["routeCoords"][i] = routeCoordN;
        }
        Json::StreamWriterBuilder builder;
        std::string resRouteCoords = Json::writeString(builder, sendRoot);

        wsConnPtr->send(resRouteCoords);
    }
}

void webSocketController::handleNewConnection(const HttpRequestPtr &req, const WebSocketConnectionPtr &wsConnPtr) {

}

void webSocketController::handleConnectionClosed(const WebSocketConnectionPtr &wsConnPtr) {

}
