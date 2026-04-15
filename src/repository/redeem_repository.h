#ifndef FURBBS_REPOSITORY_REDEEM_REPOSITORY_H
#define FURBBS_REPOSITORY_REDEEM_REPOSITORY_H

#include "base_repository.h"
#include "db/sql_queries.h"
#include <vector>
#include <optional>

namespace furbbs::repository {

struct RedeemCardEntity {
    int64_t id = 0;
    std::string code;
    int type = 0;
    int value = 0;
    std::string item_id;
    std::string item_name;
    int status = 0;
    int max_uses = 1;
    int used_count = 0;
    int64_t expiry_date = 0;
    std::string creator_id;
    std::string used_by_id;
    int64_t created_at = 0;
    int64_t used_at = 0;
    std::string batch_no;
};

class RedeemRepository : protected BaseRepository {
public:
    static RedeemRepository& Instance() {
        static RedeemRepository instance;
        return instance;
    }

    void InsertCard(const std::string& code, int type, int value,
                    const std::string& item_id, const std::string& item_name,
                    int max_uses, int64_t expiry_date, const std::string& creator_id,
                    const std::string& batch_no);

    std::optional<RedeemCardEntity> GetCardByCode(const std::string& code);

    void UpdateCardStatus(int card_id, int used_count, int status,
                          const std::string& used_by_id, int64_t used_at);

    std::vector<RedeemCardEntity> GetCardList(int status, int type,
                                              const std::string& batch_no,
                                              int limit, int offset);

    int GetTotalCards();

private:
    RedeemRepository() = default;
};

} // namespace furbbs::repository

#endif // FURBBS_REPOSITORY_REDEEM_REPOSITORY_H
