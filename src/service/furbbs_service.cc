#include "furbbs_service.h"
#include "../auth/casdoor_auth.h"
#include "../common/captcha_verifier.h"
#include "../common/open_platform.h"
#include "../common/infrastructure.h"
#include "../service_impl/redeem_service.h"
#include "../service_impl/customization_service.h"
#include "../service_impl/shop_service.h"
#include "../service_impl/social_service.h"
#include "../service_impl/advanced_service.h"

#include "../controller/user_controller.cc"
#include "../controller/post_controller.cc"
#include "../controller/social_controller.cc"
#include "../controller/economy_controller.cc"
#include "../controller/furry_core_controller.cc"

namespace furbbs::service {

// ==================== 用户相关接口 ====================

::trpc::Status FurBBSServiceImpl::GetUserInfo(::trpc::ServerContextPtr context,
                                               const ::furbbs::GetUserInfoRequest* request,
                                               ::furbbs::GetUserInfoResponse* response) {
    return ::furbbs::controller::GetUserInfo(context, request, response);
}

::trpc::Status FurBBSServiceImpl::UpdateUserProfile(::trpc::ServerContextPtr context,
                                                     const ::furbbs::UpdateProfileRequest* request,
                                                     ::furbbs::UpdateProfileResponse* response) {
    return ::furbbs::controller::UpdateUserProfile(context, request, response);
}

::trpc::Status FurBBSServiceImpl::GetUserStats(::trpc::ServerContextPtr context,
                                                const ::furbbs::GetUserStatsRequest* request,
                                                ::furbbs::GetUserStatsResponse* response) {
    return ::furbbs::controller::GetUserStats(context, request, response);
}

::trpc::Status FurBBSServiceImpl::SearchUsers(::trpc::ServerContextPtr context,
                                               const ::furbbs::SearchUsersRequest* request,
                                               ::furbbs::SearchUsersResponse* response) {
    return ::furbbs::controller::SearchUsers(context, request, response);
}

::trpc::Status FurBBSServiceImpl::GetFursonaList(::trpc::ServerContextPtr context,
                                                  const ::furbbs::GetFursonaListRequest* request,
                                                  ::furbbs::GetFursonaListResponse* response) {
    return ::furbbs::controller::GetFursonaList(context, request, response);
}

::trpc::Status FurBBSServiceImpl::CreateFursona(::trpc::ServerContextPtr context,
                                                 const ::furbbs::CreateFursonaRequest* request,
                                                 ::furbbs::CreateFursonaResponse* response) {
    return ::furbbs::controller::CreateFursona(context, request, response);
}

::trpc::Status FurBBSServiceImpl::UpdateFursona(::trpc::ServerContextPtr context,
                                                 const ::furbbs::UpdateFursonaRequest* request,
                                                 ::furbbs::UpdateFursonaResponse* response) {
    return ::furbbs::controller::UpdateFursona(context, request, response);
}

::trpc::Status FurBBSServiceImpl::DeleteFursona(::trpc::ServerContextPtr context,
                                                 const ::furbbs::DeleteFursonaRequest* request,
                                                 ::furbbs::DeleteFursonaResponse* response) {
    return ::furbbs::controller::DeleteFursona(context, request, response);
}

// ==================== 帖子相关接口 ====================

::trpc::Status FurBBSServiceImpl::CreatePost(::trpc::ServerContextPtr context,
                                              const ::furbbs::CreatePostRequest* request,
                                              ::furbbs::CreatePostResponse* response) {
    return ::furbbs::controller::CreatePost(context, request, response);
}

::trpc::Status FurBBSServiceImpl::GetPost(::trpc::ServerContextPtr context,
                                            const ::furbbs::GetPostRequest* request,
                                            ::furbbs::GetPostResponse* response) {
    return ::furbbs::controller::GetPost(context, request, response);
}

::trpc::Status FurBBSServiceImpl::GetPostList(::trpc::ServerContextPtr context,
                                               const ::furbbs::GetPostListRequest* request,
                                               ::furbbs::GetPostListResponse* response) {
    return ::furbbs::controller::GetPostList(context, request, response);
}

::trpc::Status FurBBSServiceImpl::UpdatePost(::trpc::ServerContextPtr context,
                                              const ::furbbs::UpdatePostRequest* request,
                                              ::furbbs::UpdatePostResponse* response) {
    return ::furbbs::controller::UpdatePost(context, request, response);
}

::trpc::Status FurBBSServiceImpl::DeletePost(::trpc::ServerContextPtr context,
                                              const ::furbbs::DeletePostRequest* request,
                                              ::furbbs::DeletePostResponse* response) {
    return ::furbbs::controller::DeletePost(context, request, response);
}

::trpc::Status FurBBSServiceImpl::LikePost(::trpc::ServerContextPtr context,
                                             const ::furbbs::LikePostRequest* request,
                                             ::furbbs::LikePostResponse* response) {
    return ::furbbs::controller::LikePost(context, request, response);
}

::trpc::Status FurBBSServiceImpl::CreateComment(::trpc::ServerContextPtr context,
                                                 const ::furbbs::CreateCommentRequest* request,
                                                 ::furbbs::CreateCommentResponse* response) {
    return ::furbbs::controller::CreateComment(context, request, response);
}

::trpc::Status FurBBSServiceImpl::GetComments(::trpc::ServerContextPtr context,
                                                const ::furbbs::GetCommentsRequest* request,
                                                ::furbbs::GetCommentsResponse* response) {
    return ::furbbs::controller::GetComments(context, request, response);
}

::trpc::Status FurBBSServiceImpl::DeleteComment(::trpc::ServerContextPtr context,
                                                   const ::furbbs::DeleteCommentRequest* request,
                                                   ::furbbs::DeleteCommentResponse* response) {
    return ::furbbs::controller::DeleteComment(context, request, response);
}

::trpc::Status FurBBSServiceImpl::SearchPosts(::trpc::ServerContextPtr context,
                                                const ::furbbs::SearchPostsRequest* request,
                                                ::furbbs::SearchPostsResponse* response) {
    return ::furbbs::controller::SearchPosts(context, request, response);
}

::trpc::Status FurBBSServiceImpl::SetPostEssence(::trpc::ServerContextPtr context,
                                                   const ::furbbs::SetPostEssenceRequest* request,
                                                   ::furbbs::SetPostEssenceResponse* response) {
    return ::furbbs::controller::SetPostEssence(context, request, response);
}

::trpc::Status FurBBSServiceImpl::SetPostSticky(::trpc::ServerContextPtr context,
                                                 const ::furbbs::SetPostStickyRequest* request,
                                                 ::furbbs::SetPostStickyResponse* response) {
    return ::furbbs::controller::SetPostSticky(context, request, response);
}

// ==================== 社交相关接口 ====================

::trpc::Status FurBBSServiceImpl::FollowUser(::trpc::ServerContextPtr context,
                                              const ::furbbs::FollowUserRequest* request,
                                              ::furbbs::FollowUserResponse* response) {
    return ::furbbs::controller::FollowUser(context, request, response);
}

::trpc::Status FurBBSServiceImpl::GetFollowers(::trpc::ServerContextPtr context,
                                                 const ::furbbs::GetFollowListRequest* request,
                                                 ::furbbs::GetFollowListResponse* response) {
    return ::furbbs::controller::GetFollowers(context, request, response);
}

::trpc::Status FurBBSServiceImpl::GetFollowing(::trpc::ServerContextPtr context,
                                                 const ::furbbs::GetFollowListRequest* request,
                                                 ::furbbs::GetFollowListResponse* response) {
    return ::furbbs::controller::GetFollowing(context, request, response);
}

::trpc::Status FurBBSServiceImpl::CheckFriendship(::trpc::ServerContextPtr context,
                                                    const ::furbbs::CheckFriendshipRequest* request,
                                                    ::furbbs::CheckFriendshipResponse* response) {
    return ::furbbs::controller::CheckFriendship(context, request, response);
}

::trpc::Status FurBBSServiceImpl::FavoriteFursona(::trpc::ServerContextPtr context,
                                                    const ::furbbs::FavoriteFursonaRequest* request,
                                                    ::furbbs::FavoriteFursonaResponse* response) {
    return ::furbbs::controller::FavoriteFursona(context, request, response);
}

::trpc::Status FurBBSServiceImpl::SendGift(::trpc::ServerContextPtr context,
                                             const ::furbbs::SendGiftRequest* request,
                                             ::furbbs::SendGiftResponse* response) {
    return ::furbbs::controller::SendGift(context, request, response);
}

::trpc::Status FurBBSServiceImpl::GetUserGifts(::trpc::ServerContextPtr context,
                                                 const ::furbbs::GetUserGiftsRequest* request,
                                                 ::furbbs::GetUserGiftsResponse* response) {
    return ::furbbs::controller::GetUserGifts(context, request, response);
}

::trpc::Status FurBBSServiceImpl::RateCommission(::trpc::ServerContextPtr context,
                                                  const ::furbbs::RateCommissionRequest* request,
                                                  ::furbbs::RateCommissionResponse* response) {
    return ::furbbs::controller::RateCommission(context, request, response);
}

::trpc::Status FurBBSServiceImpl::SubmitQuestion(::trpc::ServerContextPtr context,
                                                  const ::furbbs::SubmitQuestionRequest* request,
                                                  ::furbbs::SubmitQuestionResponse* response) {
    return ::furbbs::controller::SubmitQuestion(context, request, response);
}

::trpc::Status FurBBSServiceImpl::AnswerQuestion(::trpc::ServerContextPtr context,
                                                  const ::furbbs::AnswerQuestionRequest* request,
                                                  ::furbbs::AnswerQuestionResponse* response) {
    return ::furbbs::controller::AnswerQuestion(context, request, response);
}

// ==================== 经济系统接口 ====================

::trpc::Status FurBBSServiceImpl::CreatePaidContent(::trpc::ServerContextPtr context,
                                                     const ::furbbs::CreatePaidContentRequest* request,
                                                     ::furbbs::CreatePaidContentResponse* response) {
    return ::furbbs::controller::CreatePaidContent(context, request, response);
}

::trpc::Status FurBBSServiceImpl::GetUserPaidContents(::trpc::ServerContextPtr context,
                                                        const ::furbbs::GetUserPaidContentsRequest* request,
                                                        ::furbbs::GetUserPaidContentsResponse* response) {
    return ::furbbs::controller::GetUserPaidContents(context, request, response);
}

::trpc::Status FurBBSServiceImpl::PurchaseContent(::trpc::ServerContextPtr context,
                                                   const ::furbbs::PurchaseContentRequest* request,
                                                   ::furbbs::PurchaseContentResponse* response) {
    return ::furbbs::controller::PurchaseContent(context, request, response);
}

::trpc::Status FurBBSServiceImpl::CreateGallery(::trpc::ServerContextPtr context,
                                                  const ::furbbs::CreateGalleryRequest* request,
                                                  ::furbbs::CreateGalleryResponse* response) {
    return ::furbbs::controller::CreateGallery(context, request, response);
}

::trpc::Status FurBBSServiceImpl::GetGalleries(::trpc::ServerContextPtr context,
                                                const ::furbbs::GetGalleriesRequest* request,
                                                ::furbbs::GetGalleriesResponse* response) {
    return ::furbbs::controller::GetGalleries(context, request, response);
}

::trpc::Status FurBBSServiceImpl::AddGalleryItem(::trpc::ServerContextPtr context,
                                                  const ::furbbs::AddGalleryItemRequest* request,
                                                  ::furbbs::AddGalleryItemResponse* response) {
    return ::furbbs::controller::AddGalleryItem(context, request, response);
}

::trpc::Status FurBBSServiceImpl::GetGalleryItems(::trpc::ServerContextPtr context,
                                                   const ::furbbs::GetGalleryItemsRequest* request,
                                                   ::furbbs::GetGalleryItemsResponse* response) {
    return ::furbbs::controller::GetGalleryItems(context, request, response);
}

::trpc::Status FurBBSServiceImpl::FavoriteGallery(::trpc::ServerContextPtr context,
                                                    const ::furbbs::FavoriteGalleryRequest* request,
                                                    ::furbbs::FavoriteGalleryResponse* response) {
    return ::furbbs::controller::FavoriteGallery(context, request, response);
}

::trpc::Status FurBBSServiceImpl::TransferPoints(::trpc::ServerContextPtr context,
                                                   const ::furbbs::TransferPointsRequest* request,
                                                   ::furbbs::TransferPointsResponse* response) {
    return ::furbbs::controller::TransferPoints(context, request, response);
}

::trpc::Status FurBBSServiceImpl::GetTransferHistory(::trpc::ServerContextPtr context,
                                                      const ::furbbs::GetTransferHistoryRequest* request,
                                                      ::furbbs::GetTransferHistoryResponse* response) {
    return ::furbbs::controller::GetTransferHistory(context, request, response);
}

::trpc::Status FurBBSServiceImpl::CreateRedEnvelope(::trpc::ServerContextPtr context,
                                                      const ::furbbs::CreateRedEnvelopeRequest* request,
                                                      ::furbbs::CreateRedEnvelopeResponse* response) {
    return ::furbbs::controller::CreateRedEnvelope(context, request, response);
}

::trpc::Status FurBBSServiceImpl::ClaimRedEnvelope(::trpc::ServerContextPtr context,
                                                     const ::furbbs::ClaimRedEnvelopeRequest* request,
                                                     ::furbbs::ClaimRedEnvelopeResponse* response) {
    return ::furbbs::controller::ClaimRedEnvelope(context, request, response);
}

::trpc::Status FurBBSServiceImpl::GetRedEnvelopes(::trpc::ServerContextPtr context,
                                                    const ::furbbs::GetRedEnvelopesRequest* request,
                                                    ::furbbs::GetRedEnvelopesResponse* response) {
    return ::furbbs::controller::GetRedEnvelopes(context, request, response);
}

::trpc::Status FurBBSServiceImpl::RewardPost(::trpc::ServerContextPtr context,
                                              const ::furbbs::RewardPostRequest* request,
                                              ::furbbs::RewardPostResponse* response) {
    return ::furbbs::controller::RewardPost(context, request, response);
}

::trpc::Status FurBBSServiceImpl::GetPostRewards(::trpc::ServerContextPtr context,
                                                   const ::furbbs::GetPostRewardsRequest* request,
                                                   ::furbbs::GetPostRewardsResponse* response) {
    return ::furbbs::controller::GetPostRewards(context, request, response);
}

::trpc::Status FurBBSServiceImpl::CreateCollection(::trpc::ServerContextPtr context,
                                                    const ::furbbs::CreateCollectionRequest* request,
                                                    ::furbbs::CreateCollectionResponse* response) {
    return ::furbbs::controller::CreateCollection(context, request, response);
}

::trpc::Status FurBBSServiceImpl::AddToCollection(::trpc::ServerContextPtr context,
                                                   const ::furbbs::AddToCollectionRequest* request,
                                                   ::furbbs::AddToCollectionResponse* response) {
    return ::furbbs::controller::AddToCollection(context, request, response);
}

::trpc::Status FurBBSServiceImpl::GetCollections(::trpc::ServerContextPtr context,
                                                  const ::furbbs::GetCollectionsRequest* request,
                                                  ::furbbs::GetCollectionsResponse* response) {
    return ::furbbs::controller::GetCollections(context, request, response);
}

// ==================== Furry核心功能接口 ====================

::trpc::Status FurBBSServiceImpl::GenerateFursonaPrompt(::trpc::ServerContextPtr context,
                                                          const ::furbbs::GenerateFursonaPromptRequest* request,
                                                          ::furbbs::GenerateFursonaPromptResponse* response) {
    return ::furbbs::controller::GenerateFursonaPrompt(context, request, response);
}

::trpc::Status FurBBSServiceImpl::SavePrompt(::trpc::ServerContextPtr context,
                                               const ::furbbs::SavePromptRequest* request,
                                               ::furbbs::SavePromptResponse* response) {
    return ::furbbs::controller::SavePrompt(context, request, response);
}

::trpc::Status FurBBSServiceImpl::GetPrompts(::trpc::ServerContextPtr context,
                                              const ::furbbs::GetPromptsRequest* request,
                                              ::furbbs::GetPromptsResponse* response) {
    return ::furbbs::controller::GetPrompts(context, request, response);
}

::trpc::Status FurBBSServiceImpl::CreateFursonaRelation(::trpc::ServerContextPtr context,
                                                         const ::furbbs::CreateFursonaRelationRequest* request,
                                                         ::furbbs::CreateFursonaRelationResponse* response) {
    return ::furbbs::controller::CreateFursonaRelation(context, request, response);
}

::trpc::Status FurBBSServiceImpl::ConfirmRelation(::trpc::ServerContextPtr context,
                                                    const ::furbbs::ConfirmRelationRequest* request,
                                                    ::furbbs::ConfirmRelationResponse* response) {
    return ::furbbs::controller::ConfirmRelation(context, request, response);
}

::trpc::Status FurBBSServiceImpl::GetFursonaRelations(::trpc::ServerContextPtr context,
                                                       const ::furbbs::GetFursonaRelationsRequest* request,
                                                       ::furbbs::GetFursonaRelationsResponse* response) {
    return ::furbbs::controller::GetFursonaRelations(context, request, response);
}

::trpc::Status FurBBSServiceImpl::CreateWorldSetting(::trpc::ServerContextPtr context,
                                                      const ::furbbs::CreateWorldSettingRequest* request,
                                                      ::furbbs::CreateWorldSettingResponse* response) {
    return ::furbbs::controller::CreateWorldSetting(context, request, response);
}

::trpc::Status FurBBSServiceImpl::GetWorlds(::trpc::ServerContextPtr context,
                                             const ::furbbs::GetWorldsRequest* request,
                                             ::furbbs::GetWorldsResponse* response) {
    return ::furbbs::controller::GetWorlds(context, request, response);
}

::trpc::Status FurBBSServiceImpl::LikeWorld(::trpc::ServerContextPtr context,
                                             const ::furbbs::LikeWorldRequest* request,
                                             ::furbbs::LikeWorldResponse* response) {
    return ::furbbs::controller::LikeWorld(context, request, response);
}

::trpc::Status FurBBSServiceImpl::PurchaseEventTicket(::trpc::ServerContextPtr context,
                                                        const ::furbbs::PurchaseEventTicketRequest* request,
                                                        ::furbbs::PurchaseEventTicketResponse* response) {
    return ::furbbs::controller::PurchaseEventTicket(context, request, response);
}

::trpc::Status FurBBSServiceImpl::GetUserTickets(::trpc::ServerContextPtr context,
                                                   const ::furbbs::GetUserTicketsRequest* request,
                                                   ::furbbs::GetUserTicketsResponse* response) {
    return ::furbbs::controller::GetUserTickets(context, request, response);
}

::trpc::Status FurBBSServiceImpl::CheckInTicket(::trpc::ServerContextPtr context,
                                                  const ::furbbs::CheckInTicketRequest* request,
                                                  ::furbbs::CheckInTicketResponse* response) {
    return ::furbbs::controller::CheckInTicket(context, request, response);
}

::trpc::Status FurBBSServiceImpl::CreateMarketItem(::trpc::ServerContextPtr context,
                                                     const ::furbbs::CreateMarketItemRequest* request,
                                                     ::furbbs::CreateMarketItemResponse* response) {
    return ::furbbs::controller::CreateMarketItem(context, request, response);
}

::trpc::Status FurBBSServiceImpl::GetMarketItems(::trpc::ServerContextPtr context,
                                                  const ::furbbs::GetMarketItemsRequest* request,
                                                  ::furbbs::GetMarketItemsResponse* response) {
    return ::furbbs::controller::GetMarketItems(context, request, response);
}

::trpc::Status FurBBSServiceImpl::GetMarketItem(::trpc::ServerContextPtr context,
                                                 const ::furbbs::GetMarketItemRequest* request,
                                                 ::furbbs::GetMarketItemResponse* response) {
    return ::furbbs::controller::GetMarketItem(context, request, response);
}

::trpc::Status FurBBSServiceImpl::FavoriteMarketItem(::trpc::ServerContextPtr context,
                                                       const ::furbbs::FavoriteMarketItemRequest* request,
                                                       ::furbbs::FavoriteMarketItemResponse* response) {
    return ::furbbs::controller::FavoriteMarketItem(context, request, response);
}

::trpc::Status FurBBSServiceImpl::PurchaseMarketItem(::trpc::ServerContextPtr context,
                                                      const ::furbbs::PurchaseMarketItemRequest* request,
                                                      ::furbbs::PurchaseMarketItemResponse* response) {
    return ::furbbs::controller::PurchaseMarketItem(context, request, response);
}

::trpc::Status FurBBSServiceImpl::GetMarketTransactions(::trpc::ServerContextPtr context,
                                                          const ::furbbs::GetMarketTransactionsRequest* request,
                                                          ::furbbs::GetMarketTransactionsResponse* response) {
    return ::furbbs::controller::GetMarketTransactions(context, request, response);
}

} // namespace furbbs::service
