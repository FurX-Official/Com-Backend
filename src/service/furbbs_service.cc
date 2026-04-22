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
#include "../controller/community_controller.cc"
#include "../controller/community_enhanced_controller.cc"
#include "../controller/security_controller.cc"
#include "../controller/advanced_controller.cc"
#include "../controller/customization_controller.cc"
#include "../controller/shop_controller.cc"
#include "../controller/verification_controller.cc"

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

// ==================== 社区功能接口 ====================

::trpc::Status FurBBSServiceImpl::SaveFursonaCard(::trpc::ServerContextPtr context,
                                                   const ::furbbs::SaveFursonaCardRequest* request,
                                                   ::furbbs::SaveFursonaCardResponse* response) {
    return ::furbbs::controller::SaveFursonaCard(context, request, response);
}

::trpc::Status FurBBSServiceImpl::GetFursonaCard(::trpc::ServerContextPtr context,
                                                  const ::furbbs::GetFursonaCardRequest* request,
                                                  ::furbbs::GetFursonaCardResponse* response) {
    return ::furbbs::controller::GetFursonaCard(context, request, response);
}

::trpc::Status FurBBSServiceImpl::SetContentRating(::trpc::ServerContextPtr context,
                                                    const ::furbbs::SetContentRatingRequest* request,
                                                    ::furbbs::SetContentRatingResponse* response) {
    return ::furbbs::controller::SetContentRating(context, request, response);
}

::trpc::Status FurBBSServiceImpl::UpdateContentPrefs(::trpc::ServerContextPtr context,
                                                      const ::furbbs::UpdateContentPrefsRequest* request,
                                                      ::furbbs::UpdateContentPrefsResponse* response) {
    return ::furbbs::controller::UpdateContentPrefs(context, request, response);
}

::trpc::Status FurBBSServiceImpl::RequestCreationPermission(::trpc::ServerContextPtr context,
                                                             const ::furbbs::RequestCreationPermissionRequest* request,
                                                             ::furbbs::RequestCreationPermissionResponse* response) {
    return ::furbbs::controller::RequestCreationPermission(context, request, response);
}

::trpc::Status FurBBSServiceImpl::ApprovePermission(::trpc::ServerContextPtr context,
                                                     const ::furbbs::ApprovePermissionRequest* request,
                                                     ::furbbs::ApprovePermissionResponse* response) {
    return ::furbbs::controller::ApprovePermission(context, request, response);
}

::trpc::Status FurBBSServiceImpl::AddFursonaInteraction(::trpc::ServerContextPtr context,
                                                          const ::furbbs::AddFursonaInteractionRequest* request,
                                                          ::furbbs::AddFursonaInteractionResponse* response) {
    return ::furbbs::controller::AddFursonaInteraction(context, request, response);
}

::trpc::Status FurBBSServiceImpl::SubmitModeration(::trpc::ServerContextPtr context,
                                                     const ::furbbs::SubmitModerationRequest* request,
                                                     ::furbbs::SubmitModerationResponse* response) {
    return ::furbbs::controller::SubmitModeration(context, request, response);
}

::trpc::Status FurBBSServiceImpl::ReviewModeration(::trpc::ServerContextPtr context,
                                                     const ::furbbs::ReviewModerationRequest* request,
                                                     ::furbbs::ReviewModerationResponse* response) {
    return ::furbbs::controller::ReviewModeration(context, request, response);
}

// ==================== 核心基础功能接口 ====================

::trpc::Status FurBBSServiceImpl::CreateGalleryItem(::trpc::ServerContextPtr context,
                                                      const ::furbbs::CreateGalleryItemRequest* request,
                                                      ::furbbs::CreateGalleryItemResponse* response) {
    return ::furbbs::controller::CreateGalleryItem(context, request, response);
}

::trpc::Status FurBBSServiceImpl::GetUserGallery(::trpc::ServerContextPtr context,
                                                   const ::furbbs::GetUserGalleryRequest* request,
                                                   ::furbbs::GetUserGalleryResponse* response) {
    return ::furbbs::controller::GetUserGallery(context, request, response);
}

::trpc::Status FurBBSServiceImpl::LikeGalleryItem(::trpc::ServerContextPtr context,
                                                    const ::furbbs::LikeGalleryItemRequest* request,
                                                    ::furbbs::LikeGalleryItemResponse* response) {
    return ::furbbs::controller::LikeGalleryItem(context, request, response);
}

::trpc::Status FurBBSServiceImpl::CreateAlbum(::trpc::ServerContextPtr context,
                                                const ::furbbs::CreateAlbumRequest* request,
                                                ::furbbs::CreateAlbumResponse* response) {
    return ::furbbs::controller::CreateAlbum(context, request, response);
}

::trpc::Status FurBBSServiceImpl::SearchContent(::trpc::ServerContextPtr context,
                                                  const ::furbbs::SearchContentRequest* request,
                                                  ::furbbs::SearchContentResponse* response) {
    return ::furbbs::controller::SearchContent(context, request, response);
}

::trpc::Status FurBBSServiceImpl::UpdatePresence(::trpc::ServerContextPtr context,
                                                   const ::furbbs::UpdatePresenceRequest* request,
                                                   ::furbbs::UpdatePresenceResponse* response) {
    return ::furbbs::controller::UpdatePresence(context, request, response);
}

::trpc::Status FurBBSServiceImpl::GetOnlineUsers(::trpc::ServerContextPtr context,
                                                  const ::furbbs::GetOnlineUsersRequest* request,
                                                  ::furbbs::GetOnlineUsersResponse* response) {
    return ::furbbs::controller::GetOnlineUsers(context, request, response);
}

::trpc::Status FurBBSServiceImpl::RequestDataExport(::trpc::ServerContextPtr context,
                                                     const ::furbbs::RequestDataExportRequest* request,
                                                     ::furbbs::RequestDataExportResponse* response) {
    return ::furbbs::controller::RequestDataExport(context, request, response);
}

::trpc::Status FurBBSServiceImpl::GetPublicConfigs(::trpc::ServerContextPtr context,
                                                    const ::furbbs::GetPublicConfigsRequest* request,
                                                    ::furbbs::GetPublicConfigsResponse* response) {
    return ::furbbs::controller::GetPublicConfigs(context, request, response);
}

::trpc::Status FurBBSServiceImpl::GetAuditLogs(::trpc::ServerContextPtr context,
                                                 const ::furbbs::GetAuditLogsRequest* request,
                                                 ::furbbs::GetAuditLogsResponse* response) {
    return ::furbbs::controller::GetAuditLogs(context, request, response);
}

::trpc::Status FurBBSServiceImpl::BlockUser(::trpc::ServerContextPtr context,
                                             const ::furbbs::BlockUserRequest* request,
                                             ::furbbs::BlockUserResponse* response) {
    return ::furbbs::controller::BlockUser(context, request, response);
}

::trpc::Status FurBBSServiceImpl::UnblockUser(::trpc::ServerContextPtr context,
                                               const ::furbbs::UnblockUserRequest* request,
                                               ::furbbs::UnblockUserResponse* response) {
    return ::furbbs::controller::UnblockUser(context, request, response);
}

::trpc::Status FurBBSServiceImpl::GetBlockedUsers(::trpc::ServerContextPtr context,
                                                   const ::furbbs::GetBlockedUsersRequest* request,
                                                   ::furbbs::GetBlockedUsersResponse* response) {
    return ::furbbs::controller::GetBlockedUsers(context, request, response);
}

::trpc::Status FurBBSServiceImpl::GetUserSessions(::trpc::ServerContextPtr context,
                                                   const ::furbbs::GetUserSessionsRequest* request,
                                                   ::furbbs::GetUserSessionsResponse* response) {
    return ::furbbs::controller::GetUserSessions(context, request, response);
}

::trpc::Status FurBBSServiceImpl::RevokeSession(::trpc::ServerContextPtr context,
                                                 const ::furbbs::RevokeSessionRequest* request,
                                                 ::furbbs::RevokeSessionResponse* response) {
    return ::furbbs::controller::RevokeSession(context, request, response);
}

::trpc::Status FurBBSServiceImpl::GetLoginHistory(::trpc::ServerContextPtr context,
                                                   const ::furbbs::GetLoginHistoryRequest* request,
                                                   ::furbbs::GetLoginHistoryResponse* response) {
    return ::furbbs::controller::GetLoginHistory(context, request, response);
}

::trpc::Status FurBBSServiceImpl::GetAnnouncements(::trpc::ServerContextPtr context,
                                                    const ::furbbs::GetAnnouncementsRequest* request,
                                                    ::furbbs::GetAnnouncementsResponse* response) {
    return ::furbbs::controller::GetAnnouncements(context, request, response);
}

::trpc::Status FurBBSServiceImpl::CreateAnnouncement(::trpc::ServerContextPtr context,
                                                      const ::furbbs::CreateAnnouncementRequest* request,
                                                      ::furbbs::CreateAnnouncementResponse* response) {
    return ::furbbs::controller::CreateAnnouncement(context, request, response);
}

::trpc::Status FurBBSServiceImpl::GetSecurityAlerts(::trpc::ServerContextPtr context,
                                                     const ::furbbs::GetSecurityAlertsRequest* request,
                                                     ::furbbs::GetSecurityAlertsResponse* response) {
    return ::furbbs::controller::GetSecurityAlerts(context, request, response);
}

::trpc::Status FurBBSServiceImpl::ResolveAlert(::trpc::ServerContextPtr context,
                                                const ::furbbs::ResolveAlertRequest* request,
                                                ::furbbs::ResolveAlertResponse* response) {
    return ::furbbs::controller::ResolveAlert(context, request, response);
}

::trpc::Status FurBBSServiceImpl::AddIpToBlacklist(::trpc::ServerContextPtr context,
                                                    const ::furbbs::AddIpToBlacklistRequest* request,
                                                    ::furbbs::AddIpToBlacklistResponse* response) {
    return ::furbbs::controller::AddIpToBlacklist(context, request, response);
}

::trpc::Status FurBBSServiceImpl::CreateWebhook(::trpc::ServerContextPtr context,
                                                 const ::furbbs::CreateWebhookRequest* request,
                                                 ::furbbs::CreateWebhookResponse* response) {
    return ::furbbs::controller::CreateWebhook(context, request, response);
}

::trpc::Status FurBBSServiceImpl::GetUserWebhooks(::trpc::ServerContextPtr context,
                                                   const ::furbbs::GetUserWebhooksRequest* request,
                                                   ::furbbs::GetUserWebhooksResponse* response) {
    return ::furbbs::controller::GetUserWebhooks(context, request, response);
}

::trpc::Status FurBBSServiceImpl::CreateContentReport(::trpc::ServerContextPtr context,
                                                       const ::furbbs::CreateContentReportRequest* request,
                                                       ::furbbs::CreateContentReportResponse* response) {
    return ::furbbs::controller::CreateContentReport(context, request, response);
}

::trpc::Status FurBBSServiceImpl::GetPendingReports(::trpc::ServerContextPtr context,
                                                      const ::furbbs::GetPendingReportsRequest* request,
                                                      ::furbbs::GetPendingReportsResponse* response) {
    return ::furbbs::controller::GetPendingReports(context, request, response);
}

::trpc::Status FurBBSServiceImpl::HandleReport(::trpc::ServerContextPtr context,
                                                 const ::furbbs::HandleReportRequest* request,
                                                 ::furbbs::HandleReportResponse* response) {
    return ::furbbs::controller::HandleReport(context, request, response);
}

::trpc::Status FurBBSServiceImpl::CreateCommentReply(::trpc::ServerContextPtr context,
                                                      const ::furbbs::CreateCommentReplyRequest* request,
                                                      ::furbbs::CreateCommentReplyResponse* response) {
    return ::furbbs::controller::CreateCommentReply(context, request, response);
}

::trpc::Status FurBBSServiceImpl::GetCommentReplies(::trpc::ServerContextPtr context,
                                                     const ::furbbs::GetCommentRepliesRequest* request,
                                                     ::furbbs::GetCommentRepliesResponse* response) {
    return ::furbbs::controller::GetCommentReplies(context, request, response);
}

::trpc::Status FurBBSServiceImpl::DeleteCommentReply(::trpc::ServerContextPtr context,
                                                       const ::furbbs::DeleteCommentReplyRequest* request,
                                                       ::furbbs::DeleteCommentReplyResponse* response) {
    return ::furbbs::controller::DeleteCommentReply(context, request, response);
}

::trpc::Status FurBBSServiceImpl::SetPostSticky(::trpc::ServerContextPtr context,
                                                  const ::furbbs::SetPostStickyRequest* request,
                                                  ::furbbs::SetPostStickyResponse* response) {
    return ::furbbs::controller::SetPostSticky(context, request, response);
}

::trpc::Status FurBBSServiceImpl::RemovePostSticky(::trpc::ServerContextPtr context,
                                                     const ::furbbs::RemovePostStickyRequest* request,
                                                     ::furbbs::RemovePostStickyResponse* response) {
    return ::furbbs::controller::RemovePostSticky(context, request, response);
}

::trpc::Status FurBBSServiceImpl::SetPostDigest(::trpc::ServerContextPtr context,
                                                  const ::furbbs::SetPostDigestRequest* request,
                                                  ::furbbs::SetPostDigestResponse* response) {
    return ::furbbs::controller::SetPostDigest(context, request, response);
}

::trpc::Status FurBBSServiceImpl::CreateCollectionFolder(::trpc::ServerContextPtr context,
                                                          const ::furbbs::CreateCollectionFolderRequest* request,
                                                          ::furbbs::CreateCollectionFolderResponse* response) {
    return ::furbbs::controller::CreateCollectionFolder(context, request, response);
}

::trpc::Status FurBBSServiceImpl::GetUserFolders(::trpc::ServerContextPtr context,
                                                   const ::furbbs::GetUserFoldersRequest* request,
                                                   ::furbbs::GetUserFoldersResponse* response) {
    return ::furbbs::controller::GetUserFolders(context, request, response);
}

::trpc::Status FurBBSServiceImpl::AddToCollectionFolder(::trpc::ServerContextPtr context,
                                                         const ::furbbs::AddToCollectionFolderRequest* request,
                                                         ::furbbs::AddToCollectionFolderResponse* response) {
    return ::furbbs::controller::AddToCollectionFolder(context, request, response);
}

::trpc::Status FurBBSServiceImpl::AddUserTag(::trpc::ServerContextPtr context,
                                              const ::furbbs::AddUserTagRequest* request,
                                              ::furbbs::AddUserTagResponse* response) {
    return ::furbbs::controller::AddUserTag(context, request, response);
}

::trpc::Status FurBBSServiceImpl::RemoveUserTag(::trpc::ServerContextPtr context,
                                                 const ::furbbs::RemoveUserTagRequest* request,
                                                 ::furbbs::RemoveUserTagResponse* response) {
    return ::furbbs::controller::RemoveUserTag(context, request, response);
}

::trpc::Status FurBBSServiceImpl::AddKeywordFilter(::trpc::ServerContextPtr context,
                                                    const ::furbbs::AddKeywordFilterRequest* request,
                                                    ::furbbs::AddKeywordFilterResponse* response) {
    return ::furbbs::controller::AddKeywordFilter(context, request, response);
}

::trpc::Status FurBBSServiceImpl::GetKeywordFilters(::trpc::ServerContextPtr context,
                                                     const ::furbbs::GetKeywordFiltersRequest* request,
                                                     ::furbbs::GetKeywordFiltersResponse* response) {
    return ::furbbs::controller::GetKeywordFilters(context, request, response);
}

::trpc::Status FurBBSServiceImpl::SavePostDraft(::trpc::ServerContextPtr context,
                                                 const ::furbbs::SavePostDraftRequest* request,
                                                 ::furbbs::SavePostDraftResponse* response) {
    return ::furbbs::controller::SavePostDraft(context, request, response);
}

::trpc::Status FurBBSServiceImpl::GetUserDrafts(::trpc::ServerContextPtr context,
                                                  const ::furbbs::GetUserDraftsRequest* request,
                                                  ::furbbs::GetUserDraftsResponse* response) {
    return ::furbbs::controller::GetUserDrafts(context, request, response);
}

::trpc::Status FurBBSServiceImpl::RecordPostView(::trpc::ServerContextPtr context,
                                                   const ::furbbs::RecordPostViewRequest* request,
                                                   ::furbbs::RecordPostViewResponse* response) {
    return ::furbbs::controller::RecordPostView(context, request, response);
}

::trpc::Status FurBBSServiceImpl::RecordPostShare(::trpc::ServerContextPtr context,
                                                    const ::furbbs::RecordPostShareRequest* request,
                                                    ::furbbs::RecordPostShareResponse* response) {
    return ::furbbs::controller::RecordPostShare(context, request, response);
}

// ==================== 用户自定义相关接口 ====================

::trpc::Status FurBBSServiceImpl::GetProfileCustom(::trpc::ServerContextPtr context,
                                                     const ::furbbs::GetProfileCustomRequest* request,
                                                     ::furbbs::GetProfileCustomResponse* response) {
    return ::furbbs::controller::GetProfileCustom(context, request, response);
}

::trpc::Status FurBBSServiceImpl::UpdateProfileCustom(::trpc::ServerContextPtr context,
                                                        const ::furbbs::UpdateProfileCustomRequest* request,
                                                        ::furbbs::UpdateProfileCustomResponse* response) {
    return ::furbbs::controller::UpdateProfileCustom(context, request, response);
}

::trpc::Status FurBBSServiceImpl::GetFursonaCardCustom(::trpc::ServerContextPtr context,
                                                         const ::furbbs::GetFursonaCardCustomRequest* request,
                                                         ::furbbs::GetFursonaCardCustomResponse* response) {
    return ::furbbs::controller::GetFursonaCardCustom(context, request, response);
}

::trpc::Status FurBBSServiceImpl::UpdateFursonaCardCustom(::trpc::ServerContextPtr context,
                                                            const ::furbbs::UpdateFursonaCardCustomRequest* request,
                                                            ::furbbs::UpdateFursonaCardCustomResponse* response) {
    return ::furbbs::controller::UpdateFursonaCardCustom(context, request, response);
}

::trpc::Status FurBBSServiceImpl::GetNotificationSettings(::trpc::ServerContextPtr context,
                                                            const ::furbbs::GetNotificationSettingsRequest* request,
                                                            ::furbbs::GetNotificationSettingsResponse* response) {
    return ::furbbs::controller::GetNotificationSettings(context, request, response);
}

::trpc::Status FurBBSServiceImpl::UpdateNotificationSettings(::trpc::ServerContextPtr context,
                                                               const ::furbbs::UpdateNotificationSettingsRequest* request,
                                                               ::furbbs::UpdateNotificationSettingsResponse* response) {
    return ::furbbs::controller::UpdateNotificationSettings(context, request, response);
}

::trpc::Status FurBBSServiceImpl::GetFeedSettings(::trpc::ServerContextPtr context,
                                                   const ::furbbs::GetFeedSettingsRequest* request,
                                                   ::furbbs::GetFeedSettingsResponse* response) {
    return ::furbbs::controller::GetFeedSettings(context, request, response);
}

::trpc::Status FurBBSServiceImpl::UpdateFeedSettings(::trpc::ServerContextPtr context,
                                                      const ::furbbs::UpdateFeedSettingsRequest* request,
                                                      ::furbbs::UpdateFeedSettingsResponse* response) {
    return ::furbbs::controller::UpdateFeedSettings(context, request, response);
}

::trpc::Status FurBBSServiceImpl::CreateTheme(::trpc::ServerContextPtr context,
                                               const ::furbbs::CreateThemeRequest* request,
                                               ::furbbs::CreateThemeResponse* response) {
    return ::furbbs::controller::CreateTheme(context, request, response);
}

::trpc::Status FurBBSServiceImpl::GetThemeList(::trpc::ServerContextPtr context,
                                                const ::furbbs::GetThemeListRequest* request,
                                                ::furbbs::GetThemeListResponse* response) {
    return ::furbbs::controller::GetThemeList(context, request, response);
}

// ==================== 消息与私信相关接口 ====================

::trpc::Status FurBBSServiceImpl::SendMessage(::trpc::ServerContextPtr context,
                                               const ::furbbs::SendMessageRequest* request,
                                               ::furbbs::SendMessageResponse* response) {
    return ::furbbs::controller::SendMessage(context, request, response);
}

::trpc::Status FurBBSServiceImpl::GetMessages(::trpc::ServerContextPtr context,
                                               const ::furbbs::GetMessagesRequest* request,
                                               ::furbbs::GetMessagesResponse* response) {
    return ::furbbs::controller::GetMessages(context, request, response);
}

::trpc::Status FurBBSServiceImpl::GetMessageConversations(::trpc::ServerContextPtr context,
                                                           const ::furbbs::GetMessageConversationsRequest* request,
                                                           ::furbbs::GetMessageConversationsResponse* response) {
    return ::furbbs::controller::GetMessageConversations(context, request, response);
}

::trpc::Status FurBBSServiceImpl::MarkMessageRead(::trpc::ServerContextPtr context,
                                                   const ::furbbs::MarkMessageReadRequest* request,
                                                   ::furbbs::MarkMessageReadResponse* response) {
    return ::furbbs::controller::MarkMessageRead(context, request, response);
}

// ==================== 管理员与权限相关接口 ====================

::trpc::Status FurBBSServiceImpl::GrantPermission(::trpc::ServerContextPtr context,
                                                    const ::furbbs::GrantPermissionRequest* request,
                                                    ::furbbs::GrantPermissionResponse* response) {
    return ::furbbs::controller::GrantPermission(context, request, response);
}

::trpc::Status FurBBSServiceImpl::RevokePermission(::trpc::ServerContextPtr context,
                                                     const ::furbbs::RevokePermissionRequest* request,
                                                     ::furbbs::RevokePermissionResponse* response) {
    return ::furbbs::controller::RevokePermission(context, request, response);
}

::trpc::Status FurBBSServiceImpl::ListUserPermissions(::trpc::ServerContextPtr context,
                                                        const ::furbbs::ListUserPermissionsRequest* request,
                                                        ::furbbs::ListUserPermissionsResponse* response) {
    return ::furbbs::controller::ListUserPermissions(context, request, response);
}

::trpc::Status FurBBSServiceImpl::ModeratePost(::trpc::ServerContextPtr context,
                                                 const ::furbbs::ModeratePostRequest* request,
                                                 ::furbbs::ModeratePostResponse* response) {
    return ::furbbs::controller::ModeratePost(context, request, response);
}

::trpc::Status FurBBSServiceImpl::BanUser(::trpc::ServerContextPtr context,
                                           const ::furbbs::BanUserRequest* request,
                                           ::furbbs::BanUserResponse* response) {
    return ::furbbs::controller::BanUser(context, request, response);
}

::trpc::Status FurBBSServiceImpl::UnbanUser(::trpc::ServerContextPtr context,
                                             const ::furbbs::UnbanUserRequest* request,
                                             ::furbbs::UnbanUserResponse* response) {
    return ::furbbs::controller::UnbanUser(context, request, response);
}

::trpc::Status FurBBSServiceImpl::GetUserBan(::trpc::ServerContextPtr context,
                                              const ::furbbs::GetUserBanRequest* request,
                                              ::furbbs::GetUserBanResponse* response) {
    return ::furbbs::controller::GetUserBan(context, request, response);
}

::trpc::Status FurBBSServiceImpl::GetModeratorLogs(::trpc::ServerContextPtr context,
                                                    const ::furbbs::GetModeratorLogsRequest* request,
                                                    ::furbbs::GetModeratorLogsResponse* response) {
    return ::furbbs::controller::GetModeratorLogs(context, request, response);
}

::trpc::Status FurBBSServiceImpl::CreateReport(::trpc::ServerContextPtr context,
                                                const ::furbbs::CreateReportRequest* request,
                                                ::furbbs::CreateReportResponse* response) {
    return ::furbbs::controller::CreateReport(context, request, response);
}

::trpc::Status FurBBSServiceImpl::GetReports(::trpc::ServerContextPtr context,
                                              const ::furbbs::GetReportsRequest* request,
                                              ::furbbs::GetReportsResponse* response) {
    return ::furbbs::controller::GetReports(context, request, response);
}

// ==================== 商店与经济相关接口 ====================

::trpc::Status FurBBSServiceImpl::GetShopItems(::trpc::ServerContextPtr context,
                                                  const ::furbbs::GetShopItemsRequest* request,
                                                  ::furbbs::GetShopItemsResponse* response) {
    return ::furbbs::controller::GetShopItems(context, request, response);
}

::trpc::Status FurBBSServiceImpl::PurchaseItem(::trpc::ServerContextPtr context,
                                               const ::furbbs::PurchaseItemRequest* request,
                                               ::furbbs::PurchaseItemResponse* response) {
    return ::furbbs::controller::PurchaseItem(context, request, response);
}

::trpc::Status FurBBSServiceImpl::GetUserInventory(::trpc::ServerContextPtr context,
                                                     const ::furbbs::GetUserInventoryRequest* request,
                                                     ::furbbs::GetUserInventoryResponse* response) {
    return ::furbbs::controller::GetUserInventory(context, request, response);
}

::trpc::Status FurBBSServiceImpl::DailyCheckIn(::trpc::ServerContextPtr context,
                                            const ::furbbs::DailyCheckInRequest* request,
                                            ::furbbs::DailyCheckInResponse* response) {
    return ::furbbs::controller::DailyCheckIn(context, request, response);
}

::trpc::Status FurBBSServiceImpl::GetCheckInStatus(::trpc::ServerContextPtr context,
                                                     const ::furbbs::GetCheckInStatusRequest* request,
                                                     ::furbbs::GetCheckInStatusResponse* response) {
    return ::furbbs::controller::GetCheckInStatus(context, request, response);
}

::trpc::Status FurBBSServiceImpl::GetUserLevel(::trpc::ServerContextPtr context,
                                                const ::furbbs::GetUserLevelRequest* request,
                                                ::furbbs::GetUserLevelResponse* response) {
    return ::furbbs::controller::GetUserLevel(context, request, response);
}

// ==================== 实名验证相关接口 ====================

::trpc::Status FurBBSServiceImpl::RealNameVerify(::trpc::ServerContextPtr context,
                                                   const ::furbbs::RealNameVerifyRequest* request,
                                                   ::furbbs::RealNameVerifyResponse* response) {
    return ::furbbs::controller::RealNameVerify(context, request, response);
}

::trpc::Status FurBBSServiceImpl::FaceVerify(::trpc::ServerContextPtr context,
                                              const ::furbbs::FaceVerifyRequest* request,
                                              ::furbbs::FaceVerifyResponse* response) {
    return ::furbbs::controller::FaceVerify(context, request, response);
}

::trpc::Status FurBBSServiceImpl::FaceCompare(::trpc::ServerContextPtr context,
                                               const ::furbbs::FaceCompareRequest* request,
                                               ::furbbs::FaceCompareResponse* response) {
    return ::furbbs::controller::FaceCompare(context, request, response);
}

::trpc::Status FurBBSServiceImpl::GetVerifyStatus(::trpc::ServerContextPtr context,
                                                    const ::furbbs::GetVerifyStatusRequest* request,
                                                    ::furbbs::GetVerifyStatusResponse* response) {
    return ::furbbs::controller::GetVerifyStatus(context, request, response);
}

::trpc::Status FurBBSServiceImpl::QueryVerifyResult(::trpc::ServerContextPtr context,
                                                      const ::furbbs::QueryVerifyResultRequest* request,
                                                      ::furbbs::QueryVerifyResultResponse* response) {
    return ::furbbs::controller::QueryVerifyResult(context, request, response);
}

} // namespace furbbs::service
