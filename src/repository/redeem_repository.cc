#include "redeem_repository.h"

namespace furbbs::repository {

void RedeemRepository::InsertCard(const std::string& code, int type, int value,
                                   const std::string& item_id, const std::string& item_name,
                                   int max_uses, int64_t expiry_date, const std::string& creator_id,
                                   const std::string& batch_no) {
    Execute([&](pqxx::work& txn) {
        txn.exec_params(sql::REDEEM_CARD_INSERT, code, type, value,
            item_id.empty() ? nullptr : item_id,
            item_name.empty() ? nullptr : item_name,
            max_uses, expiry_date > 0 ? expiry_date : nullptr,
            creator_id, batch_no.empty() ? nullptr : batch_no);
    });
}

std::optional<RedeemCardEntity> RedeemRepository::GetCardByCode(const std::string& code) {
    return Execute<std::optional<RedeemCardEntity>>([&](pqxx::work& txn) {
        auto result = txn.exec_params(sql::REDEEM_CARD_GET_BY_CODE, code);
        if (result.empty()) return std::nullopt;

        RedeemCardEntity card;
        card.id = result[0][0].as<int>();
        card.type = result[0][1].as<int>();
        card.value = result[0][2].as<int>();
        card.item_name = result[0][3].is_null() ? "" : result[0][3].as<std::string>();
        card.status = result[0][4].as<int>();
        card.max_uses = result[0][5].as<int>();
        card.used_count = result[0][6].as<int>();
        card.expiry_date = result[0][7].is_null() ? 0 : result[0][7].as<int64_t>();
        return card;
    });
}

void RedeemRepository::UpdateCardStatus(int card_id, int used_count, int status,
                                         const std::string& used_by_id, int64_t used_at) {
    Execute([&](pqxx::work& txn) {
        txn.exec_params(sql::REDEEM_CARD_UPDATE, used_count, status, used_by_id, used_at, card_id);
    });
}

std::vector<RedeemCardEntity> RedeemRepository::GetCardList(int status, int type,
                                                             const std::string& batch_no,
                                                             int limit, int offset) {
    return Execute<std::vector<RedeemCardEntity>>([&](pqxx::work& txn) {
        pqxx::result result;
        
        bool has_status = status >= 0;
        bool has_type = type >= 0;
        bool has_batch = !batch_no.empty();

        if (!has_status && !has_type && !has_batch) {
            result = txn.exec_params(sql::REDEEM_CARD_LIST_ALL, limit, offset);
        } else if (has_status && !has_type && !has_batch) {
            result = txn.exec_params(sql::REDEEM_CARD_LIST_BY_STATUS, status, limit, offset);
        } else if (!has_status && has_type && !has_batch) {
            result = txn.exec_params(sql::REDEEM_CARD_LIST_BY_TYPE, type, limit, offset);
        } else if (!has_status && !has_type && has_batch) {
            result = txn.exec_params(sql::REDEEM_CARD_LIST_BY_BATCH, batch_no, limit, offset);
        } else if (has_status && has_type && !has_batch) {
            result = txn.exec_params(sql::REDEEM_CARD_LIST_BY_STATUS_TYPE, status, type, limit, offset);
        } else if (has_status && !has_type && has_batch) {
            result = txn.exec_params(sql::REDEEM_CARD_LIST_BY_STATUS_BATCH, status, batch_no, limit, offset);
        } else if (!has_status && has_type && has_batch) {
            result = txn.exec_params(sql::REDEEM_CARD_LIST_BY_TYPE_BATCH, type, batch_no, limit, offset);
        } else {
            result = txn.exec_params(sql::REDEEM_CARD_LIST_FULL, status, type, batch_no, limit, offset);
        }

        std::vector<RedeemCardEntity> cards;
        for (const auto& row : result) {
            RedeemCardEntity card;
            card.id = row[0].as<int64_t>();
            card.code = row[1].as<std::string>();
            card.type = row[2].as<int>();
            card.value = row[3].as<int32_t>();
            if (!row[4].is_null()) card.item_id = row[4].as<std::string>();
            if (!row[5].is_null()) card.item_name = row[5].as<std::string>();
            card.status = row[6].as<int>();
            card.max_uses = row[7].as<int32_t>();
            card.used_count = row[8].as<int32_t>();
            if (!row[9].is_null()) card.expiry_date = row[9].as<int64_t>();
            if (!row[10].is_null()) card.creator_id = row[10].as<std::string>();
            if (!row[11].is_null()) card.used_by_id = row[11].as<std::string>();
            card.created_at = row[12].as<int64_t>();
            if (!row[13].is_null()) card.used_at = row[13].as<int64_t>();
            if (!row[14].is_null()) card.batch_no = row[14].as<std::string>();
            cards.push_back(card);
        }
        return cards;
    });
}

int RedeemRepository::GetTotalCards() {
    return Execute<int>([&](pqxx::work& txn) {
        auto result = txn.exec(sql::REDEEM_CARD_COUNT);
        return result[0][0].as<int>();
    });
}

} // namespace furbbs::repository
