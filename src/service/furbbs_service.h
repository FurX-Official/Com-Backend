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
};

} // namespace furbbs::service

#endif // FURBBS_SERVICE_FURBBS_SERVICE_H
