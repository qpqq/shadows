#include <drogon/drogon.h>
#include "controllers/database.hpp"

int main() {
    DataBase db("../controllers/shadow.db");
    db.buildings_receive_test();

    //Set HTTP listener address and port
//    drogon::app().addListener("0.0.0.0", 80);
    //Load config file
    //drogon::app().loadConfigFile("../config.json");
    //Run HTTP framework,the method will block in the internal event loop
//    drogon::app().run();
}
