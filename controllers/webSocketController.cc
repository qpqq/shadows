#include <vector>
#include <string>
#include <iomanip>

#include "webSocketController.h"
#include "graph.hpp"

void webSocketController::handleNewMessage(const WebSocketConnectionPtr &wsConnPtr, std::string &&message,
                                           const WebSocketMessageType &type) {
    Json::Reader reader;
    Json::Value recRoot;
    reader.parse(message, recRoot, false);
    if (!recRoot.isNull()) { //Во время завершения сервера посылается пустое сообщение, чтобы не вылетал сервер

        auto start = std::chrono::high_resolution_clock::now();

        std::vector<std::string> fromLocation = {recRoot["fromLocation"][0].asString(),
                                                 recRoot["fromLocation"][1].asString()}; //from location coords JSON -> cpp array convert
        std::vector<std::string> toLocation = {recRoot["toLocation"][0].asString(),
                                               recRoot["toLocation"][1].asString()}; //to location coords JSON -> cpp array convert

        fromLocation = {"55.576435085570381", "37.678452148437486"};
        toLocation = {"55.90491853179855", "37.547989501953126"};

        std::cout << "[" << fromLocation[0] << ", " << fromLocation[1] << "] --> "
                  << "[" << toLocation[0] << ", " << toLocation[1] << "]" << std::endl;

        DataBase db("../controllers/shadow.db");
        Graph city(db);

        auto route = city.getRoute(fromLocation, toLocation);

        Json::Value sendRoot;

        if (!route.nodes.empty()) {
            for (int i = 0; i < route.nodes.size(); i++) {
                Json::Value routeCoordN, routeShadingN;

                if (i < route.nodes.size() - 1) {
                    routeShadingN = DataBase::toStringWithPrecision(route.shading[i]);
                    sendRoot["routeShading"][(int) route.nodes.size() - 2 - i] = routeShadingN;
                }


                routeCoordN[0] = DataBase::toStringWithPrecision(route.nodes[i].x);
                routeCoordN[1] = DataBase::toStringWithPrecision(route.nodes[i].y);
                sendRoot["routeCoords"][(int) route.nodes.size() - 1 - i] = routeCoordN;
            } // TODO почему в обратном порядке + когда точки совпадают
        } else
            sendRoot["routeCoords"] = 727;

        Json::StreamWriterBuilder builder;
        std::string resRouteCoords = Json::writeString(builder, sendRoot);

        wsConnPtr->send(resRouteCoords);

        auto stop = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start);
        std::cout << duration.count() / (1000 * 1000 * 1000) << "."
                  << std::setfill('0') << std::setw(3) << duration.count() / (1000 * 1000) % 1000 << " "
                  << std::setfill('0') << std::setw(3) << duration.count() / 1000 % 1000 << " "
                  << std::setfill('0') << std::setw(3) << duration.count() % 1000 << "s" << std::endl;

        std::cout << std::endl;
    }
}

void webSocketController::handleNewConnection(const HttpRequestPtr &req, const WebSocketConnectionPtr &wsConnPtr) {

}

void webSocketController::handleConnectionClosed(const WebSocketConnectionPtr &wsConnPtr) {

}
