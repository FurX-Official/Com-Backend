#ifndef FURBBS_SERVICE_FURBBS_SERVICE_H
#define FURBBS_SERVICE_FURBBS_SERVICE_H

#include "furbbs.trpc.pb.h"

namespace furbbs::service {

class FurBBSServiceImpl : public furbbs::FurBBSService {
public:
    FurBBSServiceImpl() = default;
    ~FurBBSServiceImpl() override = default;

    ::trpc::Status GetUser(::trpc::ServerContextPtr context,
                          const ::furbbs::GetUserRequest* request,
                          ::furbbs::GetUserResponse* response) override;

    ::trpc::Status CreatePost(::trpc::ServerContextPtr context,
                             const ::furbbs::CreatePostRequest* request,
                             ::furbbs::CreatePostResponse* response) override;

    ::trpc::Status GetPost(::trpc::ServerContextPtr context,
                          const ::furbbs::GetPostRequest* request,
                          ::furbbs::GetPostResponse* response) override;

    ::trpc::Status ListPosts(::trpc::ServerContextPtr context,
                            const ::furbbs::ListPostsRequest* request,
                            ::furbbs::ListPostsResponse* response) override;

    ::trpc::Status CreateComment(::trpc::ServerContextPtr context,
                                const ::furbbs::CreateCommentRequest* request,
                                ::furbbs::CreateCommentResponse* response) override;

    ::trpc::Status ListComments(::trpc::ServerContextPtr context,
                               const ::furbbs::ListCommentsRequest* request,
                               ::furbbs::ListCommentsResponse* response) override;

    ::trpc::Status LikePost(::trpc::ServerContextPtr context,
                           const ::furbbs::LikePostRequest* request,
                           ::furbbs::LikePostResponse* response) override;

    ::trpc::Status ListCategories(::trpc::ServerContextPtr context,
                                 const ::furbbs::ListCategoriesRequest* request,
                                 ::furbbs::ListCategoriesResponse* response) override;

    ::trpc::Status UploadGallery(::trpc::ServerContextPtr context,
                                const ::furbbs::UploadGalleryRequest* request,
                                ::furbbs::UploadGalleryResponse* response) override;

    ::trpc::Status ListGallery(::trpc::ServerContextPtr context,
                              const ::furbbs::ListGalleryRequest* request,
                              ::furbbs::ListGalleryResponse* response) override;

    ::trpc::Status FollowUser(::trpc::ServerContextPtr context,
                             const ::furbbs::FollowUserRequest* request,
                             ::furbbs::FollowUserResponse* response) override;

    ::trpc::Status CreateFursona(::trpc::ServerContextPtr context,
                                const ::furbbs::CreateFursonaRequest* request,
                                ::furbbs::CreateFursonaResponse* response) override;

    ::trpc::Status GetUserFursonas(::trpc::ServerContextPtr context,
                                  const ::furbbs::GetUserFursonasRequest* request,
                                  ::furbbs::GetUserFursonasResponse* response) override;

    ::trpc::Status CreateCommissionType(::trpc::ServerContextPtr context,
                                       const ::furbbs::CreateCommissionTypeRequest* request,
                                       ::furbbs::CreateCommissionTypeResponse* response) override;

    ::trpc::Status GetUserCommissions(::trpc::ServerContextPtr context,
                                     const ::furbbs::GetUserCommissionsRequest* request,
                                     ::furbbs::GetUserCommissionsResponse* response) override;

    ::trpc::Status UpdateCommissionStatus(::trpc::ServerContextPtr context,
                                         const ::furbbs::UpdateCommissionStatusRequest* request,
                                         ::furbbs::UpdateCommissionStatusResponse* response) override;

    ::trpc::Status CreateOrder(::trpc::ServerContextPtr context,
                              const ::furbbs::CreateOrderRequest* request,
                              ::furbbs::CreateOrderResponse* response) override;

    ::trpc::Status GetMyOrders(::trpc::ServerContextPtr context,
                              const ::furbbs::GetMyOrdersRequest* request,
                              ::furbbs::GetMyOrdersResponse* response) override;

    ::trpc::Status CreateEvent(::trpc::ServerContextPtr context,
                              const ::furbbs::CreateEventRequest* request,
                              ::furbbs::CreateEventResponse* response) override;

    ::trpc::Status ListEvents(::trpc::ServerContextPtr context,
                             const ::furbbs::ListEventsRequest* request,
                             ::furbbs::ListEventsResponse* response) override;

    ::trpc::Status JoinEvent(::trpc::ServerContextPtr context,
                            const ::furbbs::JoinEventRequest* request,
                            ::furbbs::JoinEventResponse* response) override;

    ::trpc::Status SendMessage(::trpc::ServerContextPtr context,
                              const ::furbbs::SendMessageRequest* request,
                              ::furbbs::SendMessageResponse* response) override;

    ::trpc::Status GetMessages(::trpc::ServerContextPtr context,
                              const ::furbbs::GetMessagesRequest* request,
                              ::furbbs::GetMessagesResponse* response) override;

    ::trpc::Status HealthCheck(::trpc::ServerContextPtr context,
                              const ::furbbs::HealthCheckRequest* request,
                              ::furbbs::HealthCheckResponse* response) override;

    ::trpc::Status GetServerStats(::trpc::ServerContextPtr context,
                                 const ::furbbs::GetServerStatsRequest* request,
                                 ::furbbs::GetServerStatsResponse* response) override;

    ::trpc::Status GrantPermission(::trpc::ServerContextPtr context,
                                  const ::furbbs::GrantPermissionRequest* request,
                                  ::furbbs::GrantPermissionResponse* response) override;

    ::trpc::Status RevokePermission(::trpc::ServerContextPtr context,
                                   const ::furbbs::RevokePermissionRequest* request,
                                   ::furbbs::RevokePermissionResponse* response) override;

    ::trpc::Status ListUserPermissions(::trpc::ServerContextPtr context,
                                      const ::furbbs::ListUserPermissionsRequest* request,
                                      ::furbbs::ListUserPermissionsResponse* response) override;

    ::trpc::Status ModeratePost(::trpc::ServerContextPtr context,
                               const ::furbbs::ModeratePostRequest* request,
                               ::furbbs::ModeratePostResponse* response) override;

    ::trpc::Status PinPost(::trpc::ServerContextPtr context,
                          const ::furbbs::PinPostRequest* request,
                          ::furbbs::PinPostResponse* response) override;

    ::trpc::Status LockPost(::trpc::ServerContextPtr context,
                           const ::furbbs::LockPostRequest* request,
                           ::furbbs::LockPostResponse* response) override;

    ::trpc::Status GetNotifications(::trpc::ServerContextPtr context,
                                   const ::furbbs::GetNotificationsRequest* request,
                                   ::furbbs::GetNotificationsResponse* response) override;

    ::trpc::Status MarkNotificationRead(::trpc::ServerContextPtr context,
                                       const ::furbbs::MarkNotificationReadRequest* request,
                                       ::furbbs::MarkNotificationReadResponse* response) override;

    ::trpc::Status AddFavorite(::trpc::ServerContextPtr context,
                              const ::furbbs::AddFavoriteRequest* request,
                              ::furbbs::AddFavoriteResponse* response) override;

    ::trpc::Status RemoveFavorite(::trpc::ServerContextPtr context,
                                 const ::furbbs::RemoveFavoriteRequest* request,
                                 ::furbbs::RemoveFavoriteResponse* response) override;

    ::trpc::Status GetFavorites(::trpc::ServerContextPtr context,
                               const ::furbbs::GetFavoritesRequest* request,
                               ::furbbs::GetFavoritesResponse* response) override;

    ::trpc::Status CreateReport(::trpc::ServerContextPtr context,
                               const ::furbbs::CreateReportRequest* request,
                               ::furbbs::CreateReportResponse* response) override;

    ::trpc::Status GetReports(::trpc::ServerContextPtr context,
                             const ::furbbs::GetReportsRequest* request,
                             ::furbbs::GetReportsResponse* response) override;

    ::trpc::Status HandleReport(::trpc::ServerContextPtr context,
                               const ::furbbs::HandleReportRequest* request,
                               ::furbbs::HandleReportResponse* response) override;

    ::trpc::Status Search(::trpc::ServerContextPtr context,
                         const ::furbbs::SearchRequest* request,
                         ::furbbs::SearchResponse* response) override;

    ::trpc::Status BlockUser(::trpc::ServerContextPtr context,
                            const ::furbbs::BlockUserRequest* request,
                            ::furbbs::BlockUserResponse* response) override;

    ::trpc::Status UnblockUser(::trpc::ServerContextPtr context,
                              const ::furbbs::UnblockUserRequest* request,
                              ::furbbs::UnblockUserResponse* response) override;

    ::trpc::Status GetBlockedUsers(::trpc::ServerContextPtr context,
                                  const ::furbbs::GetBlockedUsersRequest* request,
                                  ::furbbs::GetBlockedUsersResponse* response) override;

    ::trpc::Status SaveDraft(::trpc::ServerContextPtr context,
                            const ::furbbs::SaveDraftRequest* request,
                            ::furbbs::SaveDraftResponse* response) override;

    ::trpc::Status GetDrafts(::trpc::ServerContextPtr context,
                            const ::furbbs::GetDraftsRequest* request,
                            ::furbbs::GetDraftsResponse* response) override;

    ::trpc::Status DeleteDraft(::trpc::ServerContextPtr context,
                              const ::furbbs::DeleteDraftRequest* request,
                              ::furbbs::DeleteDraftResponse* response) override;

    ::trpc::Status ExportUserData(::trpc::ServerContextPtr context,
                                   const ::furbbs::ExportDataRequest* request,
                                   ::furbbs::ExportDataResponse* response) override;

    ::trpc::Status GetUserLevel(::trpc::ServerContextPtr context,
                               const ::furbbs::GetUserLevelRequest* request,
                               ::furbbs::GetUserLevelResponse* response) override;

    ::trpc::Status DailyCheckIn(::trpc::ServerContextPtr context,
                               const ::furbbs::DailyCheckInRequest* request,
                               ::furbbs::DailyCheckInResponse* response) override;

    ::trpc::Status GetCheckInStatus(::trpc::ServerContextPtr context,
                                    const ::furbbs::GetCheckInStatusRequest* request,
                                    ::furbbs::GetCheckInStatusResponse* response) override;

    ::trpc::Status SendFriendRequest(::trpc::ServerContextPtr context,
                                    const ::furbbs::SendFriendRequestRequest* request,
                                    ::furbbs::SendFriendRequestResponse* response) override;

    ::trpc::Status AcceptFriendRequest(::trpc::ServerContextPtr context,
                                      const ::furbbs::AcceptFriendRequestRequest* request,
                                      ::furbbs::AcceptFriendRequestResponse* response) override;

    ::trpc::Status GetFriendRequests(::trpc::ServerContextPtr context,
                                    const ::furbbs::GetFriendRequestsRequest* request,
                                    ::furbbs::GetFriendRequestsResponse* response) override;

    ::trpc::Status GetFriends(::trpc::ServerContextPtr context,
                             const ::furbbs::GetFriendsRequest* request,
                             ::furbbs::GetFriendsResponse* response) override;

    ::trpc::Status RemoveFriend(::trpc::ServerContextPtr context,
                               const ::furbbs::RemoveFriendRequest* request,
                               ::furbbs::RemoveFriendResponse* response) override;

    ::trpc::Status CreateOpenApp(::trpc::ServerContextPtr context,
                                const ::furbbs::CreateAppRequest* request,
                                ::furbbs::CreateAppResponse* response) override;

    ::trpc::Status GetOpenApp(::trpc::ServerContextPtr context,
                             const ::furbbs::GetAppRequest* request,
                             ::furbbs::GetAppResponse* response) override;

    ::trpc::Status ListOpenApps(::trpc::ServerContextPtr context,
                               const ::furbbs::ListAppsRequest* request,
                               ::furbbs::ListAppsResponse* response) override;

    ::trpc::Status RegenerateAppSecret(::trpc::ServerContextPtr context,
                                      const ::furbbs::RegenerateSecretRequest* request,
                                      ::furbbs::RegenerateSecretResponse* response) override;

    ::trpc::Status GetApiStats(::trpc::ServerContextPtr context,
                              const ::furbbs::GetApiStatsRequest* request,
                              ::furbbs::GetApiStatsResponse* response) override;

    ::trpc::Status CreateWebhook(::trpc::ServerContextPtr context,
                                const ::furbbs::CreateWebhookRequest* request,
                                ::furbbs::CreateWebhookResponse* response) override;

    ::trpc::Status GetAnnouncements(::trpc::ServerContextPtr context,
                                   const ::furbbs::GetAnnouncementsRequest* request,
                                   ::furbbs::GetAnnouncementsResponse* response) override;

    ::trpc::Status GetBanners(::trpc::ServerContextPtr context,
                             const ::furbbs::GetBannersResponse* request,
                             ::furbbs::GetBannersResponse* response) override;

    ::trpc::Status GetHotTopics(::trpc::ServerContextPtr context,
                               const ::furbbs::GetHotTopicsResponse* request,
                               ::furbbs::GetHotTopicsResponse* response) override;

    ::trpc::Status GetRankings(::trpc::ServerContextPtr context,
                              const ::furbbs::GetRankingsRequest* request,
                              ::furbbs::GetRankingsResponse* response) override;

    ::trpc::Status UploadFile(::trpc::ServerContextPtr context,
                             const ::furbbs::UploadFileRequest* request,
                             ::furbbs::UploadFileResponse* response) override;

    ::trpc::Status GetFile(::trpc::ServerContextPtr context,
                          const ::furbbs::GetFileRequest* request,
                          ::furbbs::GetFileResponse* response) override;

    ::trpc::Status DeleteFile(::trpc::ServerContextPtr context,
                             const ::furbbs::DeleteFileRequest* request,
                             ::furbbs::DeleteFileResponse* response) override;

    ::trpc::Status SendEmailCode(::trpc::ServerContextPtr context,
                                const ::furbbs::SendEmailCodeRequest* request,
                                ::furbbs::SendEmailCodeResponse* response) override;

    ::trpc::Status VerifyEmailCode(::trpc::ServerContextPtr context,
                                  const ::furbbs::VerifyEmailCodeRequest* request,
                                  ::furbbs::VerifyEmailCodeResponse* response) override;

    ::trpc::Status GetCacheStats(::trpc::ServerContextPtr context,
                                const ::furbbs::GetSystemMetricsResponse* request,
                                ::furbbs::GetCacheStatsResponse* response) override;

    ::trpc::Status GetSystemMetrics(::trpc::ServerContextPtr context,
                                   const ::furbbs::GetSystemMetricsResponse* request,
                                   ::furbbs::GetSystemMetricsResponse* response) override;

    ::trpc::Status GetForumSections(::trpc::ServerContextPtr context,
                                   const ::furbbs::GetForumSectionsRequest* request,
                                   ::furbbs::GetForumSectionsResponse* response) override;

    ::trpc::Status CreateForumSection(::trpc::ServerContextPtr context,
                                     const ::furbbs::CreateForumSectionRequest* request,
                                     ::furbbs::CreateForumSectionResponse* response) override;

    ::trpc::Status GetTags(::trpc::ServerContextPtr context,
                          const ::furbbs::GetTagsRequest* request,
                          ::furbbs::GetTagsResponse* response) override;

    ::trpc::Status SearchPosts(::trpc::ServerContextPtr context,
                              const ::furbbs::SearchPostsRequest* request,
                              ::furbbs::SearchPostsResponse* response) override;

    ::trpc::Status GetUserBadges(::trpc::ServerContextPtr context,
                                const ::furbbs::GetUserBadgesRequest* request,
                                ::furbbs::GetUserBadgesResponse* response) override;

    ::trpc::Status GetAllBadges(::trpc::ServerContextPtr context,
                               const ::furbbs::GetSystemMetricsResponse* request,
                               ::furbbs::GetAllBadgesResponse* response) override;

    ::trpc::Status UpdateOnlineStatus(::trpc::ServerContextPtr context,
                                     const ::furbbs::UpdateOnlineStatusRequest* request,
                                     ::furbbs::UpdateOnlineStatusResponse* response) override;

    ::trpc::Status GetOnlineUsers(::trpc::ServerContextPtr context,
                                 const ::furbbs::GetOnlineUsersRequest* request,
                                 ::furbbs::GetOnlineUsersResponse* response) override;

    ::trpc::Status GetReadingHistory(::trpc::ServerContextPtr context,
                                    const ::furbbs::GetReadingHistoryRequest* request,
                                    ::furbbs::GetReadingHistoryResponse* response) override;

    ::trpc::Status GetForumStats(::trpc::ServerContextPtr context,
                                const ::furbbs::GetSystemMetricsResponse* request,
                                ::furbbs::GetForumStatsResponse* response) override;

    ::trpc::Status CreatePoll(::trpc::ServerContextPtr context,
                             const ::furbbs::CreatePollRequest* request,
                             ::furbbs::CreatePollResponse* response) override;

    ::trpc::Status VotePoll(::trpc::ServerContextPtr context,
                           const ::furbbs::VotePollRequest* request,
                           ::furbbs::VotePollResponse* response) override;

    ::trpc::Status GetPoll(::trpc::ServerContextPtr context,
                          const ::furbbs::GetPollRequest* request,
                          ::furbbs::GetPollResponse* response) override;

    ::trpc::Status GetGiftList(::trpc::ServerContextPtr context,
                              const ::furbbs::GetSystemMetricsResponse* request,
                              ::furbbs::GetGiftListResponse* response) override;

    ::trpc::Status SendGift(::trpc::ServerContextPtr context,
                           const ::furbbs::SendGiftRequest* request,
                           ::furbbs::SendGiftResponse* response) override;

    ::trpc::Status GetUserGifts(::trpc::ServerContextPtr context,
                               const ::furbbs::GetUserGiftsRequest* request,
                               ::furbbs::GetUserGiftsResponse* response) override;

    ::trpc::Status BanUser(::trpc::ServerContextPtr context,
                          const ::furbbs::BanUserRequest* request,
                          ::furbbs::BanUserResponse* response) override;

    ::trpc::Status UnbanUser(::trpc::ServerContextPtr context,
                            const ::furbbs::UnbanUserRequest* request,
                            ::furbbs::UnbanUserResponse* response) override;

    ::trpc::Status GetUserBan(::trpc::ServerContextPtr context,
                            const ::furbbs::GetUserBanRequest* request,
                            ::furbbs::GetUserBanResponse* response) override;

    ::trpc::Status GetModeratorLogs(::trpc::ServerContextPtr context,
                                  const ::furbbs::GetModeratorLogsRequest* request,
                                  ::furbbs::GetModeratorLogsResponse* response) override;

    ::trpc::Status ManagePost(::trpc::ServerContextPtr context,
                            const ::furbbs::ManagePostRequest* request,
                            ::furbbs::ManagePostResponse* response) override;

    ::trpc::Status GetShopItems(::trpc::ServerContextPtr context,
                                const ::furbbs::GetSystemMetricsResponse* request,
                                ::furbbs::GetShopItemsResponse* response) override;

    ::trpc::Status PurchaseItem(::trpc::ServerContextPtr context,
                               const ::furbbs::PurchaseItemRequest* request,
                               ::furbbs::PurchaseItemResponse* response) override;

    ::trpc::Status GetUserInventory(::trpc::ServerContextPtr context,
                                   const ::furbbs::GetUserInventoryRequest* request,
                                   ::furbbs::GetUserInventoryResponse* response) override;

    ::trpc::Status UpdateProfile(::trpc::ServerContextPtr context,
                                const ::furbbs::UpdateProfileRequest* request,
                                ::furbbs::UpdateProfileResponse* response) override;

    ::trpc::Status CreateFavoriteFolder(::trpc::ServerContextPtr context,
                                        const ::furbbs::CreateFavoriteFolderRequest* request,
                                        ::furbbs::CreateFavoriteFolderResponse* response) override;

    ::trpc::Status GetFavoriteFolders(::trpc::ServerContextPtr context,
                                         const ::furbbs::GetFavoriteFoldersRequest* request,
                                         ::furbbs::GetFavoriteFoldersResponse* response) override;

    ::trpc::Status SubscribeUser(::trpc::ServerContextPtr context,
                                 const ::furbbs::SubscribeUserRequest* request,
                                 ::furbbs::SubscribeUserResponse* response) override;

    ::trpc::Status GetDailyTasks(::trpc::ServerContextPtr context,
                                   const ::furbbs::GetDailyTasksRequest* request,
                                   ::furbbs::GetDailyTasksResponse* response) override;

    ::trpc::Status SendPrivateMessage(::trpc::ServerContextPtr context,
                                      const ::furbbs::SendPrivateMessageRequest* request,
                                      ::furbbs::SendPrivateMessageResponse* response) override;

    ::trpc::Status GetPrivateMessages(::trpc::ServerContextPtr context,
                                        const ::furbbs::GetPrivateMessagesRequest* request,
                                        ::furbbs::GetPrivateMessagesResponse* response) override;

    ::trpc::Status GetMessageConversations(::trpc::ServerContextPtr context,
                                           const ::furbbs::GetMessageConversationsRequest* request,
                                           ::furbbs::GetMessageConversationsResponse* response) override;

    ::trpc::Status MarkMessageRead(::trpc::ServerContextPtr context,
                                   const ::furbbs::MarkMessageReadRequest* request,
                                   ::furbbs::MarkMessageReadResponse* response) override;

    ::trpc::Status GenerateInviteCode(::trpc::ServerContextPtr context,
                                       const ::furbbs::GenerateInviteCodeRequest* request,
                                       ::furbbs::GenerateInviteCodeResponse* response) override;

    ::trpc::Status GetMyInviteCodes(::trpc::ServerContextPtr context,
                                    const ::furbbs::GetMyInviteCodesRequest* request,
                                    ::furbbs::GetMyInviteCodesResponse* response) override;

    ::trpc::Status GetModerationQueue(::trpc::ServerContextPtr context,
                                      const ::furbbs::GetModerationQueueRequest* request,
                                      ::furbbs::GetModerationQueueResponse* response) override;

    ::trpc::Status ReviewPost(::trpc::ServerContextPtr context,
                              const ::furbbs::ReviewPostRequest* request,
                              ::furbbs::ReviewPostResponse* response) override;

    ::trpc::Status GetUserActivity(::trpc::ServerContextPtr context,
                                   const ::furbbs::GetUserActivityRequest* request,
                                   ::furbbs::GetUserActivityResponse* response) override;

    ::trpc::Status GetActivityRanking(::trpc::ServerContextPtr context,
                                      const ::furbbs::GetActivityRankingRequest* request,
                                      ::furbbs::GetActivityRankingResponse* response) override;

    ::trpc::Status DoLuckyDraw(::trpc::ServerContextPtr context,
                               const ::furbbs::DoLuckyDrawRequest* request,
                               ::furbbs::DoLuckyDrawResponse* response) override;

    ::trpc::Status UseCheckinCard(::trpc::ServerContextPtr context,
                                  const ::furbbs::UseCheckinCardRequest* request,
                                  ::furbbs::UseCheckinCardResponse* response) override;

    ::trpc::Status GetFAQList(::trpc::ServerContextPtr context,
                              const ::furbbs::GetFAQListRequest* request,
                              ::furbbs::GetFAQListResponse* response) override;

    ::trpc::Status ManageFAQ(::trpc::ServerContextPtr context,
                            const ::furbbs::ManageFAQRequest* request,
                            ::furbbs::ManageFAQResponse* response) override;

    ::trpc::Status GetHelpArticles(::trpc::ServerContextPtr context,
                                   const ::furbbs::GetHelpArticlesRequest* request,
                                   ::furbbs::GetHelpArticlesResponse* response) override;

    ::trpc::Status ManageHelpArticle(::trpc::ServerContextPtr context,
                                     const ::furbbs::ManageHelpArticleRequest* request,
                                     ::furbbs::ManageHelpArticleResponse* response) override;

    ::trpc::Status SubmitFeedback(::trpc::ServerContextPtr context,
                                  const ::furbbs::SubmitFeedbackRequest* request,
                                  ::furbbs::SubmitFeedbackResponse* response) override;

    ::trpc::Status GetFeedbackList(::trpc::ServerContextPtr context,
                                   const ::furbbs::GetFeedbackListRequest* request,
                                   ::furbbs::GetFeedbackListResponse* response) override;

    ::trpc::Status ReplyFeedback(::trpc::ServerContextPtr context,
                                const ::furbbs::ReplyFeedbackRequest* request,
                                ::furbbs::ReplyFeedbackResponse* response) override;

    ::trpc::Status GetAdvertisements(::trpc::ServerContextPtr context,
                                     const ::furbbs::GetAdvertisementsRequest* request,
                                     ::furbbs::GetAdvertisementsResponse* response) override;

    ::trpc::Status ManageAdvertisement(::trpc::ServerContextPtr context,
                                       const ::furbbs::ManageAdvertisementRequest* request,
                                       ::furbbs::ManageAdvertisementResponse* response) override;

    ::trpc::Status GetFriendLinks(::trpc::ServerContextPtr context,
                                  const ::furbbs::GetSystemMetricsResponse* request,
                                  ::furbbs::GetFriendLinksResponse* response) override;

    ::trpc::Status ManageFriendLink(::trpc::ServerContextPtr context,
                                    const ::furbbs::ManageFriendLinkRequest* request,
                                    ::furbbs::ManageFriendLinkResponse* response) override;

    ::trpc::Status GetStatistics(::trpc::ServerContextPtr context,
                                 const ::furbbs::GetSystemMetricsResponse* request,
                                 ::furbbs::GetStatisticsResponse* response) override;

    ::trpc::Status VerifyCaptcha(::trpc::ServerContextPtr context,
                                 const ::furbbs::VerifyCaptchaRequest* request,
                                 ::furbbs::VerifyCaptchaResponse* response) override;

    ::trpc::Status SubmitReport(::trpc::ServerContextPtr context,
                                const ::furbbs::SubmitReportRequest* request,
                                ::furbbs::SubmitReportResponse* response) override;

    ::trpc::Status GetReportList(::trpc::ServerContextPtr context,
                                 const ::furbbs::GetReportListRequest* request,
                                 ::furbbs::GetReportListResponse* response) override;

    ::trpc::Status HandleReport(::trpc::ServerContextPtr context,
                                const ::furbbs::HandleReportRequest* request,
                                ::furbbs::HandleReportResponse* response) override;

    ::trpc::Status GetModerationLog(::trpc::ServerContextPtr context,
                                    const ::furbbs::GetModerationLogRequest* request,
                                    ::furbbs::GetModerationLogResponse* response) override;

    ::trpc::Status GetMembershipPlans(::trpc::ServerContextPtr context,
                                      const ::furbbs::GetSystemMetricsResponse* request,
                                      ::furbbs::GetMembershipPlansResponse* response) override;

    ::trpc::Status GetUserMembership(::trpc::ServerContextPtr context,
                                     const ::furbbs::GetUserMembershipRequest* request,
                                     ::furbbs::GetUserMembershipResponse* response) override;

    ::trpc::Status SubscribeMembership(::trpc::ServerContextPtr context,
                                       const ::furbbs::SubscribeMembershipRequest* request,
                                       ::furbbs::SubscribeMembershipResponse* response) override;

    ::trpc::Status GetAchievements(::trpc::ServerContextPtr context,
                                   const ::furbbs::GetSystemMetricsResponse* request,
                                   ::furbbs::GetAchievementsResponse* response) override;

    ::trpc::Status GetUserAchievements(::trpc::ServerContextPtr context,
                                       const ::furbbs::GetUserAchievementsRequest* request,
                                       ::furbbs::GetUserAchievementsResponse* response) override;

    ::trpc::Status GenerateCards(::trpc::ServerContextPtr context,
                                 const ::furbbs::GenerateCardsRequest* request,
                                 ::furbbs::GenerateCardsResponse* response) override;

    ::trpc::Status RedeemCard(::trpc::ServerContextPtr context,
                               const ::furbbs::RedeemCardRequest* request,
                               ::furbbs::RedeemCardResponse* response) override;

    ::trpc::Status GetCardList(::trpc::ServerContextPtr context,
                               const ::furbbs::GetCardListRequest* request,
                               ::furbbs::GetCardListResponse* response) override;

    ::trpc::Status GetUserTitles(::trpc::ServerContextPtr context,
                                 const ::furbbs::GetSystemMetricsResponse* request,
                                 ::furbbs::GetUserTitlesResponse* response) override;

    ::trpc::Status SetActiveTitle(::trpc::ServerContextPtr context,
                                  const ::furbbs::SetActiveTitleRequest* request,
                                  ::furbbs::SetActiveTitleResponse* response) override;

    ::trpc::Status GetAvatarFrames(::trpc::ServerContextPtr context,
                                   const ::furbbs::GetSystemMetricsResponse* request,
                                   ::furbbs::GetAvatarFramesResponse* response) override;

    ::trpc::Status SetActiveFrame(::trpc::ServerContextPtr context,
                                  const ::furbbs::SetActiveFrameRequest* request,
                                  ::furbbs::SetActiveFrameResponse* response) override;

    ::trpc::Status GetCustomizationItems(::trpc::ServerContextPtr context,
                                         const ::furbbs::GetSystemMetricsResponse* request,
                                         ::furbbs::GetCustomizationItemsResponse* response) override;

    ::trpc::Status SetCustomization(::trpc::ServerContextPtr context,
                                    const ::furbbs::SetCustomizationRequest* request,
                                    ::furbbs::SetCustomizationResponse* response) override;

    ::trpc::Status GetShopItems(::trpc::ServerContextPtr context,
                                const ::furbbs::GetShopItemsRequest* request,
                                ::furbbs::GetShopItemsResponse* response) override;

    ::trpc::Status PurchaseItem(::trpc::ServerContextPtr context,
                                 const ::furbbs::PurchaseItemRequest* request,
                                 ::furbbs::PurchaseItemResponse* response) override;

    ::trpc::Status GetDailyTasks(::trpc::ServerContextPtr context,
                                  const ::furbbs::GetDailyTasksRequest* request,
                                  ::furbbs::GetDailyTasksResponse* response) override;

    ::trpc::Status ClaimTaskReward(::trpc::ServerContextPtr context,
                                    const ::furbbs::ClaimTaskRewardRequest* request,
                                    ::furbbs::ClaimTaskRewardResponse* response) override;

    ::trpc::Status CheckIn(::trpc::ServerContextPtr context,
                           const ::furbbs::GetDailyTasksRequest* request,
                           ::furbbs::CheckInResponse* response) override;

    ::trpc::Status GetCheckInStatus(::trpc::ServerContextPtr context,
                                      const ::furbbs::GetCheckInStatusRequest* request,
                                      ::furbbs::GetCheckInStatusResponse* response) override;

    ::trpc::Status SetEssence(::trpc::ServerContextPtr context,
                               const ::furbbs::SetEssenceRequest* request,
                               ::furbbs::SetEssenceResponse* response) override;

    ::trpc::Status SetSticky(::trpc::ServerContextPtr context,
                              const ::furbbs::SetStickyRequest* request,
                              ::furbbs::SetStickyResponse* response) override;

    ::trpc::Status FollowUser(::trpc::ServerContextPtr context,
                               const ::furbbs::FollowUserRequest* request,
                               ::furbbs::FollowUserResponse* response) override;

    ::trpc::Status GetFollowing(::trpc::ServerContextPtr context,
                                const ::furbbs::GetFollowListRequest* request,
                                ::furbbs::GetFollowListResponse* response) override;

    ::trpc::Status GetFollowers(::trpc::ServerContextPtr context,
                                const ::furbbs::GetFollowListRequest* request,
                                ::furbbs::GetFollowListResponse* response) override;

    ::trpc::Status GetFriendCircle(::trpc::ServerContextPtr context,
                                   const ::furbbs::GetFriendCircleRequest* request,
                                   ::furbbs::GetFriendCircleResponse* response) override;

    ::trpc::Status FavoriteFursona(::trpc::ServerContextPtr context,
                                    const ::furbbs::FavoriteFursonaRequest* request,
                                    ::furbbs::FavoriteFursonaResponse* response) override;

    ::trpc::Status GetFavoriteFursonas(::trpc::ServerContextPtr context,
                                       const ::furbbs::GetFavoriteFursonasRequest* request,
                                       ::furbbs::GetFavoriteFursonasResponse* response) override;

    ::trpc::Status GetGiftList(::trpc::ServerContextPtr context,
                               const ::furbbs::GetGiftListRequest* request,
                               ::furbbs::GetGiftListResponse* response) override;

    ::trpc::Status SendGift(::trpc::ServerContextPtr context,
                            const ::furbbs::SendGiftRequest* request,
                            ::furbbs::SendGiftResponse* response) override;

    ::trpc::Status GetUserGifts(::trpc::ServerContextPtr context,
                                const ::furbbs::GetUserGiftsRequest* request,
                                ::furbbs::GetUserGiftsResponse* response) override;

    ::trpc::Status RateArtist(::trpc::ServerContextPtr context,
                              const ::furbbs::RateArtistRequest* request,
                              ::furbbs::RateArtistResponse* response) override;

    ::trpc::Status GetArtistReviews(::trpc::ServerContextPtr context,
                                    const ::furbbs::GetArtistReviewsRequest* request,
                                    ::furbbs::GetArtistReviewsResponse* response) override;

    ::trpc::Status ManageArtistBlacklist(::trpc::ServerContextPtr context,
                                          const ::furbbs::ManageArtistBlacklistRequest* request,
                                          ::furbbs::ManageArtistBlacklistResponse* response) override;

    ::trpc::Status CreateQuestionBox(::trpc::ServerContextPtr context,
                                     const ::furbbs::CreateQuestionBoxRequest* request,
                                     ::furbbs::CreateQuestionBoxResponse* response) override;

    ::trpc::Status GetQuestionBoxes(::trpc::ServerContextPtr context,
                                    const ::furbbs::GetQuestionBoxesRequest* request,
                                    ::furbbs::GetQuestionBoxesResponse* response) override;

    ::trpc::Status AskQuestion(::trpc::ServerContextPtr context,
                               const ::furbbs::AskQuestionRequest* request,
                               ::furbbs::AskQuestionResponse* response) override;

    ::trpc::Status AnswerQuestion(::trpc::ServerContextPtr context,
                                  const ::furbbs::AnswerQuestionRequest* request,
                                  ::furbbs::AnswerQuestionResponse* response) override;

    ::trpc::Status GetQuestions(::trpc::ServerContextPtr context,
                                const ::furbbs::GetQuestionsRequest* request,
                                ::furbbs::GetQuestionsResponse* response) override;

    ::trpc::Status CreateGroup(::trpc::ServerContextPtr context,
                               const ::furbbs::CreateGroupRequest* request,
                               ::furbbs::CreateGroupResponse* response) override;

    ::trpc::Status GetGroupList(::trpc::ServerContextPtr context,
                                 const ::furbbs::GetGroupListRequest* request,
                                 ::furbbs::GetGroupListResponse* response) override;

    ::trpc::Status GetGroupDetail(::trpc::ServerContextPtr context,
                                  const ::furbbs::GetGroupDetailRequest* request,
                                  ::furbbs::GetGroupDetailResponse* response) override;

    ::trpc::Status ManageGroupMember(::trpc::ServerContextPtr context,
                                     const ::furbbs::ManageGroupMemberRequest* request,
                                     ::furbbs::ManageGroupMemberResponse* response) override;

    ::trpc::Status CreateGroupPost(::trpc::ServerContextPtr context,
                                   const ::furbbs::GroupPostRequest* request,
                                   ::furbbs::GroupPostResponse* response) override;

    ::trpc::Status RegisterEvent(::trpc::ServerContextPtr context,
                                 const ::furbbs::RegisterEventRequest* request,
                                 ::furbbs::RegisterEventResponse* response) override;

    ::trpc::Status GetEventRegistrations(::trpc::ServerContextPtr context,
                                          const ::furbbs::GetEventRegistrationsRequest* request,
                                          ::furbbs::GetEventRegistrationsResponse* response) override;

    ::trpc::Status ManageEventRegistration(::trpc::ServerContextPtr context,
                                            const ::furbbs::ManageEventRegistrationRequest* request,
                                            ::furbbs::ManageEventRegistrationResponse* response) override;

    ::trpc::Status GetMentions(::trpc::ServerContextPtr context,
                               const ::furbbs::GetMentionsRequest* request,
                               ::furbbs::GetMentionsResponse* response) override;

    ::trpc::Status MarkMentionRead(::trpc::ServerContextPtr context,
                                    const ::furbbs::MarkMentionReadRequest* request,
                                    ::furbbs::MarkMentionReadResponse* response) override;

    ::trpc::Status FavoritePost(::trpc::ServerContextPtr context,
                                 const ::furbbs::FavoritePostRequest* request,
                                 ::furbbs::FavoritePostResponse* response) override;

    ::trpc::Status GetFavoritePosts(::trpc::ServerContextPtr context,
                                     const ::furbbs::GetFavoritePostsRequest* request,
                                     ::furbbs::GetFavoritePostsResponse* response) override;

    ::trpc::Status SaveDraft(::trpc::ServerContextPtr context,
                              const ::furbbs::SaveDraftRequest* request,
                              ::furbbs::SaveDraftResponse* response) override;

    ::trpc::Status GetDrafts(::trpc::ServerContextPtr context,
                             const ::furbbs::GetDraftsRequest* request,
                             ::furbbs::GetDraftsResponse* response) override;

    ::trpc::Status DeleteDraft(::trpc::ServerContextPtr context,
                                const ::furbbs::DeleteDraftRequest* request,
                                ::furbbs::DeleteDraftResponse* response) override;
};

} // namespace furbbs::service

#endif // FURBBS_SERVICE_FURBBS_SERVICE_H
