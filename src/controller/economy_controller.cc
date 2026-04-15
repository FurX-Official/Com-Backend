#include "base_controller.h"
#include "../service_impl/economy_service.h"
#include "../auth/casdoor_auth.h"
#include <furbbs.pb.h>

namespace furbbs::controller {

::trpc::Status CreatePaidContent(::trpc::ServerContextPtr context,
                                                     const ::furbbs::CreatePaidContentRequest* request,
                                                     ::furbbs::CreatePaidContentResponse* response) {
    auto auth = BaseController::VerifyToken(request->access_token());
    if (!auth.valid) {
        BaseController::SetResponse(response, 401, "Unauthorized");
        return ::trpc::kSuccStatus;
    }

    int64_t content_id = service::EconomyService::Instance().CreatePaidContent(
        auth.user_id, request->post_id(), request->price(),
        request->preview_content(), request->full_content());

    response->set_content_id(content_id);
    BaseController::SetResponse(response, content_id > 0 ? 200 : 400,
                               content_id > 0 ? "Created" : "Failed");
    return ::trpc::kSuccStatus;
}

::trpc::Status GetUserPaidContents(::trpc::ServerContextPtr context,
                                                        const ::furbbs::GetUserPaidContentsRequest* request,
                                                        ::furbbs::GetUserPaidContentsResponse* response) {
    std::string viewer_id;
    auto auth = BaseController::VerifyToken(request->access_token());
    if (auth.valid) viewer_id = auth.user_id;

    int total = 0;
    auto contents = service::EconomyService::Instance().GetUserPaidContents(
        request->access_token(), request->user_id(),
        request->page(), request->page_size(), total);

    for (const auto& c : contents) {
        auto* pc = response->add_contents();
        pc->set_id(c.id);
        pc->set_post_id(c.post_id);
        pc->set_price(c.price);
        pc->set_preview_content(c.preview_content);
        pc->set_full_content(c.full_content);
        pc->set_purchase_count(c.purchase_count);
        pc->set_purchased(c.purchased);
    }
    response->set_total(total);
    BaseController::SetResponse(response, 200, "Success");
    return ::trpc::kSuccStatus;
}

::trpc::Status PurchaseContent(::trpc::ServerContextPtr context,
                                                   const ::furbbs::PurchaseContentRequest* request,
                                                   ::furbbs::PurchaseContentResponse* response) {
    auto auth = BaseController::VerifyToken(request->access_token());
    if (!auth.valid) {
        BaseController::SetResponse(response, 401, "Unauthorized");
        return ::trpc::kSuccStatus;
    }

    bool success = service::EconomyService::Instance().PurchaseContent(
        request->access_token(), request->content_id());

    BaseController::SetResponse(response, success ? 200 : 400,
                               success ? "Purchased" : "Insufficient points or failed");
    return ::trpc::kSuccStatus;
}

::trpc::Status CreateGallery(::trpc::ServerContextPtr context,
                                                  const ::furbbs::CreateGalleryRequest* request,
                                                  ::furbbs::CreateGalleryResponse* response) {
    auto auth = BaseController::VerifyToken(request->access_token());
    if (!auth.valid) {
        BaseController::SetResponse(response, 401, "Unauthorized");
        return ::trpc::kSuccStatus;
    }

    int64_t gallery_id = service::EconomyService::Instance().CreateGallery(
        auth.user_id, request->name(), request->description(),
        request->cover_image(), request->is_public(), request->is_nsfw());

    response->set_gallery_id(gallery_id);
    BaseController::SetResponse(response, gallery_id > 0 ? 200 : 400,
                               gallery_id > 0 ? "Created" : "Failed");
    return ::trpc::kSuccStatus;
}

::trpc::Status GetGalleries(::trpc::ServerContextPtr context,
                                                const ::furbbs::GetGalleriesRequest* request,
                                                ::furbbs::GetGalleriesResponse* response) {
    std::string viewer_id;
    auto auth = BaseController::VerifyToken(request->access_token());
    if (auth.valid) viewer_id = auth.user_id;

    int total = 0;
    auto galleries = service::EconomyService::Instance().GetGalleries(
        request->access_token(), request->user_id(), request->only_public(),
        request->page(), request->page_size(), total);

    for (const auto& g : galleries) {
        auto* gallery = response->add_galleries();
        gallery->set_id(g.id);
        gallery->set_name(g.name);
        gallery->set_item_count(g.item_count);
        gallery->set_is_favorited(g.is_favorited);
        gallery->set_username(g.username);
    }
    response->set_total(total);
    BaseController::SetResponse(response, 200, "Success");
    return ::trpc::kSuccStatus;
}

::trpc::Status AddGalleryItem(::trpc::ServerContextPtr context,
                                                  const ::furbbs::AddGalleryItemRequest* request,
                                                  ::furbbs::AddGalleryItemResponse* response) {
    auto auth = BaseController::VerifyToken(request->access_token());
    if (!auth.valid) {
        BaseController::SetResponse(response, 401, "Unauthorized");
        return ::trpc::kSuccStatus;
    }

    furbbs::repository::GalleryItemEntity item;
    item.title = request->title();
    item.description = request->description();
    item.image_url = request->image_url();
    item.thumbnail_url = request->thumbnail_url();
    item.fursona_id = request->fursona_id();
    item.artist_name = request->artist_name();
    item.is_nsfw = request->is_nsfw();

    int64_t item_id = service::EconomyService::Instance().AddGalleryItem(
        auth.user_id, request->gallery_id(), item);

    response->set_item_id(item_id);
    BaseController::SetResponse(response, item_id > 0 ? 200 : 403,
                               item_id > 0 ? "Added" : "Permission denied");
    return ::trpc::kSuccStatus;
}

::trpc::Status GetGalleryItems(::trpc::ServerContextPtr context,
                                                   const ::furbbs::GetGalleryItemsRequest* request,
                                                   ::furbbs::GetGalleryItemsResponse* response) {
    std::string viewer_id;
    auto auth = BaseController::VerifyToken(request->access_token());
    if (auth.valid) viewer_id = auth.user_id;

    int total = 0;
    auto items = service::EconomyService::Instance().GetGalleryItems(
        request->access_token(), request->gallery_id(),
        request->page(), request->page_size(), total);

    for (const auto& i : items) {
        auto* item = response->add_items();
        item->set_id(i.id);
        item->set_title(i.title);
        item->set_image_url(i.image_url);
    }
    response->set_total(total);
    BaseController::SetResponse(response, 200, "Success");
    return ::trpc::kSuccStatus;
}

::trpc::Status FavoriteGallery(::trpc::ServerContextPtr context,
                                                     const ::furbbs::FavoriteGalleryRequest* request,
                                                     ::furbbs::FavoriteGalleryResponse* response) {
    auto auth = BaseController::VerifyToken(request->access_token());
    if (!auth.valid) {
        BaseController::SetResponse(response, 401, "Unauthorized");
        return ::trpc::kSuccStatus;
    }

    bool success = service::EconomyService::Instance().SetGalleryFavorite(
        auth.user_id, request->gallery_id(), request->favorite());

    BaseController::SetResponse(response, success ? 200 : 400,
                               success ? "Updated" : "Failed");
    return ::trpc::kSuccStatus;
}

::trpc::Status TransferPoints(::trpc::ServerContextPtr context,
                                                    const ::furbbs::TransferPointsRequest* request,
                                                    ::furbbs::TransferPointsResponse* response) {
    auto auth = BaseController::VerifyToken(request->access_token());
    if (!auth.valid) {
        BaseController::SetResponse(response, 401, "Unauthorized");
        return ::trpc::kSuccStatus;
    }

    bool success = service::EconomyService::Instance().TransferPoints(
        auth.user_id, request->to_user_id(), request->amount(), request->message());

    BaseController::SetResponse(response, success ? 200 : 400,
                               success ? "Transferred" : "Insufficient points or failed");
    return ::trpc::kSuccStatus;
}

::trpc::Status GetTransferHistory(::trpc::ServerContextPtr context,
                                                      const ::furbbs::GetTransferHistoryRequest* request,
                                                      ::furbbs::GetTransferHistoryResponse* response) {
    auto auth = BaseController::VerifyToken(request->access_token());
    if (!auth.valid) {
        BaseController::SetResponse(response, 401, "Unauthorized");
        return ::trpc::kSuccStatus;
    }

    auto transfers = service::EconomyService::Instance().GetTransferHistory(
        auth.user_id, request->page(), request->page_size());

    for (const auto& t : transfers) {
        auto* tr = response->add_transfers();
        tr->set_id(t.id);
        tr->set_from_username(t.from_username);
        tr->set_to_username(t.to_username);
        tr->set_amount(t.amount);
    }
    BaseController::SetResponse(response, 200, "Success");
    return ::trpc::kSuccStatus;
}

::trpc::Status CreateRedEnvelope(::trpc::ServerContextPtr context,
                                                      const ::furbbs::CreateRedEnvelopeRequest* request,
                                                      ::furbbs::CreateRedEnvelopeResponse* response) {
    auto auth = BaseController::VerifyToken(request->access_token());
    if (!auth.valid) {
        BaseController::SetResponse(response, 401, "Unauthorized");
        return ::trpc::kSuccStatus;
    }

    int64_t envelope_id = service::EconomyService::Instance().CreateRedEnvelope(
        auth.user_id, request->total_amount(), request->count(),
        request->message(), request->is_random());

    response->set_envelope_id(envelope_id);
    BaseController::SetResponse(response, envelope_id > 0 ? 200 : 400,
                               envelope_id > 0 ? "Created" : "Insufficient points or failed");
    return ::trpc::kSuccStatus;
}

::trpc::Status ClaimRedEnvelope(::trpc::ServerContextPtr context,
                                                     const ::furbbs::ClaimRedEnvelopeRequest* request,
                                                     ::furbbs::ClaimRedEnvelopeResponse* response) {
    auto auth = BaseController::VerifyToken(request->access_token());
    if (!auth.valid) {
        BaseController::SetResponse(response, 401, "Unauthorized");
        return ::trpc::kSuccStatus;
    }

    int32_t amount = service::EconomyService::Instance().ClaimRedEnvelope(
        auth.user_id, request->envelope_id());

    response->set_amount(amount);
    BaseController::SetResponse(response, amount > 0 ? 200 : 400,
                               amount > 0 ? "Claimed" : "Expired or already claimed");
    return ::trpc::kSuccStatus;
}

::trpc::Status GetRedEnvelopes(::trpc::ServerContextPtr context,
                                                   const ::furbbs::GetRedEnvelopesRequest* request,
                                                   ::furbbs::GetRedEnvelopesResponse* response) {
    auto envelopes = service::EconomyService::Instance().GetActiveRedEnvelopes(
        request->page(), request->page_size());

    for (const auto& e : envelopes) {
        auto* env = response->add_envelopes();
        env->set_id(e.id);
        env->set_sender_name(e.sender_name);
        env->set_total_amount(e.total_amount);
    }
    BaseController::SetResponse(response, 200, "Success");
    return ::trpc::kSuccStatus;
}

::trpc::Status RewardPost(::trpc::ServerContextPtr context,
                                              const ::furbbs::RewardPostRequest* request,
                                              ::furbbs::RewardPostResponse* response) {
    auto auth = BaseController::VerifyToken(request->access_token());
    if (!auth.valid) {
        BaseController::SetResponse(response, 401, "Unauthorized");
        return ::trpc::kSuccStatus;
    }

    bool success = service::EconomyService::Instance().RewardPost(
        auth.user_id, request->post_id(), request->amount(),
        request->message(), request->anonymous());

    BaseController::SetResponse(response, success ? 200 : 400,
                               success ? "Rewarded" : "Insufficient points or failed");
    return ::trpc::kSuccStatus;
}

::trpc::Status GetPostRewards(::trpc::ServerContextPtr context,
                                                   const ::furbbs::GetPostRewardsRequest* request,
                                                   ::furbbs::GetPostRewardsResponse* response) {
    auto rewards = service::EconomyService::Instance().GetPostRewards(
        request->post_id(), request->page(), request->page_size());

    for (const auto& r : rewards) {
        auto* reward = response->add_rewards();
        reward->set_id(r.id);
        reward->set_amount(r.amount);
        reward->set_sender_name(r.sender_name);
    }
    BaseController::SetResponse(response, 200, "Success");
    return ::trpc::kSuccStatus;
}

::trpc::Status CreateCollection(::trpc::ServerContextPtr context,
                                                    const ::furbbs::CreateCollectionRequest* request,
                                                    ::furbbs::CreateCollectionResponse* response) {
    auto auth = BaseController::VerifyToken(request->access_token());
    if (!auth.valid) {
        BaseController::SetResponse(response, 401, "Unauthorized");
        return ::trpc::kSuccStatus;
    }

    int64_t collection_id = service::EconomyService::Instance().CreateCollection(
        auth.user_id, request->name(), request->description(),
        request->cover_image(), request->is_public());

    response->set_collection_id(collection_id);
    BaseController::SetResponse(response, collection_id > 0 ? 200 : 400,
                               collection_id > 0 ? "Created" : "Failed");
    return ::trpc::kSuccStatus;
}

::trpc::Status AddToCollection(::trpc::ServerContextPtr context,
                                                   const ::furbbs::AddToCollectionRequest* request,
                                                   ::furbbs::AddToCollectionResponse* response) {
    auto auth = BaseController::VerifyToken(request->access_token());
    if (!auth.valid) {
        BaseController::SetResponse(response, 401, "Unauthorized");
        return ::trpc::kSuccStatus;
    }

    bool success = service::EconomyService::Instance().AddToCollection(
        auth.user_id, request->collection_id(), request->post_id());

    BaseController::SetResponse(response, success ? 200 : 403,
                               success ? "Added" : "Permission denied");
    return ::trpc::kSuccStatus;
}

::trpc::Status GetCollections(::trpc::ServerContextPtr context,
                                                  const ::furbbs::GetCollectionsRequest* request,
                                                  ::furbbs::GetCollectionsResponse* response) {
    std::string viewer_id;
    auto auth = BaseController::VerifyToken(request->access_token());
    if (auth.valid) viewer_id = auth.user_id;

    auto collections = service::EconomyService::Instance().GetUserCollections(
        request->access_token(), request->user_id(),
        request->page(), request->page_size());

    for (const auto& c : collections) {
        auto* col = response->add_collections();
        col->set_id(c.id);
        col->set_name(c.name);
        col->set_item_count(c.item_count);
    }
    BaseController::SetResponse(response, 200, "Success");
    return ::trpc::kSuccStatus;
}

} // namespace furbbs::controller
