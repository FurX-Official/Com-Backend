#ifndef FURBBS_REPOSITORY_BASE_REPOSITORY_H
#define FURBBS_REPOSITORY_BASE_REPOSITORY_H

#include <functional>
#include <pqxx/pqxx>
#include "db/database.h"

namespace furbbs::repository {

class BaseRepository {
protected:
    template<typename T>
    T Execute(std::function<T(pqxx::work&)> func) {
        return db::Database::Instance().Execute(func);
    }

    void Execute(std::function<void(pqxx::work&)> func) {
        db::Database::Instance().Execute(func);
    }
};

} // namespace furbbs::repository

#endif // FURBBS_REPOSITORY_BASE_REPOSITORY_H
