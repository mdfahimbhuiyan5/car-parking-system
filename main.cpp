// Car Parking System with REST API Integration
// Main Program File

#include <iostream>
#include <string>
#include <map>
#include <ctime>
#include "crow.h"

class Vehicle {
    std::string licensePlate;
    std::string type; // "bike", "car", "truck"
    time_t entryTime;

public:
    Vehicle(const std::string &plate, const std::string &type) : licensePlate(plate), type(type) {
        entryTime = time(nullptr);
    }

    double calculateCost() {
        double rate = (type == "bike") ? 1.0 : (type == "car") ? 2.0 : 3.0;
        time_t now = time(nullptr);
        double hours = difftime(now, entryTime) / 3600.0;
        return hours * rate;
    }

    std::string getLicensePlate() const { return licensePlate; }
    time_t getEntryTime() const { return entryTime; }
    std::string getType() const { return type; }
};

class ParkingLot {
    std::map<std::string, Vehicle> vehicles; // Key: licensePlate

public:
    bool parkVehicle(const Vehicle &vehicle) {
        if (vehicles.find(vehicle.getLicensePlate()) != vehicles.end()) {
            return false; // Vehicle already parked
        }
        vehicles[vehicle.getLicensePlate()] = vehicle;
        return true;
    }

    bool removeVehicle(const std::string &licensePlate, double &cost) {
        auto it = vehicles.find(licensePlate);
        if (it == vehicles.end()) {
            return false; // Vehicle not found
        }
        cost = it->second.calculateCost();
        vehicles.erase(it);
        return true;
    }

    std::map<std::string, Vehicle> getVehicles() const {
        return vehicles;
    }
};

int main() {
    ParkingLot parkingLot;
    crow::SimpleApp app;

    CROW_ROUTE(app, "/park").methods(crow::HTTPMethod::POST)([&parkingLot](const crow::request &req) {
        auto json = crow::json::load(req.body);
        if (!json["licensePlate"].s() || !json["type"].s()) {
            return crow::response(400, "Invalid input");
        }

        std::string licensePlate = json["licensePlate"].s();
        std::string type = json["type"].s();
        Vehicle vehicle(licensePlate, type);

        if (parkingLot.parkVehicle(vehicle)) {
            return crow::response(200, "Vehicle parked successfully");
        } else {
            return crow::response(400, "Vehicle already parked");
        }
    });

    CROW_ROUTE(app, "/remove").methods(crow::HTTPMethod::POST)([&parkingLot](const crow::request &req) {
        auto json = crow::json::load(req.body);
        if (!json["licensePlate"].s()) {
            return crow::response(400, "Invalid input");
        }

        std::string licensePlate = json["licensePlate"].s();
        double cost;

        if (parkingLot.removeVehicle(licensePlate, cost)) {
            crow::json::wvalue response;
            response["message"] = "Vehicle removed successfully";
            response["cost"] = cost;
            return crow::response(200, response);
        } else {
            return crow::response(404, "Vehicle not found");
        }
    });

    CROW_ROUTE(app, "/status").methods(crow::HTTPMethod::GET)([&parkingLot]() {
        auto vehicles = parkingLot.getVehicles();
        crow::json::wvalue response;

        for (const auto &pair : vehicles) {
            crow::json::wvalue vehicle;
            vehicle["licensePlate"] = pair.second.getLicensePlate();
            vehicle["type"] = pair.second.getType();
            vehicle["entryTime"] = pair.second.getEntryTime();
            response[pair.first] = vehicle;
        }

        return crow::response(200, response);
    });

    app.port(8080).multithreaded().run();

    return 0;
}
