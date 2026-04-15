#include "furbbs_service.h"
#include <spdlog/spdlog.h>
#include "../db/database.h"
#include "../auth/casdoor_auth.h"

namespace furbbs::service {

::trpc::Status FurBBSServiceImpl::GetUser(::trpc::ServerContextPtr context,
                                         const ::furbbs::GetUserRequest* request,
                                         ::furbbs::GetUserResponse* response) {
    try {
        auto user_opt = auth::CasdoorAuth::Instance().GetUserInfo(request->user_id());
        if (!user_opt) {
            response->set_code(404);
            response->set_message("User not found");
            return ::trpc::kSuccStatus;
        }

        const auto& user = *user_opt;
        auto* user_info = response->mutable_user();
        user_info->set_id(user.id);
        user_info->set_name(user.name);
        user_info->set_avatar(user.avatar);
        user_info->set_email(user.email);
        user_info->set_display_name(user.display_name);
        user_info->set_bio(user.bio);

        response->set_code(200);
        response->set_message("Success");
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::CreatePost(::trpc::ServerContextPtr context,
                                            const ::furbbs::CreatePostRequest* request,
                                            ::furbbs::CreatePostResponse* response) {
    try {
        auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(request->access_token());
        if (!user_opt) {
            response->set_code(401);
            response->set_message("Invalid token");
            return ::trpc::kSuccStatus;
        }

        int64_t post_id = db::Database::Instance().Execute([&](pqxx::work& txn) {
            auto result = txn.exec_params(R"(
                INSERT INTO posts (title, content, author_id, category_id)
                VALUES ($1, $2, $3, $4)
                RETURNING id
            )", request->title(), request->content(), user_opt->id, request->category_id());

            int64_t id = result[0][0].as<int64_t>();

            for (const auto& tag_id : request->tag_ids()) {
                txn.exec_params(R"(
                    INSERT INTO post_tags (post_id, tag_id) VALUES ($1, $2)
                )", id, tag_id);
            }

            for (const auto& image : request->images()) {
                txn.exec_params(R"(
                    INSERT INTO post_images (post_id, url) VALUES ($1, $2)
                )", id, image);
            }

            txn.exec_params(R"(
                UPDATE categories SET post_count = post_count + 1 WHERE id = $1
            )", request->category_id());

            return id;
        });

        response->set_code(200);
        response->set_message("Post created successfully");
        response->set_post_id(post_id);
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::GetPost(::trpc::ServerContextPtr context,
                                         const ::furbbs::GetPostRequest* request,
                                         ::furbbs::GetPostResponse* response) {
    try {
        db::Database::Instance().Execute([&](pqxx::work& txn) {
            auto result = txn.exec_params(R"(
                SELECT p.id, p.title, p.content, p.author_id, p.category_id,
                       p.view_count, p.like_count, p.comment_count,
                       p.is_pinned, p.is_locked, p.created_at, p.updated_at,
                       u.name, u.display_name, u.avatar, c.name, c.icon
                FROM posts p
                JOIN users u ON p.author_id = u.id
                JOIN categories c ON p.category_id = c.id
                WHERE p.id = $1
            )", request->post_id());

            if (result.empty()) {
                response->set_code(404);
                response->set_message("Post not found");
                return;
            }

            const auto& row = result[0];
            auto* post = response->mutable_post();
            post->set_id(row[0].as<int64_t>());
            post->set_title(row[1].as<std::string>());
            post->set_content(row[2].as<std::string>());
            post->set_author_id(row[3].as<std::string>());
            post->set_category_id(row[4].as<int64_t>());
            post->set_view_count(row[5].as<int32_t>());
            post->set_like_count(row[6].as<int32_t>());
            post->set_comment_count(row[7].as<int32_t>());
            post->set_is_pinned(row[8].as<bool>());
            post->set_is_locked(row[9].as<bool>());
            post->set_created_at(row[10].as<int64_t>());
            post->set_updated_at(row[11].as<int64_t>());

            post->mutable_author()->set_id(row[3].as<std::string>());
            post->mutable_author()->set_name(row[12].as<std::string>());
            post->mutable_author()->set_display_name(row[13].as<std::string>());
            post->mutable_author()->set_avatar(row[14].as<std::string>());

            post->mutable_category()->set_id(row[4].as<int64_t>());
            post->mutable_category()->set_name(row[15].as<std::string>());
            post->mutable_category()->set_icon(row[16].as<std::string>());

            auto img_result = txn.exec_params(R"(
                SELECT url FROM post_images WHERE post_id = $1
            )", request->post_id());
            for (const auto& img_row : img_result) {
                post->add_images(img_row[0].as<std::string>());
            }

            auto tag_result = txn.exec_params(R"(
                SELECT t.id, t.name, t.color FROM tags t
                JOIN post_tags pt ON t.id = pt.tag_id
                WHERE pt.post_id = $1
            )", request->post_id());
            for (const auto& tag_row : tag_result) {
                auto* tag = post->add_tags();
                tag->set_id(tag_row[0].as<int64_t>());
                tag->set_name(tag_row[1].as<std::string>());
                tag->set_color(tag_row[2].as<std::string>());
            }

            txn.exec_params(R"(
                UPDATE posts SET view_count = view_count + 1 WHERE id = $1
            )", request->post_id());

            response->set_code(200);
            response->set_message("Success");
        });
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::ListPosts(::trpc::ServerContextPtr context,
                                           const ::furbbs::ListPostsRequest* request,
                                           ::furbbs::ListPostsResponse* response) {
    try {
        db::Database::Instance().Execute([&](pqxx::work& txn) {
            int32_t offset = (request->page() - 1) * request->page_size();
            std::string sort_sql = "p.created_at DESC";
            if (request->sort_by() == "hot") {
                sort_sql = "(p.like_count + p.view_count) DESC";
            } else if (request->sort_by() == "updated") {
                sort_sql = "p.updated_at DESC";
            }

            std::string category_filter = "";
            if (request->category_id() > 0) {
                category_filter = "AND p.category_id = " + std::to_string(request->category_id());
            }

            std::string keyword_filter = "";
            if (!request->keyword().empty()) {
                keyword_filter = "AND (p.title ILIKE '%" + request->keyword() + 
                                "%' OR p.content ILIKE '%" + request->keyword() + "%')";
            }

            auto count_result = txn.exec(R"(
                SELECT COUNT(*) FROM posts p
                WHERE 1=1 )" + category_filter + " " + keyword_filter
            );
            response->set_total(count_result[0][0].as<int32_t>());

            auto result = txn.exec(R"(
                SELECT p.id, p.title, p.content, p.author_id, p.category_id,
                       p.view_count, p.like_count, p.comment_count,
                       p.is_pinned, p.created_at,
                       u.name, u.display_name, u.avatar, c.name
                FROM posts p
                JOIN users u ON p.author_id = u.id
                JOIN categories c ON p.category_id = c.id
                WHERE 1=1 )" + category_filter + " " + keyword_filter + R"(
                ORDER BY p.is_pinned DESC, )" + sort_sql + R"(
                LIMIT )" + std::to_string(request->page_size()) + R"( OFFSET )" + std::to_string(offset)
            );

            for (const auto& row : result) {
                auto* post = response->add_posts();
                post->set_id(row[0].as<int64_t>());
                post->set_title(row[1].as<std::string>());
                post->set_content(row[2].as<std::string>().substr(0, 200));
                post->set_author_id(row[3].as<std::string>());
                post->set_category_id(row[4].as<int64_t>());
                post->set_view_count(row[5].as<int32_t>());
                post->set_like_count(row[6].as<int32_t>());
                post->set_comment_count(row[7].as<int32_t>());
                post->set_is_pinned(row[8].as<bool>());
                post->set_created_at(row[9].as<int64_t>());

                post->mutable_author()->set_id(row[3].as<std::string>());
                post->mutable_author()->set_name(row[10].as<std::string>());
                post->mutable_author()->set_display_name(row[11].as<std::string>());
                post->mutable_author()->set_avatar(row[12].as<std::string>());

                post->mutable_category()->set_id(row[4].as<int64_t>());
                post->mutable_category()->set_name(row[13].as<std::string>());
            }

            response->set_code(200);
            response->set_message("Success");
        });
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::CreateComment(::trpc::ServerContextPtr context,
                                               const ::furbbs::CreateCommentRequest* request,
                                               ::furbbs::CreateCommentResponse* response) {
    try {
        auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(request->access_token());
        if (!user_opt) {
            response->set_code(401);
            response->set_message("Invalid token");
            return ::trpc::kSuccStatus;
        }

        int64_t comment_id = db::Database::Instance().Execute([&](pqxx::work& txn) {
            int64_t parent_id = request->parent_id() > 0 ? request->parent_id() : 0;
            auto result = txn.exec_params(R"(
                INSERT INTO comments (post_id, content, author_id, parent_id)
                VALUES ($1, $2, $3, $4)
                RETURNING id
            )", request->post_id(), request->content(), user_opt->id, parent_id);

            txn.exec_params(R"(
                UPDATE posts SET comment_count = comment_count + 1 WHERE id = $1
            )", request->post_id());

            return result[0][0].as<int64_t>();
        });

        response->set_code(200);
        response->set_message("Comment created successfully");
        response->set_comment_id(comment_id);
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::ListComments(::trpc::ServerContextPtr context,
                                              const ::furbbs::ListCommentsRequest* request,
                                              ::furbbs::ListCommentsResponse* response) {
    try {
        db::Database::Instance().Execute([&](pqxx::work& txn) {
            int32_t offset = (request->page() - 1) * request->page_size();

            auto count_result = txn.exec_params(R"(
                SELECT COUNT(*) FROM comments WHERE post_id = $1
            )", request->post_id());
            response->set_total(count_result[0][0].as<int32_t>());

            auto result = txn.exec_params(R"(
                SELECT c.id, c.post_id, c.content, c.author_id,
                       c.parent_id, c.like_count, c.created_at,
                       u.name, u.display_name, u.avatar
                FROM comments c
                JOIN users u ON c.author_id = u.id
                WHERE c.post_id = $1
                ORDER BY c.created_at DESC
                LIMIT $2 OFFSET $3
            )", request->post_id(), request->page_size(), offset);

            for (const auto& row : result) {
                auto* comment = response->add_comments();
                comment->set_id(row[0].as<int64_t>());
                comment->set_post_id(row[1].as<int64_t>());
                comment->set_content(row[2].as<std::string>());
                comment->set_author_id(row[3].as<std::string>());
                comment->set_parent_id(row[4].as<int64_t>());
                comment->set_like_count(row[5].as<int32_t>());
                comment->set_created_at(row[6].as<int64_t>());

                comment->mutable_author()->set_id(row[3].as<std::string>());
                comment->mutable_author()->set_name(row[7].as<std::string>());
                comment->mutable_author()->set_display_name(row[8].as<std::string>());
                comment->mutable_author()->set_avatar(row[9].as<std::string>());
            }

            response->set_code(200);
            response->set_message("Success");
        });
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::LikePost(::trpc::ServerContextPtr context,
                                          const ::furbbs::LikePostRequest* request,
                                          ::furbbs::LikePostResponse* response) {
    try {
        auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(request->access_token());
        if (!user_opt) {
            response->set_code(401);
            response->set_message("Invalid token");
            return ::trpc::kSuccStatus;
        }

        db::Database::Instance().Execute([&](pqxx::work& txn) {
            try {
                txn.exec_params(R"(
                    INSERT INTO post_likes (post_id, user_id) VALUES ($1, $2)
                )", request->post_id(), user_opt->id);

                txn.exec_params(R"(
                    UPDATE posts SET like_count = like_count + 1 WHERE id = $1
                )", request->post_id());

                response->set_code(200);
                response->set_message("Liked successfully");
            } catch (const pqxx::unique_violation&) {
                response->set_code(400);
                response->set_message("Already liked");
            }
        });
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::ListCategories(::trpc::ServerContextPtr context,
                                                 const ::furbbs::ListCategoriesRequest* request,
                                                 ::furbbs::ListCategoriesResponse* response) {
    try {
        db::Database::Instance().Execute([&](pqxx::work& txn) {
            auto result = txn.exec(R"(
                SELECT id, name, description, icon, post_count, created_at
                FROM categories ORDER BY id
            )");

            for (const auto& row : result) {
                auto* category = response->add_categories();
                category->set_id(row[0].as<int64_t>());
                category->set_name(row[1].as<std::string>());
                category->set_description(row[2].as<std::string>());
                category->set_icon(row[3].as<std::string>());
                category->set_post_count(row[4].as<int32_t>());
                category->set_created_at(row[5].as<int64_t>());
            }

            response->set_code(200);
            response->set_message("Success");
        });
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::UploadGallery(::trpc::ServerContextPtr context,
                                                const ::furbbs::UploadGalleryRequest* request,
                                                ::furbbs::UploadGalleryResponse* response) {
    try {
        auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(request->access_token());
        if (!user_opt) {
            response->set_code(401);
            response->set_message("Invalid token");
            return ::trpc::kSuccStatus;
        }

        int64_t gallery_id = db::Database::Instance().Execute([&](pqxx::work& txn) {
            auto result = txn.exec_params(R"(
                INSERT INTO gallery (title, description, image_url, author_id)
                VALUES ($1, $2, $3, $4)
                RETURNING id
            )", request->title(), request->description(), request->image_url(), user_opt->id);

            int64_t id = result[0][0].as<int64_t>();
            for (const auto& tag_id : request->tag_ids()) {
                txn.exec_params(R"(
                    INSERT INTO gallery_tags (gallery_id, tag_id) VALUES ($1, $2)
                )", id, tag_id);
            }

            return id;
        });

        response->set_code(200);
        response->set_message("Gallery item uploaded successfully");
        response->set_gallery_id(gallery_id);
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::ListGallery(::trpc::ServerContextPtr context,
                                              const ::furbbs::ListGalleryRequest* request,
                                              ::furbbs::ListGalleryResponse* response) {
    try {
        db::Database::Instance().Execute([&](pqxx::work& txn) {
            int32_t offset = (request->page() - 1) * request->page_size();
            std::string tag_filter = "";
            if (request->tag_id() > 0) {
                tag_filter = "AND gt.tag_id = " + std::to_string(request->tag_id());
            }

            std::string join = "";
            if (request->tag_id() > 0) {
                join = "JOIN gallery_tags gt ON g.id = gt.gallery_id";
            }

            auto count_result = txn.exec(R"(
                SELECT COUNT(DISTINCT g.id) FROM gallery g )" + join + " WHERE 1=1 " + tag_filter
            );
            response->set_total(count_result[0][0].as<int32_t>());

            auto result = txn.exec(R"(
                SELECT DISTINCT g.id, g.title, g.description, g.image_url,
                       g.author_id, g.like_count, g.view_count, g.created_at,
                       u.name, u.display_name, u.avatar
                FROM gallery g
                JOIN users u ON g.author_id = u.id )" + join + R"(
                WHERE 1=1 )" + tag_filter + R"(
                ORDER BY g.created_at DESC
                LIMIT )" + std::to_string(request->page_size()) + R"( OFFSET )" + std::to_string(offset)
            );

            for (const auto& row : result) {
                auto* item = response->add_items();
                item->set_id(row[0].as<int64_t>());
                item->set_title(row[1].as<std::string>());
                item->set_description(row[2].as<std::string>());
                item->set_image_url(row[3].as<std::string>());
                item->set_like_count(row[5].as<int32_t>());
                item->set_view_count(row[6].as<int32_t>());
                item->set_created_at(row[7].as<int64_t>());

                item->mutable_author()->set_id(row[4].as<std::string>());
                item->mutable_author()->set_name(row[8].as<std::string>());
                item->mutable_author()->set_display_name(row[9].as<std::string>());
                item->mutable_author()->set_avatar(row[10].as<std::string>());
            }

            response->set_code(200);
            response->set_message("Success");
        });
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::FollowUser(::trpc::ServerContextPtr context,
                                            const ::furbbs::FollowUserRequest* request,
                                            ::furbbs::FollowUserResponse* response) {
    try {
        auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(request->access_token());
        if (!user_opt) {
            response->set_code(401);
            response->set_message("Invalid token");
            return ::trpc::kSuccStatus;
        }

        if (user_opt->id == request->target_user_id()) {
            response->set_code(400);
            response->set_message("Cannot follow yourself");
            return ::trpc::kSuccStatus;
        }

        db::Database::Instance().Execute([&](pqxx::work& txn) {
            try {
                txn.exec_params(R"(
                    INSERT INTO follows (follower_id, following_id) VALUES ($1, $2)
                )", user_opt->id, request->target_user_id());

                txn.exec_params(R"(
                    UPDATE users SET following_count = following_count + 1 WHERE id = $1;
                    UPDATE users SET follower_count = follower_count + 1 WHERE id = $2;
                )", user_opt->id, request->target_user_id());

                response->set_code(200);
                response->set_message("Followed successfully");
            } catch (const pqxx::unique_violation&) {
                response->set_code(400);
                response->set_message("Already following");
            }
        });
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

} // namespace furbbs::service
