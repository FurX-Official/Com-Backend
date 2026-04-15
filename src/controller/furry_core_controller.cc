#include "base_controller.h"
#include "../service_impl/furry_core_service.h"
#include "../auth/casdoor_auth.h"
#include <furbbs.pb.h>

namespace furbbs::controller {

::trpc::Status GenerateFursonaPrompt(::trpc::ServerContextPtr context,
                                        const ::furbbs::GenerateFursonaPromptRequest* request,
                                        ::furbbs::GenerateFursonaPromptResponse* response) {
    auto auth = BaseController::VerifyToken(request->access_token());
    if (!auth.valid) {
        BaseController::SetResponse(response, 401, "Unauthorized");
        return ::trpc::kSuccStatus;
    }

    std::map<std::string, std::string> params;
    if (!request->species().empty()) params["species"] = request->species();
    if (!request->color().empty()) params["color"] = request->color();
    if (!request->style().empty()) params["style"] = request->style();
    if (!request->gender().empty()) params["gender"] = request->gender();

    std::string prompt = service::FurryCoreService::Instance().GenerateFursonaPrompt(params);
    response->set_prompt(prompt);
    BaseController::SetResponse(response, 200, "Success");
    return ::trpc::kSuccStatus;
}

::trpc::Status SavePrompt(::trpc::ServerContextPtr context,
                            const ::furbbs::SavePromptRequest* request,
                            ::furbbs::SavePromptResponse* response) {
    auto auth = BaseController::VerifyToken(request->access_token());
    if (!auth.valid) {
        BaseController::SetResponse(response, 401, "Unauthorized");
        return ::trpc::kSuccStatus;
    }

    repository::AIPromptEntity data;
    data.fursona_id = request->fursona_id();
    data.prompt_type = request->prompt_type();
    data.title = request->title();
    data.prompt = request->prompt();
    data.model = request->model();
    for (int i = 0; i < request->style_tags_size(); i++) {
        data.style_tags.push_back(request->style_tags(i));
    }
    data.is_public = request->is_public();

    int64_t id = service::FurryCoreService::Instance().SavePrompt(auth.user_id, data);
    response->set_prompt_id(id);
    BaseController::SetResponse(response, id > 0 ? 200 : 400,
                               id > 0 ? "Saved" : "Failed");
    return ::trpc::kSuccStatus;
}

::trpc::Status GetPrompts(::trpc::ServerContextPtr context,
                           const ::furbbs::GetPromptsRequest* request,
                           ::furbbs::GetPromptsResponse* response) {
    auto auth = BaseController::VerifyToken(request->access_token());

    auto list = service::FurryCoreService::Instance().GetPrompts(
        request->user_id(), auth.user_id, request->only_public(),
        request->page(), request->page_size());

    for (auto& item : list) {
        auto* p = response->add_prompts();
        p->set_id(item.id);
        p->set_title(item.title);
        p->set_prompt(item.prompt);
        p->set_prompt_type(item.prompt_type);
        p->set_is_public(item.is_public);
        p->set_created_at(item.created_at);
    }
    BaseController::SetResponse(response, 200, "Success");
    return ::trpc::kSuccStatus;
}

::trpc::Status CreateFursonaRelation(::trpc::ServerContextPtr context,
                                      const ::furbbs::CreateFursonaRelationRequest* request,
                                      ::furbbs::CreateFursonaRelationResponse* response) {
    auto auth = BaseController::VerifyToken(request->access_token());
    if (!auth.valid) {
        BaseController::SetResponse(response, 401, "Unauthorized");
        return ::trpc::kSuccStatus;
    }

    repository::FursonaRelationEntity data;
    data.fursona_a_id = request->fursona_a_id();
    data.fursona_b_id = request->fursona_b_id();
    data.relation_type = request->relation_type();
    data.anniversary = request->anniversary();

    int64_t id = service::FurryCoreService::Instance().CreateRelation(auth.user_id, data);
    response->set_relation_id(id);
    BaseController::SetResponse(response, id > 0 ? 200 : 400,
                               id > 0 ? "Created" : "Relation exists");
    return ::trpc::kSuccStatus;
}

::trpc::Status ConfirmRelation(::trpc::ServerContextPtr context,
                               const ::furbbs::ConfirmRelationRequest* request,
                               ::furbbs::ConfirmRelationResponse* response) {
    auto auth = BaseController::VerifyToken(request->access_token());
    if (!auth.valid) {
        BaseController::SetResponse(response, 401, "Unauthorized");
        return ::trpc::kSuccStatus;
    }

    bool success = service::FurryCoreService::Instance().ConfirmRelation(
        auth.user_id, request->relation_id());

    BaseController::SetResponse(response, success ? 200 : 400,
                               success ? "Confirmed" : "Failed");
    return ::trpc::kSuccStatus;
}

::trpc::Status GetFursonaRelations(::trpc::ServerContextPtr context,
                                    const ::furbbs::GetFursonaRelationsRequest* request,
                                    ::furbbs::GetFursonaRelationsResponse* response) {
    auto list = service::FurryCoreService::Instance().GetFursonaRelations(request->fursona_id());

    for (auto& r : list) {
        auto* rel = response->add_relations();
        rel->set_id(r.id);
        rel->set_fursona_a_id(r.fursona_a_id);
        rel->set_fursona_a_name(r.fursona_a_name);
        rel->set_fursona_b_id(r.fursona_b_id);
        rel->set_fursona_b_name(r.fursona_b_name);
        rel->set_relation_type(r.relation_type);
        rel->set_user_a_confirmed(r.user_a_confirmed);
        rel->set_user_b_confirmed(r.user_b_confirmed);
        rel->set_anniversary(r.anniversary);
    }
    BaseController::SetResponse(response, 200, "Success");
    return ::trpc::kSuccStatus;
}

::trpc::Status CreateWorldSetting(::trpc::ServerContextPtr context,
                                 const ::furbbs::CreateWorldSettingRequest* request,
                                 ::furbbs::CreateWorldSettingResponse* response) {
    auto auth = BaseController::VerifyToken(request->access_token());
    if (!auth.valid) {
        BaseController::SetResponse(response, 401, "Unauthorized");
        return ::trpc::kSuccStatus;
    }

    repository::WorldSettingEntity data;
    data.name = request->name();
    data.description = request->description();
    data.cover_image = request->cover_image();
    data.setting_type = request->setting_type();
    for (int i = 0; i < request->tags_size(); i++) {
        data.tags.push_back(request->tags(i));
    }
    data.is_public = request->is_public();

    int64_t id = service::FurryCoreService::Instance().CreateWorld(auth.user_id, data);
    response->set_world_id(id);
    BaseController::SetResponse(response, id > 0 ? 200 : 400,
                               id > 0 ? "Created" : "Failed");
    return ::trpc::kSuccStatus;
}

::trpc::Status GetWorlds(::trpc::ServerContextPtr context,
                        const ::furbbs::GetWorldsRequest* request,
                        ::furbbs::GetWorldsResponse* response) {
    auto auth = BaseController::VerifyToken(request->access_token());

    auto list = service::FurryCoreService::Instance().GetWorlds(
        request->user_id(), auth.user_id, request->only_public(),
        request->page(), request->page_size());

    for (auto& w : list) {
        auto* world = response->add_worlds();
        world->set_id(w.id);
        world->set_name(w.name);
        world->set_description(w.description);
        world->set_cover_image(w.cover_image);
        world->set_is_public(w.is_public);
        world->set_view_count(w.view_count);
        world->set_like_count(w.like_count);
        world->set_created_at(w.created_at);
    }
    BaseController::SetResponse(response, 200, "Success");
    return ::trpc::kSuccStatus;
}

::trpc::Status LikeWorld(::trpc::ServerContextPtr context,
                          const ::furbbs::LikeWorldRequest* request,
                          ::furbbs::LikeWorldResponse* response) {
    auto auth = BaseController::VerifyToken(request->access_token());
    if (!auth.valid) {
        BaseController::SetResponse(response, 401, "Unauthorized");
        return ::trpc::kSuccStatus;
    }

    bool success = service::FurryCoreService::Instance().LikeWorld(
        auth.user_id, request->world_id(), request->like());

    BaseController::SetResponse(response, success ? 200 : 400,
                               success ? "Updated" : "Failed");
    return ::trpc::kSuccStatus;
}

::trpc::Status PurchaseEventTicket(::trpc::ServerContextPtr context,
                                const ::furbbs::PurchaseEventTicketRequest* request,
                                ::furbbs::PurchaseEventTicketResponse* response) {
    auto auth = BaseController::VerifyToken(request->access_token());
    if (!auth.valid) {
        BaseController::SetResponse(response, 401, "Unauthorized");
        return ::trpc::kSuccStatus;
    }

    int64_t id = service::FurryCoreService::Instance().PurchaseTicket(
        auth.user_id, request->event_id(), request->ticket_type(), request->price());
    response->set_ticket_id(id);
    BaseController::SetResponse(response, id > 0 ? 200 : 400,
                               id > 0 ? "Purchased" : "Failed");
    return ::trpc::kSuccStatus;
}

::trpc::Status GetUserTickets(::trpc::ServerContextPtr context,
                                 const ::furbbs::GetUserTicketsRequest* request,
                                 ::furbbs::GetUserTicketsResponse* response) {
    auto auth = BaseController::VerifyToken(request->access_token());
    if (!auth.valid) {
        BaseController::SetResponse(response, 401, "Unauthorized");
        return ::trpc::kSuccStatus;
    }

    auto list = service::FurryCoreService::Instance().GetUserTickets(auth.user_id);

    for (auto& t : list) {
        auto* ticket = response->add_tickets();
        ticket->set_id(t.id);
        ticket->set_event_id(t.event_id);
        ticket->set_ticket_type(t.ticket_type);
        ticket->set_checked_in(t.checked_in);
        ticket->set_qr_code(t.qr_code);
        ticket->set_created_at(t.created_at);
    }
    BaseController::SetResponse(response, 200, "Success");
    return ::trpc::kSuccStatus;
}

::trpc::Status CheckInTicket(::trpc::ServerContextPtr context,
                             const ::furbbs::CheckInTicketRequest* request,
                             ::furbbs::CheckInTicketResponse* response) {
    auto auth = BaseController::VerifyToken(request->access_token());
    if (!auth.valid) {
        BaseController::SetResponse(response, 401, "Unauthorized");
        return ::trpc::kSuccStatus;
    }

    bool success = service::FurryCoreService::Instance().CheckInTicket(
        auth.user_id, request->ticket_id(), auth.is_admin);

    BaseController::SetResponse(response, success ? 200 : 403,
                               success ? "Checked in" : "No permission");
    return ::trpc::kSuccStatus;
}

::trpc::Status CreateMarketItem(::trpc::ServerContextPtr context,
                                const ::furbbs::CreateMarketItemRequest* request,
                                ::furbbs::CreateMarketItemResponse* response) {
    auto auth = BaseController::VerifyToken(request->access_token());
    if (!auth.valid) {
        BaseController::SetResponse(response, 401, "Unauthorized");
        return ::trpc::kSuccStatus;
    }

    repository::MarketItemEntity item;
    item.title = request->title();
    item.description = request->description();
    item.category = request->category();
    item.price = request->price();
    item.price_type = request->price_type();
    for (int i = 0; i < request->images_size(); i++) {
        item.images.push_back(request->images(i));
    }
    for (int i = 0; i < request->tags_size(); i++) {
        item.tags.push_back(request->tags(i));
    }

    int64_t id = service::FurryCoreService::Instance().CreateMarketItem(auth.user_id, item);
    response->set_item_id(id);
    BaseController::SetResponse(response, id > 0 ? 200 : 400,
                               id > 0 ? "Created" : "Failed");
    return ::trpc::kSuccStatus;
}

::trpc::Status GetMarketItems(::trpc::ServerContextPtr context,
                             const ::furbbs::GetMarketItemsRequest* request,
                             ::furbbs::GetMarketItemsResponse* response) {
    auto auth = BaseController::VerifyToken(request->access_token());

    auto list = service::FurryCoreService::Instance().GetMarketItems(
        request->category(), request->keyword(), request->seller_id(),
        request->page(), request->page_size());

    for (auto& m : list) {
        auto* item = response->add_items();
        item->set_id(m.id);
        item->set_title(m.title);
        item->set_category(m.category);
        item->set_price(m.price);
        item->set_price_type(m.price_type);
        item->set_status(m.status);
        item->set_view_count(m.view_count);
        item->set_favorite_count(m.favorite_count);
        item->set_created_at(m.created_at);
    }
    response->set_total(service::FurryCoreService::Instance().GetMarketItemCount(
        request->category(), request->keyword()));
    BaseController::SetResponse(response, 200, "Success");
    return ::trpc::kSuccStatus;
}

::trpc::Status GetMarketItem(::trpc::ServerContextPtr context,
                               const ::furbbs::GetMarketItemRequest* request,
                               ::furbbs::GetMarketItemResponse* response) {
    auto auth = BaseController::VerifyToken(request->access_token());

    auto item = service::FurryCoreService::Instance().GetMarketItem(
        request->item_id(), auth.user_id);

    if (item.has_value()) {
        auto* out = response->mutable_item();
        out->set_id(item->id);
        out->set_title(item->title);
        out->set_description(item->description);
        out->set_category(item->category);
        out->set_price(item->price);
        out->set_price_type(item->price_type);
        for (auto& img : item->images) {
            out->add_images(img);
        }
        out->set_status(item->status);
        out->set_view_count(item->view_count);
        out->set_is_favorited(item->is_favorited);
    }
    BaseController::SetResponse(response, 200, "Success");
    return ::trpc::kSuccStatus;
}

::trpc::Status FavoriteMarketItem(::trpc::ServerContextPtr context,
                              const ::furbbs::FavoriteMarketItemRequest* request,
                              ::furbbs::FavoriteMarketItemResponse* response) {
    auto auth = BaseController::VerifyToken(request->access_token());
    if (!auth.valid) {
        BaseController::SetResponse(response, 401, "Unauthorized");
        return ::trpc::kSuccStatus;
    }

    bool success = service::FurryCoreService::Instance().FavoriteMarketItem(
        auth.user_id, request->item_id(), request->favorite());

    BaseController::SetResponse(response, success ? 200 : 400,
                               success ? "Updated" : "Failed");
    return ::trpc::kSuccStatus;
}

::trpc::Status PurchaseMarketItem(::trpc::ServerContextPtr context,
                                const ::furbbs::PurchaseMarketItemRequest* request,
                                ::furbbs::PurchaseMarketItemResponse* response) {
    auto auth = BaseController::VerifyToken(request->access_token());
    if (!auth.valid) {
        BaseController::SetResponse(response, 401, "Unauthorized");
        return ::trpc::kSuccStatus;
    }

    int64_t id = service::FurryCoreService::Instance().CreateTransaction(
        request->item_id(), auth.user_id, request->contact_info());

    response->set_transaction_id(id);
    BaseController::SetResponse(response, id > 0 ? 200 : 400,
                               id > 0 ? "Purchased" : "Failed");
    return ::trpc::kSuccStatus;
}

::trpc::Status GetMarketTransactions(::trpc::ServerContextPtr context,
                                        const ::furbbs::GetMarketTransactionsRequest* request,
                                        ::furbbs::GetMarketTransactionsResponse* response) {
    auto auth = BaseController::VerifyToken(request->access_token());
    if (!auth.valid) {
        BaseController::SetResponse(response, 401, "Unauthorized");
        return ::trpc::kSuccStatus;
    }

    auto list = service::FurryCoreService::Instance().GetUserTransactions(auth.user_id);

    for (auto& t : list) {
        auto* trans = response->add_transactions();
        trans->set_id(t.id);
        trans->set_item_id(t.item_id);
        trans->set_item_title(t.item_title);
        trans->set_price(t.price);
        trans->set_status(t.status);
        trans->set_created_at(t.created_at);
    }
    BaseController::SetResponse(response, 200, "Success");
    return ::trpc::kSuccStatus;
}

} // namespace furbbs::controller
