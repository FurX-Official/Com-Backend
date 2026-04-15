#ifndef FURBBS_DB_DATABASE_H
#define FURBBS_DB_DATABASE_H

#include <pqxx/pqxx>
#include <memory>
#include <mutex>
#include <queue>
#include <functional>
#include <string>

namespace furbbs::db {

class Database {
public:
    static Database& Instance();

    bool Init(const std::string& host, uint16_t port, 
              const std::string& dbname, const std::string& user, 
              const std::string& password, uint32_t max_connections);

    std::shared_ptr<pqxx::connection> GetConnection();
    void ReleaseConnection(std::shared_ptr<pqxx::connection> conn);

    template<typename Func>
    auto Execute(Func&& func) -> decltype(func(std::declval<pqxx::work&>())) {
        auto conn = GetConnection();
        pqxx::work txn(*conn);
        try {
            auto result = func(txn);
            txn.commit();
            ReleaseConnection(conn);
            return result;
        } catch (...) {
            ReleaseConnection(conn);
            throw;
        }
    }

    bool InitTables();

private:
    Database() = default;
    ~Database() = default;

    std::string connection_string_;
    std::queue<std::shared_ptr<pqxx::connection>> connections_;
    std::mutex mutex_;
    uint32_t max_connections_;
    uint32_t current_connections_ = 0;
};

} // namespace furbbs::db

#endif // FURBBS_DB_DATABASE_H
