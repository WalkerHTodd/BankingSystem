#include <crow.h>
#include <unordered_map>
#include <mutex>

using namespace std;

struct Account {
    string name;
    double balance;
};

unordered_map<int, Account> accounts; // In-memory storage
mutex accountMutex; // Prevent race conditions

crow::json::wvalue createAccount(const crow::request& req) {
    auto body = crow::json::load(req.body);
    if (!body || !body["id"] || !body["name"] || !body["initial_deposit"]) {
        return crow::json::wvalue{{"error", "Invalid request"}};
    }

    int id = body["id"].i();
    string name = body["name"].s();
    double deposit = body["initial_deposit"].d();

    lock_guard<mutex> lock(accountMutex);
    if (accounts.find(id) != accounts.end()) {
        return crow::json::wvalue{{"error", "Account ID already exists"}};
    }

    accounts[id] = {name, deposit};
    return crow::json::wvalue{{"status", "Account created successfully"}};
}

crow::json::wvalue checkBalance(const crow::request& req) {
    auto query_params = crow::query_string(req.url_params);
    if (!query_params.get("id")) {
        return crow::json::wvalue{{"error", "Invalid request"}};
    }

    int id = stoi(query_params.get("id"));

    lock_guard<mutex> lock(accountMutex);
    if (accounts.find(id) == accounts.end()) {
        return crow::json::wvalue{{"error", "Account not found"}};
    }

    return crow::json::wvalue{
        {"name", accounts[id].name},
        {"balance", accounts[id].balance}
    };
}

int main() {
    crow::SimpleApp app;

    CROW_ROUTE(app, "/create").methods(crow::HTTPMethod::Post)(createAccount);
    CROW_ROUTE(app, "/balance").methods(crow::HTTPMethod::Get)(checkBalance);

    app.port(18080).multithreaded().run();
    return 0;
}
