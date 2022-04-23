#pragma once

#include <drogon/WebSocketController.h>

using namespace drogon;

class webSocketController : public drogon::WebSocketController<webSocketController> {
public:

    WebSocketConnectionPtr wsConnPtr;

    virtual void handleNewMessage(const WebSocketConnectionPtr &,
                                  std::string &&,
                                  const WebSocketMessageType &) override;

    virtual void handleNewConnection(const HttpRequestPtr &,
                                     const WebSocketConnectionPtr &) override;

    virtual void handleConnectionClosed(const WebSocketConnectionPtr &) override;

    virtual void sendRoute(const WebSocketConnectionPtr &, std::string resRouteCoords);

    WS_PATH_LIST_BEGIN
        WS_PATH_ADD("/echo");
    WS_PATH_LIST_END
};
