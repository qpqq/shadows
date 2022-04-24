#include "webSocketController.h"
#include <chrono>
#include <thread>
//include route
void webSocketController::handleNewMessage(const WebSocketConnectionPtr& wsConnPtr, std::string &&message, const WebSocketMessageType &type)
{
	Json::Reader reader;
	Json::Value recRoot;
	reader.parse(message, recRoot, false);
	if(!recRoot.isNull()){//Во время завершения сервера посылается пустое сообщение, чтобы не вылетал сервер
		float fromLocation[2] = {std::stof(recRoot["fromLocation"][0].asString()), std::stof(recRoot["fromLocation"][1].asString())}; //from location coords JSON -> cpp array convert
		float toLocation[2] = {std::stof(recRoot["toLocation"][0].asString()), std::stof(recRoot["toLocation"][1].asString())}; //to location coords JSON -> cpp array convert
		//Route route = Route.makeRoute(fromLocation[0], fromLocation[1], toLocation[0], toLocation[1]);

		float routeCoords[2][2] = { {std::stof(recRoot["fromLocation"][0].asString()), std::stof(recRoot["fromLocation"][1].asString())} , {std::stof(recRoot["toLocation"][0].asString()), std::stof(recRoot["toLocation"][1].asString())}};

		Json::Value sendRoot; //ПРИМЕР ОТПРАВКИ
		for(int i = 0; i < 2; i++){//2 -> route.coords.size()
			Json::Value routeCoordN;
			routeCoordN[0] = routeCoords[i][0]; //routeCoords[i][0] -> route.coords[i].lat
			routeCoordN[1] = routeCoords[i][1]; //routeCoords[i][1] -> route.coords[i].lon
			sendRoot["routeCoords"][i] = routeCoordN;
		}
		Json::StreamWriterBuilder builder;
		std::string resRouteCoords = Json::writeString(builder, sendRoot);

		wsConnPtr->send(resRouteCoords);
	}else{
		wsConnPtr->send(message);
	}

}

void webSocketController::handleNewConnection(const HttpRequestPtr &req, const WebSocketConnectionPtr& wsConnPtr)
{

}

void webSocketController::handleConnectionClosed(const WebSocketConnectionPtr& wsConnPtr)
{
    
}
