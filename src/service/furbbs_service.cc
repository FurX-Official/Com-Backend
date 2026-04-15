#include "furbbs_service.h"
#include <spdlog/spdlog.h>
#include <atomic>
#include <ctime>
#include <sstream>
#include "../db/database.h"
#include "../auth/casdoor_auth.h"
#include "../common/security.h"
#include "../common/content_filter.h"
#include "../common/open_platform.h"
#include "../common/infrastructure.h"

namespace furbbs::service {

namespace {

void SendNotification(const std::string& user_id, 
                      const std::string& type,
                      const std::string& actor_id,
                      int64_t related_id,
                      const std::string& related_type,
                      const std::string& title,
                      const std::string& content = "") {
    try {
        if (user_id == actor_id) return;
        
        db::Database::Instance().Execute([&](pqxx::work& txn) {
            txn.exec_params(R"(
                INSERT INTO notifications (user_id, type, actor_id, related_id, related_type, title, content)
                VALUES ($1, $2, $3, $4, $5, $6, $7)
            )", user_id, type, actor_id, related_id, related_type, title, content);
        });
    } catch (const std::exception& e) {
        spdlog::warn("Failed to send notification: {}", e.what());
    }
}

void AddPoints(pqxx::work& txn, const std::string& user_id, int points, const std::string& type, const std::string& desc) {
    txn.exec_params(R"(
        INSERT INTO user_points (user_id, points, level)
        VALUES ($1, $2, 1)
        ON CONFLICT (user_id) DO UPDATE SET points = user_points.points + $2, updated_at = EXTRACT(EPOCH FROM NOW()) * 1000
    )", user_id, points);

    txn.exec_params(R"(
        INSERT INTO point_transactions (user_id, type, amount, description)
        VALUES ($1, $2, $3, $4)
    )", user_id, type, points, desc);

    auto result = txn.exec_params("SELECT points FROM user_points WHERE user_id = $1", user_id);
    if (!result.empty()) {
        int64_t total_points = result[0][0].as<int64_t>();
        int new_level = furbbs::common::PointSystem::GetLevel(total_points);
        txn.exec_params("UPDATE user_points SET level = $1 WHERE user_id = $2", new_level, user_id);
    }
}

std::string GetCurrentDate() {
    std::time_t t = std::time(nullptr);
    std::tm tm = *std::localtime(&t);
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d", &tm);
    return std::string(buf);
}

} // anonymous namespace

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

        std::string filtered_title = furbbs::common::ContentFilter::Instance().Filter(request->title());
        std::string filtered_content = furbbs::common::ContentFilter::Instance().Filter(request->content());

        int64_t post_id = db::Database::Instance().Execute([&](pqxx::work& txn) {
            auto result = txn.exec_params(R"(
                INSERT INTO posts (title, content, author_id, category_id)
                VALUES ($1, $2, $3, $4)
                RETURNING id
            )", filtered_title, filtered_content, user_opt->id, request->category_id());

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

            AddPoints(txn, user_opt->id, furbbs::common::PointSystem::POINTS_POST, "post", "发布帖子");

            return id;
        });

        furbbs::common::AuditLogger::Instance().Log(user_opt->id, "CREATE_POST", "", "Post ID: " + std::to_string(post_id));

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

        std::string filtered_content = furbbs::common::ContentFilter::Instance().Filter(request->content());

        int64_t comment_id = db::Database::Instance().Execute([&](pqxx::work& txn) {
            int64_t parent_id = request->parent_id() > 0 ? request->parent_id() : 0;
            auto result = txn.exec_params(R"(
                INSERT INTO comments (post_id, content, author_id, parent_id)
                VALUES ($1, $2, $3, $4)
                RETURNING id
            )", request->post_id(), filtered_content, user_opt->id, parent_id);

            txn.exec_params(R"(
                UPDATE posts SET comment_count = comment_count + 1 WHERE id = $1
            )", request->post_id());

            AddPoints(txn, user_opt->id, furbbs::common::PointSystem::POINTS_COMMENT, "comment", "发表评论");

            return result[0][0].as<int64_t>();
        });

        furbbs::common::AuditLogger::Instance().Log(user_opt->id, "CREATE_COMMENT", "", "Comment ID: " + std::to_string(comment_id));

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

                SendNotification(request->target_user_id(), "FOLLOW", user_opt->id, 0, "user",
                                user_opt->display_name.empty() ? user_opt->name : user_opt->display_name + " 关注了你");

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

::trpc::Status FurBBSServiceImpl::CreateFursona(::trpc::ServerContextPtr context,
                                               const ::furbbs::CreateFursonaRequest* request,
                                               ::furbbs::CreateFursonaResponse* response) {
    try {
        auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(request->access_token());
        if (!user_opt) {
            response->set_code(401);
            response->set_message("Invalid token");
            return ::trpc::kSuccStatus;
        }

        int64_t fursona_id = db::Database::Instance().Execute([&](pqxx::work& txn) {
            if (request->is_main()) {
                txn.exec_params(R"(
                    UPDATE fursonas SET is_main = FALSE WHERE user_id = $1
                )", user_opt->id);
            }

            pqxx::array<std::string> colors_array(request->colors().begin(), request->colors().end());
            auto result = txn.exec_params(R"(
                INSERT INTO fursonas (user_id, name, species, gender, pronouns, description, 
                                     reference_image, colors, is_main)
                VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9)
                RETURNING id
            )", user_opt->id, request->name(), request->species(), request->gender(),
                request->pronouns(), request->description(), request->reference_image(),
                colors_array, request->is_main());

            return result[0][0].as<int64_t>();
        });

        response->set_code(200);
        response->set_message("Fursona created successfully");
        response->set_fursona_id(fursona_id);
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::GetUserFursonas(::trpc::ServerContextPtr context,
                                                  const ::furbbs::GetUserFursonasRequest* request,
                                                  ::furbbs::GetUserFursonasResponse* response) {
    try {
        db::Database::Instance().Execute([&](pqxx::work& txn) {
            auto result = txn.exec_params(R"(
                SELECT id, name, species, gender, pronouns, description,
                       reference_image, colors, is_main, created_at
                FROM fursonas
                WHERE user_id = $1
                ORDER BY is_main DESC, created_at DESC
            )", request->user_id());

            for (const auto& row : result) {
                auto* fursona = response->add_fursonas();
                fursona->set_id(row[0].as<int64_t>());
                fursona->set_user_id(request->user_id());
                fursona->set_name(row[1].as<std::string>());
                fursona->set_species(row[2].as<std::string>());
                fursona->set_gender(row[3].as<std::string>());
                fursona->set_pronouns(row[4].as<std::string>());
                fursona->set_description(row[5].as<std::string>());
                fursona->set_reference_image(row[6].as<std::string>());
                fursona->set_is_main(row[8].as<bool>());
                fursona->set_created_at(row[9].as<int64_t>());

                auto colors = row[7].as<pqxx::array<std::string>>();
                for (const auto& color : colors) {
                    fursona->add_colors(color);
                }
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

::trpc::Status FurBBSServiceImpl::CreateCommissionType(::trpc::ServerContextPtr context,
                                                      const ::furbbs::CreateCommissionTypeRequest* request,
                                                      ::furbbs::CreateCommissionTypeResponse* response) {
    try {
        auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(request->access_token());
        if (!user_opt) {
            response->set_code(401);
            response->set_message("Invalid token");
            return ::trpc::kSuccStatus;
        }

        int64_t commission_id = db::Database::Instance().Execute([&](pqxx::work& txn) {
            pqxx::array<std::string> examples_array(request->examples().begin(), request->examples().end());
            auto result = txn.exec_params(R"(
                INSERT INTO commission_types (user_id, name, description, base_price, 
                                              currency, delivery_days, examples)
                VALUES ($1, $2, $3, $4, $5, $6, $7)
                RETURNING id
            )", user_opt->id, request->name(), request->description(), request->base_price(),
                "USD", request->delivery_days(), examples_array);

            txn.exec_params(R"(
                UPDATE users SET commission_status = 'OPEN' WHERE id = $1
            )", user_opt->id);

            return result[0][0].as<int64_t>();
        });

        response->set_code(200);
        response->set_message("Commission type created successfully");
        response->set_commission_type_id(commission_id);
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::GetUserCommissions(::trpc::ServerContextPtr context,
                                                     const ::furbbs::GetUserCommissionsRequest* request,
                                                     ::furbbs::GetUserCommissionsResponse* response) {
    try {
        db::Database::Instance().Execute([&](pqxx::work& txn) {
            auto result = txn.exec_params(R"(
                SELECT ct.id, ct.name, ct.description, ct.base_price, ct.currency,
                       ct.delivery_days, ct.examples, ct.is_active,
                       u.name, u.display_name, u.avatar
                FROM commission_types ct
                JOIN users u ON ct.user_id = u.id
                WHERE ct.user_id = $1 AND ct.is_active = TRUE
                ORDER BY ct.created_at DESC
            )", request->user_id());

            for (const auto& row : result) {
                auto* comm = response->add_commissions();
                comm->set_id(row[0].as<int64_t>());
                comm->set_user_id(request->user_id());
                comm->set_name(row[1].as<std::string>());
                comm->set_description(row[2].as<std::string>());
                comm->set_base_price(row[3].as<double>());
                comm->set_currency(row[4].as<std::string>());
                comm->set_delivery_days(row[5].as<int32_t>());
                comm->set_is_active(row[7].as<bool>());

                auto examples = row[6].as<pqxx::array<std::string>>();
                for (const auto& ex : examples) {
                    comm->add_examples(ex);
                }

                comm->mutable_user()->set_id(request->user_id());
                comm->mutable_user()->set_name(row[8].as<std::string>());
                comm->mutable_user()->set_display_name(row[9].as<std::string>());
                comm->mutable_user()->set_avatar(row[10].as<std::string>());
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

::trpc::Status FurBBSServiceImpl::UpdateCommissionStatus(::trpc::ServerContextPtr context,
                                                         const ::furbbs::UpdateCommissionStatusRequest* request,
                                                         ::furbbs::UpdateCommissionStatusResponse* response) {
    try {
        auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(request->access_token());
        if (!user_opt) {
            response->set_code(401);
            response->set_message("Invalid token");
            return ::trpc::kSuccStatus;
        }

        std::string status_str = "CLOSED";
        if (request->status() == ::furbbs::COMMISSION_OPEN) status_str = "OPEN";
        else if (request->status() == ::furbbs::COMMISSION_ONLY_FRIENDS) status_str = "ONLY_FRIENDS";

        db::Database::Instance().Execute([&](pqxx::work& txn) {
            txn.exec_params(R"(
                UPDATE users SET commission_status = $1 WHERE id = $2
            )", status_str, user_opt->id);
        });

        response->set_code(200);
        response->set_message("Commission status updated");
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::CreateOrder(::trpc::ServerContextPtr context,
                                              const ::furbbs::CreateOrderRequest* request,
                                              ::furbbs::CreateOrderResponse* response) {
    try {
        auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(request->access_token());
        if (!user_opt) {
            response->set_code(401);
            response->set_message("Invalid token");
            return ::trpc::kSuccStatus;
        }

        int64_t order_id = db::Database::Instance().Execute([&](pqxx::work& txn) {
            auto comm_result = txn.exec_params(R"(
                SELECT user_id, base_price FROM commission_types WHERE id = $1 AND is_active = TRUE
            )", request->commission_type_id());

            if (comm_result.empty()) {
                throw std::runtime_error("Commission type not found or inactive");
            }

            std::string seller_id = comm_result[0][0].as<std::string>();
            double price = comm_result[0][1].as<double>();

            if (seller_id == user_opt->id) {
                throw std::runtime_error("Cannot order your own commission");
            }

            auto result = txn.exec_params(R"(
                INSERT INTO commission_orders (commission_type_id, buyer_id, seller_id,
                                               requirements, reference_url, price)
                VALUES ($1, $2, $3, $4, $5, $6)
                RETURNING id
            )", request->commission_type_id(), user_opt->id, seller_id,
                request->requirements(), request->reference_url(), price);

            return result[0][0].as<int64_t>();
        });

        response->set_code(200);
        response->set_message("Order created successfully");
        response->set_order_id(order_id);
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::GetMyOrders(::trpc::ServerContextPtr context,
                                              const ::furbbs::GetMyOrdersRequest* request,
                                              ::furbbs::GetMyOrdersResponse* response) {
    try {
        auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(request->access_token());
        if (!user_opt) {
            response->set_code(401);
            response->set_message("Invalid token");
            return ::trpc::kSuccStatus;
        }

        db::Database::Instance().Execute([&](pqxx::work& txn) {
            int32_t offset = (request->page() - 1) * request->page_size();

            auto count_result = txn.exec_params(R"(
                SELECT COUNT(*) FROM commission_orders
                WHERE buyer_id = $1 OR seller_id = $1
            )", user_opt->id);
            response->set_total(count_result[0][0].as<int32_t>());

            auto result = txn.exec_params(R"(
                SELECT o.id, o.commission_type_id, o.buyer_id, o.seller_id,
                       o.requirements, o.price, o.status, o.created_at,
                       ct.name,
                       b.name, b.display_name, b.avatar,
                       s.name, s.display_name, s.avatar
                FROM commission_orders o
                JOIN commission_types ct ON o.commission_type_id = ct.id
                JOIN users b ON o.buyer_id = b.id
                JOIN users s ON o.seller_id = s.id
                WHERE o.buyer_id = $1 OR o.seller_id = $1
                ORDER BY o.created_at DESC
                LIMIT $2 OFFSET $3
            )", user_opt->id, request->page_size(), offset);

            for (const auto& row : result) {
                auto* order = response->add_orders();
                order->set_id(row[0].as<int64_t>());
                order->set_commission_type_id(row[1].as<int64_t>());
                order->set_price(row[5].as<double>());

                std::string status = row[6].as<std::string>();
                if (status == "PENDING") order->set_status(::furbbs::ORDER_PENDING);
                else if (status == "ACCEPTED") order->set_status(::furbbs::ORDER_ACCEPTED);
                else if (status == "IN_PROGRESS") order->set_status(::furbbs::ORDER_IN_PROGRESS);
                else if (status == "FINISHED") order->set_status(::furbbs::ORDER_FINISHED);
                else order->set_status(::furbbs::ORDER_CANCELLED);

                order->set_created_at(row[7].as<int64_t>());
                order->mutable_commission_type()->set_name(row[8].as<std::string>());

                order->mutable_buyer()->set_id(row[2].as<std::string>());
                order->mutable_buyer()->set_name(row[9].as<std::string>());
                order->mutable_buyer()->set_display_name(row[10].as<std::string>());
                order->mutable_buyer()->set_avatar(row[11].as<std::string>());

                order->mutable_seller()->set_id(row[3].as<std::string>());
                order->mutable_seller()->set_name(row[12].as<std::string>());
                order->mutable_seller()->set_display_name(row[13].as<std::string>());
                order->mutable_seller()->set_avatar(row[14].as<std::string>());
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

::trpc::Status FurBBSServiceImpl::CreateEvent(::trpc::ServerContextPtr context,
                                              const ::furbbs::CreateEventRequest* request,
                                              ::furbbs::CreateEventResponse* response) {
    try {
        auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(request->access_token());
        if (!user_opt) {
            response->set_code(401);
            response->set_message("Invalid token");
            return ::trpc::kSuccStatus;
        }

        int64_t event_id = db::Database::Instance().Execute([&](pqxx::work& txn) {
            pqxx::array<std::string> images_array(request->images().begin(), request->images().end());
            auto result = txn.exec_params(R"(
                INSERT INTO events (title, description, location, organizer_id,
                                   start_time, end_time, max_attendees, images)
                VALUES ($1, $2, $3, $4, $5, $6, $7, $8)
                RETURNING id
            )", request->title(), request->description(), request->location(),
                user_opt->id, request->start_time(), request->end_time(),
                request->max_attendees(), images_array);

            return result[0][0].as<int64_t>();
        });

        response->set_code(200);
        response->set_message("Event created successfully");
        response->set_event_id(event_id);
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::ListEvents(::trpc::ServerContextPtr context,
                                             const ::furbbs::ListEventsRequest* request,
                                             ::furbbs::ListEventsResponse* response) {
    try {
        db::Database::Instance().Execute([&](pqxx::work& txn) {
            int32_t offset = (request->page() - 1) * request->page_size();
            std::string status_filter = "";

            auto count_result = txn.exec(R"(
                SELECT COUNT(*) FROM events
            )");
            response->set_total(count_result[0][0].as<int32_t>());

            auto result = txn.exec(R"(
                SELECT e.id, e.title, e.description, e.location, e.organizer_id,
                       e.start_time, e.end_time, e.max_attendees, e.attendee_count,
                       e.status, e.images, e.created_at,
                       u.name, u.display_name, u.avatar
                FROM events e
                JOIN users u ON e.organizer_id = u.id
                ORDER BY e.start_time ASC
                LIMIT )" + std::to_string(request->page_size()) + R"( OFFSET )" + std::to_string(offset)
            );

            for (const auto& row : result) {
                auto* event = response->add_events();
                event->set_id(row[0].as<int64_t>());
                event->set_title(row[1].as<std::string>());
                event->set_description(row[2].as<std::string>());
                event->set_location(row[3].as<std::string>());
                event->set_start_time(row[5].as<int64_t>());
                event->set_end_time(row[6].as<int64_t>());
                event->set_max_attendees(row[7].as<int32_t>());
                event->set_attendee_count(row[8].as<int32_t>());

                std::string status = row[9].as<std::string>();
                if (status == "PLANNING") event->set_status(::furbbs::EVENT_PLANNING);
                else if (status == "CONFIRMED") event->set_status(::furbbs::EVENT_CONFIRMED);
                else if (status == "ONGOING") event->set_status(::furbbs::EVENT_ONGOING);
                else if (status == "ENDED") event->set_status(::furbbs::EVENT_ENDED);
                else event->set_status(::furbbs::EVENT_CANCELLED);

                event->set_created_at(row[11].as<int64_t>());

                auto images = row[10].as<pqxx::array<std::string>>();
                for (const auto& img : images) {
                    event->add_images(img);
                }

                event->mutable_organizer()->set_id(row[4].as<std::string>());
                event->mutable_organizer()->set_name(row[12].as<std::string>());
                event->mutable_organizer()->set_display_name(row[13].as<std::string>());
                event->mutable_organizer()->set_avatar(row[14].as<std::string>());
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

::trpc::Status FurBBSServiceImpl::JoinEvent(::trpc::ServerContextPtr context,
                                            const ::furbbs::JoinEventRequest* request,
                                            ::furbbs::JoinEventResponse* response) {
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
                    INSERT INTO event_attendees (event_id, user_id) VALUES ($1, $2)
                )", request->event_id(), user_opt->id);

                txn.exec_params(R"(
                    UPDATE events SET attendee_count = attendee_count + 1 WHERE id = $1
                )", request->event_id());

                response->set_code(200);
                response->set_message("Joined event successfully");
            } catch (const pqxx::unique_violation&) {
                response->set_code(400);
                response->set_message("Already joined this event");
            }
        });
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::SendMessage(::trpc::ServerContextPtr context,
                                              const ::furbbs::SendMessageRequest* request,
                                              ::furbbs::SendMessageResponse* response) {
    try {
        auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(request->access_token());
        if (!user_opt) {
            response->set_code(401);
            response->set_message("Invalid token");
            return ::trpc::kSuccStatus;
        }

        int64_t message_id = db::Database::Instance().Execute([&](pqxx::work& txn) {
            auto result = txn.exec_params(R"(
                INSERT INTO messages (sender_id, receiver_id, content)
                VALUES ($1, $2, $3)
                RETURNING id
            )", user_opt->id, request->receiver_id(), request->content());

            return result[0][0].as<int64_t>();
        });

        response->set_code(200);
        response->set_message("Message sent");
        response->set_message_id(message_id);
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::GetMessages(::trpc::ServerContextPtr context,
                                              const ::furbbs::GetMessagesRequest* request,
                                              ::furbbs::GetMessagesResponse* response) {
    try {
        auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(request->access_token());
        if (!user_opt) {
            response->set_code(401);
            response->set_message("Invalid token");
            return ::trpc::kSuccStatus;
        }

        db::Database::Instance().Execute([&](pqxx::work& txn) {
            int32_t offset = (request->page() - 1) * request->page_size();

            auto count_result = txn.exec_params(R"(
                SELECT COUNT(*) FROM messages
                WHERE (sender_id = $1 AND receiver_id = $2)
                   OR (sender_id = $2 AND receiver_id = $1)
            )", user_opt->id, request->other_user_id());
            response->set_total(count_result[0][0].as<int32_t>());

            auto result = txn.exec_params(R"(
                SELECT m.id, m.sender_id, m.receiver_id, m.content, m.is_read, m.created_at,
                       s.name, s.display_name, s.avatar,
                       r.name, r.display_name, r.avatar
                FROM messages m
                JOIN users s ON m.sender_id = s.id
                JOIN users r ON m.receiver_id = r.id
                WHERE (sender_id = $1 AND receiver_id = $2)
                   OR (sender_id = $2 AND receiver_id = $1)
                ORDER BY m.created_at DESC
                LIMIT $3 OFFSET $4
            )", user_opt->id, request->other_user_id(), request->page_size(), offset);

            for (const auto& row : result) {
                auto* msg = response->add_messages();
                msg->set_id(row[0].as<int64_t>());
                msg->set_content(row[3].as<std::string>());
                msg->set_is_read(row[4].as<bool>());
                msg->set_created_at(row[5].as<int64_t>());

                msg->mutable_sender()->set_id(row[1].as<std::string>());
                msg->mutable_sender()->set_name(row[6].as<std::string>());
                msg->mutable_sender()->set_display_name(row[7].as<std::string>());
                msg->mutable_sender()->set_avatar(row[8].as<std::string>());

                msg->mutable_receiver()->set_id(row[2].as<std::string>());
                msg->mutable_receiver()->set_name(row[9].as<std::string>());
                msg->mutable_receiver()->set_display_name(row[10].as<std::string>());
                msg->mutable_receiver()->set_avatar(row[11].as<std::string>());
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

static const int64_t SERVER_START_TIME = std::chrono::duration_cast<std::chrono::seconds>(
    std::chrono::system_clock::now().time_since_epoch()
).count();

::trpc::Status FurBBSServiceImpl::HealthCheck(::trpc::ServerContextPtr context,
                                             const ::furbbs::HealthCheckRequest* request,
                                             ::furbbs::HealthCheckResponse* response) {
    try {
        bool db_healthy = false;
        bool casdoor_healthy = false;
        
        try {
            db::Database::Instance().Execute([&](pqxx::work& txn) {
                auto result = txn.exec("SELECT 1");
                db_healthy = !result.empty();
            });
        } catch (...) {
            db_healthy = false;
        }

        casdoor_healthy = auth::CasdoorAuth::Instance().IsHealthy();

        auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count();

        response->set_code(200);
        response->set_message("OK");
        response->set_database_healthy(db_healthy);
        response->set_casdoor_healthy(casdoor_healthy);
        response->set_server_time(now);
        response->set_version("1.0.0");
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::GetServerStats(::trpc::ServerContextPtr context,
                                                const ::furbbs::GetServerStatsRequest* request,
                                                ::furbbs::GetServerStatsResponse* response) {
    try {
        auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(request->access_token());
        if (!user_opt) {
            response->set_code(401);
            response->set_message("Invalid token");
            return ::trpc::kSuccStatus;
        }

        if (!common::PermissionManager::Instance().IsAdmin(user_opt->id)) {
            response->set_code(403);
            response->set_message("Admin access required");
            return ::trpc::kSuccStatus;
        }

        db::Database::Instance().Execute([&](pqxx::work& txn) {
            auto now = std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::system_clock::now().time_since_epoch()
            ).count();

            response->set_code(200);
            response->set_message("Success");
            response->set_uptime_seconds(now - SERVER_START_TIME);
            
            auto users_result = txn.exec("SELECT COUNT(*) FROM users");
            response->set_total_users(users_result[0][0].as<int64_t>());
            
            auto posts_result = txn.exec("SELECT COUNT(*) FROM posts");
            response->set_total_posts(posts_result[0][0].as<int64_t>());
            
            auto comments_result = txn.exec("SELECT COUNT(*) FROM comments");
            response->set_total_comments(comments_result[0][0].as<int64_t>());
            
            auto galleries_result = txn.exec("SELECT COUNT(*) FROM gallery");
            response->set_total_galleries(galleries_result[0][0].as<int64_t>());
            
            auto fursonas_result = txn.exec("SELECT COUNT(*) FROM fursonas");
            response->set_total_fursonas(fursonas_result[0][0].as<int64_t>());
            
            auto commissions_result = txn.exec("SELECT COUNT(*) FROM commission_types");
            response->set_total_commissions(commissions_result[0][0].as<int64_t>());
            
            auto events_result = txn.exec("SELECT COUNT(*) FROM events");
            response->set_total_events(events_result[0][0].as<int64_t>());
        });
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::GrantPermission(::trpc::ServerContextPtr context,
                                                 const ::furbbs::GrantPermissionRequest* request,
                                                 ::furbbs::GrantPermissionResponse* response) {
    try {
        auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(request->access_token());
        if (!user_opt) {
            response->set_code(401);
            response->set_message("Invalid token");
            return ::trpc::kSuccStatus;
        }

        if (!common::PermissionManager::Instance().IsAdmin(user_opt->id)) {
            response->set_code(403);
            response->set_message("Admin access required");
            return ::trpc::kSuccStatus;
        }

        common::Permission perm = static_cast<common::Permission>(1 << request->permission());
        common::PermissionManager::Instance().GrantPermission(request->target_user_id(), perm);

        response->set_code(200);
        response->set_message("Permission granted successfully");
        spdlog::info("Admin {} granted permission {} to user {}", 
                     user_opt->id, request->permission(), request->target_user_id());
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::RevokePermission(::trpc::ServerContextPtr context,
                                                  const ::furbbs::RevokePermissionRequest* request,
                                                  ::furbbs::RevokePermissionResponse* response) {
    try {
        auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(request->access_token());
        if (!user_opt) {
            response->set_code(401);
            response->set_message("Invalid token");
            return ::trpc::kSuccStatus;
        }

        if (!common::PermissionManager::Instance().IsAdmin(user_opt->id)) {
            response->set_code(403);
            response->set_message("Admin access required");
            return ::trpc::kSuccStatus;
        }

        common::Permission perm = static_cast<common::Permission>(1 << request->permission());
        common::PermissionManager::Instance().RevokePermission(request->target_user_id(), perm);

        response->set_code(200);
        response->set_message("Permission revoked successfully");
        spdlog::info("Admin {} revoked permission {} from user {}", 
                     user_opt->id, request->permission(), request->target_user_id());
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::ListUserPermissions(::trpc::ServerContextPtr context,
                                                     const ::furbbs::ListUserPermissionsRequest* request,
                                                     ::furbbs::ListUserPermissionsResponse* response) {
    try {
        auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(request->access_token());
        if (!user_opt) {
            response->set_code(401);
            response->set_message("Invalid token");
            return ::trpc::kSuccStatus;
        }

        if (!common::PermissionManager::Instance().IsAdmin(user_opt->id) && 
            user_opt->id != request->target_user_id()) {
            response->set_code(403);
            response->set_message("Admin access required");
            return ::trpc::kSuccStatus;
        }

        for (int i = 0; i <= 9; i++) {
            common::Permission perm = static_cast<common::Permission>(1 << i);
            if (common::PermissionManager::Instance().HasPermission(request->target_user_id(), perm)) {
                response->add_permissions(static_cast<::furbbs::PermissionType>(i));
            }
        }

        response->set_code(200);
        response->set_message("Success");
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::ModeratePost(::trpc::ServerContextPtr context,
                                              const ::furbbs::ModeratePostRequest* request,
                                              ::furbbs::ModeratePostResponse* response) {
    try {
        auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(request->access_token());
        if (!user_opt) {
            response->set_code(401);
            response->set_message("Invalid token");
            return ::trpc::kSuccStatus;
        }

        if (!common::PermissionManager::Instance().IsModerator(user_opt->id)) {
            response->set_code(403);
            response->set_message("Moderator access required");
            return ::trpc::kSuccStatus;
        }

        db::Database::Instance().Execute([&](pqxx::work& txn) {
            if (request->remove()) {
                txn.exec_params(R"(
                    UPDATE posts SET is_removed = TRUE, moderation_reason = $2, moderated_by = $3
                    WHERE id = $1
                )", request->post_id(), request->reason(), user_opt->id);
                spdlog::warn("Moderator {} removed post {}, reason: {}", 
                            user_opt->id, request->post_id(), request->reason());
            } else {
                txn.exec_params(R"(
                    UPDATE posts SET is_removed = FALSE, moderation_reason = NULL, moderated_by = NULL
                    WHERE id = $1
                )", request->post_id());
                spdlog::info("Moderator {} restored post {}", user_opt->id, request->post_id());
            }
        });

        response->set_code(200);
        response->set_message("Post moderated successfully");
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::PinPost(::trpc::ServerContextPtr context,
                                         const ::furbbs::PinPostRequest* request,
                                         ::furbbs::PinPostResponse* response) {
    try {
        auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(request->access_token());
        if (!user_opt) {
            response->set_code(401);
            response->set_message("Invalid token");
            return ::trpc::kSuccStatus;
        }

        if (!common::PermissionManager::Instance().HasPermission(
                user_opt->id, common::Permission::PIN_POST)) {
            response->set_code(403);
            response->set_message("Pin permission required");
            return ::trpc::kSuccStatus;
        }

        db::Database::Instance().Execute([&](pqxx::work& txn) {
            txn.exec_params(R"(
                UPDATE posts SET is_pinned = $2 WHERE id = $1
            )", request->post_id(), request->is_pinned());
        });

        response->set_code(200);
        response->set_message(request->is_pinned() ? "Post pinned" : "Post unpinned");
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::LockPost(::trpc::ServerContextPtr context,
                                          const ::furbbs::LockPostRequest* request,
                                          ::furbbs::LockPostResponse* response) {
    try {
        auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(request->access_token());
        if (!user_opt) {
            response->set_code(401);
            response->set_message("Invalid token");
            return ::trpc::kSuccStatus;
        }

        if (!common::PermissionManager::Instance().HasPermission(
                user_opt->id, common::Permission::LOCK_POST)) {
            response->set_code(403);
            response->set_message("Lock permission required");
            return ::trpc::kSuccStatus;
        }

        db::Database::Instance().Execute([&](pqxx::work& txn) {
            txn.exec_params(R"(
                UPDATE posts SET is_locked = $2 WHERE id = $1
            )", request->post_id(), request->is_locked());
        });

        response->set_code(200);
        response->set_message(request->is_locked() ? "Post locked" : "Post unlocked");
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::GetNotifications(::trpc::ServerContextPtr context,
                                                   const ::furbbs::GetNotificationsRequest* request,
                                                   ::furbbs::GetNotificationsResponse* response) {
    try {
        auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(request->access_token());
        if (!user_opt) {
            response->set_code(401);
            response->set_message("Invalid token");
            return ::trpc::kSuccStatus;
        }

        int32_t page = request->page() > 0 ? request->page() : 1;
        int32_t page_size = request->page_size() > 0 ? request->page_size() : 20;
        int32_t offset = (page - 1) * page_size;

        db::Database::Instance().Execute([&](pqxx::work& txn) {
            std::string sql = R"(
                SELECT n.id, n.type, n.actor_id, n.related_id, n.related_type, n.title, n.content, n.is_read, n.created_at,
                       u.name, u.display_name, u.avatar
                FROM notifications n
                LEFT JOIN users u ON n.actor_id = u.id
                WHERE n.user_id = $1
            )";
            
            if (request->unread_only()) {
                sql += " AND n.is_read = FALSE ";
            }
            sql += " ORDER BY n.created_at DESC LIMIT $2 OFFSET $3";

            auto result = txn.exec_params(sql, user_opt->id, page_size, offset);
            
            auto count_result = txn.exec_params("SELECT COUNT(*) FROM notifications WHERE user_id = $1", user_opt->id);
            auto unread_result = txn.exec_params("SELECT COUNT(*) FROM notifications WHERE user_id = $1 AND is_read = FALSE", user_opt->id);

            for (const auto& row : result) {
                auto* notif = response->add_notifications();
                notif->set_id(row[0].as<int64_t>());
                notif->set_user_id(user_opt->id);
                notif->set_type(static_cast<::furbbs::NotificationType>(row[1].as<int32_t>()));
                notif->set_actor_id(row[2].as<std::string>());
                notif->set_related_id(row[3].as<int64_t>());
                notif->set_related_type(row[4].as<std::string>());
                notif->set_title(row[5].as<std::string>());
                notif->set_content(row[6].as<std::string>());
                notif->set_is_read(row[7].as<bool>());
                notif->set_created_at(row[8].as<int64_t>());

                if (!row[9].is_null()) {
                    auto* actor = notif->mutable_actor();
                    actor->set_id(row[2].as<std::string>());
                    actor->set_name(row[9].as<std::string>());
                    actor->set_display_name(row[10].as<std::string>());
                    actor->set_avatar(row[11].as<std::string>());
                }
            }

            response->set_total(count_result[0][0].as<int32_t>());
            response->set_unread_count(unread_result[0][0].as<int32_t>());
        });

        response->set_code(200);
        response->set_message("Success");
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::MarkNotificationRead(::trpc::ServerContextPtr context,
                                                       const ::furbbs::MarkNotificationReadRequest* request,
                                                       ::furbbs::MarkNotificationReadResponse* response) {
    try {
        auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(request->access_token());
        if (!user_opt) {
            response->set_code(401);
            response->set_message("Invalid token");
            return ::trpc::kSuccStatus;
        }

        db::Database::Instance().Execute([&](pqxx::work& txn) {
            if (request->read_all()) {
                txn.exec_params("UPDATE notifications SET is_read = TRUE WHERE user_id = $1", user_opt->id);
            } else {
                txn.exec_params("UPDATE notifications SET is_read = TRUE WHERE id = $1 AND user_id = $2",
                                request->notification_id(), user_opt->id);
            }
        });

        response->set_code(200);
        response->set_message("Marked as read");
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::AddFavorite(::trpc::ServerContextPtr context,
                                            const ::furbbs::AddFavoriteRequest* request,
                                            ::furbbs::AddFavoriteResponse* response) {
    try {
        auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(request->access_token());
        if (!user_opt) {
            response->set_code(401);
            response->set_message("Invalid token");
            return ::trpc::kSuccStatus;
        }

        int64_t favorite_id = db::Database::Instance().Execute([&](pqxx::work& txn) {
            std::string title, thumbnail;
            
            if (request->target_type() == "post") {
                auto result = txn.exec_params("SELECT title FROM posts WHERE id = $1", request->target_id());
                if (!result.empty()) title = result[0][0].as<std::string>();
            }
            
            auto result = txn.exec_params(R"(
                INSERT INTO favorites (user_id, target_id, target_type, title, thumbnail)
                VALUES ($1, $2, $3, $4, $5)
                RETURNING id
            )", user_opt->id, request->target_id(), request->target_type(), title, thumbnail);
            
            return result[0][0].as<int64_t>();
        });

        response->set_code(200);
        response->set_message("Added to favorites");
        response->set_favorite_id(favorite_id);
    } catch (const pqxx::unique_violation&) {
        response->set_code(400);
        response->set_message("Already in favorites");
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::RemoveFavorite(::trpc::ServerContextPtr context,
                                                 const ::furbbs::RemoveFavoriteRequest* request,
                                                 ::furbbs::RemoveFavoriteResponse* response) {
    try {
        auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(request->access_token());
        if (!user_opt) {
            response->set_code(401);
            response->set_message("Invalid token");
            return ::trpc::kSuccStatus;
        }

        db::Database::Instance().Execute([&](pqxx::work& txn) {
            txn.exec_params("DELETE FROM favorites WHERE id = $1 AND user_id = $2",
                            request->favorite_id(), user_opt->id);
        });

        response->set_code(200);
        response->set_message("Removed from favorites");
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::GetFavorites(::trpc::ServerContextPtr context,
                                              const ::furbbs::GetFavoritesRequest* request,
                                              ::furbbs::GetFavoritesResponse* response) {
    try {
        auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(request->access_token());
        if (!user_opt) {
            response->set_code(401);
            response->set_message("Invalid token");
            return ::trpc::kSuccStatus;
        }

        int32_t page = request->page() > 0 ? request->page() : 1;
        int32_t page_size = request->page_size() > 0 ? request->page_size() : 20;
        int32_t offset = (page - 1) * page_size;

        db::Database::Instance().Execute([&](pqxx::work& txn) {
            std::string sql = R"(
                SELECT id, target_id, target_type, title, thumbnail, created_at
                FROM favorites WHERE user_id = $1
            )";
            
            if (!request->target_type().empty()) {
                sql += " AND target_type = '" + txn.esc(request->target_type()) + "' ";
            }
            sql += " ORDER BY created_at DESC LIMIT $2 OFFSET $3";

            auto result = txn.exec_params(sql, user_opt->id, page_size, offset);
            auto count_result = txn.exec_params("SELECT COUNT(*) FROM favorites WHERE user_id = $1", user_opt->id);

            for (const auto& row : result) {
                auto* fav = response->add_favorites();
                fav->set_id(row[0].as<int64_t>());
                fav->set_user_id(user_opt->id);
                fav->set_target_id(row[1].as<int64_t>());
                fav->set_target_type(row[2].as<std::string>());
                fav->set_title(row[3].as<std::string>());
                fav->set_thumbnail(row[4].as<std::string>());
                fav->set_created_at(row[5].as<int64_t>());
            }

            response->set_total(count_result[0][0].as<int32_t>());
        });

        response->set_code(200);
        response->set_message("Success");
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::CreateReport(::trpc::ServerContextPtr context,
                                              const ::furbbs::CreateReportRequest* request,
                                              ::furbbs::CreateReportResponse* response) {
    try {
        auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(request->access_token());
        if (!user_opt) {
            response->set_code(401);
            response->set_message("Invalid token");
            return ::trpc::kSuccStatus;
        }

        std::vector<std::string> report_types = {"SPAM", "HARASSMENT", "EXPLICIT", "VIOLENCE", "COPYRIGHT", "OTHER"};

        int64_t report_id = db::Database::Instance().Execute([&](pqxx::work& txn) {
            auto result = txn.exec_params(R"(
                INSERT INTO reports (reporter_id, target_type, target_id, type, description)
                VALUES ($1, $2, $3, $4, $5)
                RETURNING id
            )", user_opt->id, request->target_type(), request->target_id(),
                report_types[request->type()], common::Security::SanitizeHtml(request->description()));
            
            return result[0][0].as<int64_t>();
        });

        response->set_code(200);
        response->set_message("Report submitted");
        response->set_report_id(report_id);
        
        spdlog::warn("User {} reported {} #{}: {}", user_opt->id, 
                     request->target_type(), request->target_id(), report_types[request->type()]);
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::GetReports(::trpc::ServerContextPtr context,
                                             const ::furbbs::GetReportsRequest* request,
                                             ::furbbs::GetReportsResponse* response) {
    try {
        auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(request->access_token());
        if (!user_opt) {
            response->set_code(401);
            response->set_message("Invalid token");
            return ::trpc::kSuccStatus;
        }

        if (!common::PermissionManager::Instance().IsModerator(user_opt->id)) {
            response->set_code(403);
            response->set_message("Moderator access required");
            return ::trpc::kSuccStatus;
        }

        int32_t page = request->page() > 0 ? request->page() : 1;
        int32_t page_size = request->page_size() > 0 ? request->page_size() : 20;
        int32_t offset = (page - 1) * page_size;

        db::Database::Instance().Execute([&](pqxx::work& txn) {
            std::string sql = R"(
                SELECT id, reporter_id, target_type, target_id, type, description, status, created_at
                FROM reports WHERE 1=1
            )";
            
            if (!request->status().empty()) {
                sql += " AND status = '" + txn.esc(request->status()) + "' ";
            }
            sql += " ORDER BY created_at DESC LIMIT $1 OFFSET $2";

            auto result = txn.exec_params(sql, page_size, offset);
            auto count_result = txn.exec("SELECT COUNT(*) FROM reports");

            for (const auto& row : result) {
                auto* report = response->add_reports();
                report->set_id(row[0].as<int64_t>());
                report->set_reporter_id(row[1].as<std::string>());
                report->set_target_type(row[2].as<std::string>());
                report->set_target_id(row[3].as<int64_t>());
            }

            response->set_total(count_result[0][0].as<int32_t>());
        });

        response->set_code(200);
        response->set_message("Success");
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::HandleReport(::trpc::ServerContextPtr context,
                                             const ::furbbs::HandleReportRequest* request,
                                             ::furbbs::HandleReportResponse* response) {
    try {
        auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(request->access_token());
        if (!user_opt) {
            response->set_code(401);
            response->set_message("Invalid token");
            return ::trpc::kSuccStatus;
        }

        if (!common::PermissionManager::Instance().IsModerator(user_opt->id)) {
            response->set_code(403);
            response->set_message("Moderator access required");
            return ::trpc::kSuccStatus;
        }

        auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count();

        db::Database::Instance().Execute([&](pqxx::work& txn) {
            txn.exec_params(R"(
                UPDATE reports SET status = $2, handled_by = $3, handle_note = $4, handled_at = $5
                WHERE id = $1
            )", request->report_id(), request->status(), user_opt->id, request->handle_note(), now);

            if (request->remove_content()) {
                txn.exec_params("UPDATE posts SET is_removed = TRUE WHERE id = $1", request->report_id());
            }
        });

        response->set_code(200);
        response->set_message("Report handled");
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::Search(::trpc::ServerContextPtr context,
                                        const ::furbbs::SearchRequest* request,
                                        ::furbbs::SearchResponse* response) {
    try {
        int32_t page = request->page() > 0 ? request->page() : 1;
        int32_t page_size = request->page_size() > 0 ? request->page_size() : 20;
        int32_t offset = (page - 1) * page_size;
        std::string search_term = "%" + request->query() + "%";

        db::Database::Instance().Execute([&](pqxx::work& txn) {
            std::string sql;
            std::string type = request->type();
            
            if (type.empty() || type == "post") {
                sql += R"(
                    SELECT id, 'post' as type, title, LEFT(content, 200), author_id, created_at
                    FROM posts WHERE is_removed = FALSE AND (title ILIKE $1 OR content ILIKE $1)
                )";
            }
            
            sql += " ORDER BY created_at DESC LIMIT $2 OFFSET $3";

            auto result = txn.exec_params(sql, search_term, page_size, offset);

            for (const auto& row : result) {
                auto* item = response->add_results();
                item->set_id(row[0].as<int64_t>());
                item->set_type(row[1].as<std::string>());
                item->set_title(row[2].as<std::string>());
                item->set_content(row[3].as<std::string>());
                item->set_author_id(row[4].as<std::string>());
                item->set_created_at(row[5].as<int64_t>());
            }

            response->set_total(result.size());
        });

        response->set_code(200);
        response->set_message("Success");
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::BlockUser(::trpc::ServerContextPtr context,
                                         const ::furbbs::BlockUserRequest* request,
                                         ::furbbs::BlockUserResponse* response) {
    try {
        auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(request->access_token());
        if (!user_opt) {
            response->set_code(401);
            response->set_message("Invalid token");
            return ::trpc::kSuccStatus;
        }

        if (user_opt->id == request->target_user_id()) {
            response->set_code(400);
            response->set_message("Cannot block yourself");
            return ::trpc::kSuccStatus;
        }

        db::Database::Instance().Execute([&](pqxx::work& txn) {
            txn.exec_params(R"(
                INSERT INTO user_blocks (blocker_id, blocked_id) VALUES ($1, $2)
                ON CONFLICT DO NOTHING
            )", user_opt->id, request->target_user_id());
        });

        response->set_code(200);
        response->set_message("User blocked");
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::UnblockUser(::trpc::ServerContextPtr context,
                                           const ::furbbs::UnblockUserRequest* request,
                                           ::furbbs::UnblockUserResponse* response) {
    try {
        auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(request->access_token());
        if (!user_opt) {
            response->set_code(401);
            response->set_message("Invalid token");
            return ::trpc::kSuccStatus;
        }

        db::Database::Instance().Execute([&](pqxx::work& txn) {
            txn.exec_params(R"(
                DELETE FROM user_blocks WHERE blocker_id = $1 AND blocked_id = $2
            )", user_opt->id, request->target_user_id());
        });

        response->set_code(200);
        response->set_message("User unblocked");
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::GetBlockedUsers(::trpc::ServerContextPtr context,
                                             const ::furbbs::GetBlockedUsersRequest* request,
                                             ::furbbs::GetBlockedUsersResponse* response) {
    try {
        auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(request->access_token());
        if (!user_opt) {
            response->set_code(401);
            response->set_message("Invalid token");
            return ::trpc::kSuccStatus;
        }

        db::Database::Instance().Execute([&](pqxx::work& txn) {
            auto result = txn.exec_params(R"(
                SELECT u.id, u.name, u.display_name, u.avatar FROM user_blocks b
                JOIN users u ON b.blocked_id = u.id
                WHERE b.blocker_id = $1
                ORDER BY b.created_at DESC
            )", user_opt->id);

            for (const auto& row : result) {
                auto* user = response->add_users();
                user->set_id(row[0].as<std::string>());
                user->set_name(row[1].as<std::string>());
                user->set_display_name(row[2].as<std::string>());
                user->set_avatar(row[3].as<std::string>());
            }

            response->set_total(result.size());
        });

        response->set_code(200);
        response->set_message("Success");
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::SaveDraft(::trpc::ServerContextPtr context,
                                          const ::furbbs::SaveDraftRequest* request,
                                          ::furbbs::SaveDraftResponse* response) {
    try {
        auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(request->access_token());
        if (!user_opt) {
            response->set_code(401);
            response->set_message("Invalid token");
            return ::trpc::kSuccStatus;
        }

        auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count();

        int64_t draft_id = db::Database::Instance().Execute([&](pqxx::work& txn) {
            pqxx::array<std::string> tags_array(request->tags().begin(), request->tags().end());
            
            if (request->draft_id() > 0) {
                auto result = txn.exec_params(R"(
                    UPDATE drafts SET type = $2, title = $3, content = $4, 
                               category_id = $5, tags = $6, updated_at = $7
                    WHERE id = $1 AND user_id = $8 RETURNING id
                )", request->draft_id(), request->type(), 
                    common::Security::SanitizeHtml(request->title()),
                    common::Security::SanitizeXss(request->content()),
                    request->category_id(), tags_array, now, user_opt->id);
                return result[0][0].as<int64_t>();
            } else {
                auto result = txn.exec_params(R"(
                    INSERT INTO drafts (user_id, type, title, content, category_id, tags, updated_at)
                    VALUES ($1, $2, $3, $4, $5, $6, $7)
                    RETURNING id
                )", user_opt->id, request->type(),
                    common::Security::SanitizeHtml(request->title()),
                    common::Security::SanitizeXss(request->content()),
                    request->category_id(), tags_array, now);
                return result[0][0].as<int64_t>();
            }
        });

        response->set_code(200);
        response->set_message("Draft saved");
        response->set_draft_id(draft_id);
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::GetDrafts(::trpc::ServerContextPtr context,
                                          const ::furbbs::GetDraftsRequest* request,
                                          ::furbbs::GetDraftsResponse* response) {
    try {
        auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(request->access_token());
        if (!user_opt) {
            response->set_code(401);
            response->set_message("Invalid token");
            return ::trpc::kSuccStatus;
        }

        int32_t page = request->page() > 0 ? request->page() : 1;
        int32_t page_size = request->page_size() > 0 ? request->page_size() : 20;
        int32_t offset = (page - 1) * page_size;

        db::Database::Instance().Execute([&](pqxx::work& txn) {
            std::string sql = R"(
                SELECT id, type, title, content, category_id, created_at, updated_at
                FROM drafts WHERE user_id = $1
            )";
            
            if (!request->type().empty()) {
                sql += " AND type = '" + txn.esc(request->type()) + "' ";
            }
            sql += " ORDER BY updated_at DESC LIMIT $2 OFFSET $3";

            auto result = txn.exec_params(sql, user_opt->id, page_size, offset);
            auto count_result = txn.exec_params("SELECT COUNT(*) FROM drafts WHERE user_id = $1", user_opt->id);

            for (const auto& row : result) {
                auto* draft = response->add_drafts();
                draft->set_id(row[0].as<int64_t>());
                draft->set_user_id(user_opt->id);
                draft->set_type(row[1].as<std::string>());
                draft->set_title(row[2].as<std::string>());
                draft->set_content(row[3].as<std::string>());
                draft->set_category_id(row[4].as<int64_t>());
                draft->set_created_at(row[5].as<int64_t>());
                draft->set_updated_at(row[6].as<int64_t>());
            }

            response->set_total(count_result[0][0].as<int32_t>());
        });

        response->set_code(200);
        response->set_message("Success");
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::DeleteDraft(::trpc::ServerContextPtr context,
                                           const ::furbbs::DeleteDraftRequest* request,
                                           ::furbbs::DeleteDraftResponse* response) {
    try {
        auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(request->access_token());
        if (!user_opt) {
            response->set_code(401);
            response->set_message("Invalid token");
            return ::trpc::kSuccStatus;
        }

        db::Database::Instance().Execute([&](pqxx::work& txn) {
            txn.exec_params("DELETE FROM drafts WHERE id = $1 AND user_id = $2",
                            request->draft_id(), user_opt->id);
        });

        response->set_code(200);
        response->set_message("Draft deleted");
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::ExportUserData(::trpc::ServerContextPtr context,
                                                   const ::furbbs::ExportDataRequest* request,
                                                   ::furbbs::ExportDataResponse* response) {
    try {
        auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(request->access_token());
        if (!user_opt) {
            response->set_code(401);
            response->set_message("Invalid token");
            return ::trpc::kSuccStatus;
        }

        db::Database::Instance().Execute([&](pqxx::work& txn) {
            std::map<std::string, int32_t> counts;
            
            auto posts_result = txn.exec_params("SELECT COUNT(*) FROM posts WHERE author_id = $1", user_opt->id);
            counts["posts"] = posts_result[0][0].as<int32_t>();
            
            auto comments_result = txn.exec_params("SELECT COUNT(*) FROM comments WHERE author_id = $1", user_opt->id);
            counts["comments"] = comments_result[0][0].as<int32_t>();
            
            auto gallery_result = txn.exec_params("SELECT COUNT(*) FROM gallery WHERE author_id = $1", user_opt->id);
            counts["gallery_items"] = gallery_result[0][0].as<int32_t>();
            
            auto fursonas_result = txn.exec_params("SELECT COUNT(*) FROM fursonas WHERE user_id = $1", user_opt->id);
            counts["fursonas"] = fursonas_result[0][0].as<int32_t>();

            for (const auto& [key, value] : counts) {
                (*response->mutable_item_counts())[key] = value;
            }
        });

        response->set_code(200);
        response->set_message("Data export prepared");
        response->set_download_url("");
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::GetUserLevel(::trpc::ServerContextPtr context,
                                              const ::furbbs::GetUserLevelRequest* request,
                                              ::furbbs::GetUserLevelResponse* response) {
    try {
        std::string user_id = request->user_id();
        
        db::Database::Instance().Execute([&](pqxx::work& txn) {
            auto result = txn.exec_params(R"(
                SELECT points, level FROM user_points WHERE user_id = $1
            )", user_id);

            int64_t points = 0;
            int level = 1;
            
            if (!result.empty()) {
                points = result[0][0].as<int64_t>();
                level = result[0][1].as<int>();
            }

            const auto& levels = furbbs::common::PointSystem::LEVELS;
            auto* level_info = response->mutable_level();
            level_info->set_level(level);
            level_info->set_name(furbbs::common::PointSystem::GetLevelName(points));
            level_info->set_points(points);
            level_info->set_next_level_points(furbbs::common::PointSystem::GetNextLevelPoints(points));
            
            for (const auto& l : levels) {
                if (l.level == level) {
                    level_info->set_icon(l.icon);
                    break;
                }
            }

            response->set_total_points(points);
            response->set_rank(furbbs::common::PointSystem::CalculateRank(points));

            auto hist_result = txn.exec_params(R"(
                SELECT id, type, amount, description, created_at
                FROM point_transactions
                WHERE user_id = $1
                ORDER BY created_at DESC
                LIMIT 20
            )", user_id);

            for (const auto& row : hist_result) {
                auto* trans = response->add_history();
                trans->set_id(row[0].as<int64_t>());
                trans->set_user_id(user_id);
                trans->set_type(row[1].as<std::string>());
                trans->set_amount(row[2].as<int32_t>());
                if (!row[3].is_null()) trans->set_description(row[3].as<std::string>());
                trans->set_created_at(row[4].as<int64_t>());
            }
        });

        furbbs::common::AuditLogger::Instance().Log(user_id, "GET_LEVEL", "", "Viewed level info");
        
        response->set_code(200);
        response->set_message("Success");
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::DailyCheckIn(::trpc::ServerContextPtr context,
                                              const ::furbbs::DailyCheckInRequest* request,
                                              ::furbbs::DailyCheckInResponse* response) {
    try {
        auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(request->access_token());
        if (!user_opt) {
            response->set_code(401);
            response->set_message("Invalid token");
            return ::trpc::kSuccStatus;
        }

        db::Database::Instance().Execute([&](pqxx::work& txn) {
            auto result = txn.exec_params(R"(
                SELECT last_check_in, consecutive_check_ins FROM user_points WHERE user_id = $1
            )", user_opt->id);

            std::string today = GetCurrentDate();
            std::string last_date = "";
            int consecutive = 0;

            if (!result.empty() && !result[0][0].is_null()) {
                last_date = result[0][0].as<std::string>();
                consecutive = result[0][1].as<int>();
            }

            if (last_date == today) {
                response->set_is_checked_in_today(true);
                response->set_points_earned(0);
                response->set_consecutive_days(consecutive);
                response->set_code(400);
                response->set_message("Already checked in today");
                return;
            }

            int points = furbbs::common::PointSystem::POINTS_CHECKIN_BASE;
            points += consecutive * furbbs::common::PointSystem::POINTS_CHECKIN_CONSECUTIVE_BONUS;
            consecutive++;

            txn.exec_params(R"(
                INSERT INTO user_points (user_id, points, level, last_check_in, consecutive_check_ins)
                VALUES ($1, $2, 1, $3, $4)
                ON CONFLICT (user_id) DO UPDATE 
                SET points = user_points.points + $2, 
                    last_check_in = $3, 
                    consecutive_check_ins = $4,
                    updated_at = EXTRACT(EPOCH FROM NOW()) * 1000
            )", user_opt->id, points, today, consecutive);

            txn.exec_params(R"(
                INSERT INTO point_transactions (user_id, type, amount, description)
                VALUES ($1, 'checkin', $2, $3)
            )", user_opt->id, points, "每日签到+" + std::to_string(points) + "分,连续" + std::to_string(consecutive) + "天");

            response->set_points_earned(points);
            response->set_consecutive_days(consecutive);
            response->set_is_checked_in_today(true);
        });

        furbbs::common::AuditLogger::Instance().Log(user_opt->id, "CHECKIN", "", "Daily check-in completed");
        
        response->set_code(200);
        response->set_message("Check-in successful");
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::GetCheckInStatus(::trpc::ServerContextPtr context,
                                                   const ::furbbs::GetCheckInStatusRequest* request,
                                                   ::furbbs::GetCheckInStatusResponse* response) {
    try {
        auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(request->access_token());
        if (!user_opt) {
            response->set_code(401);
            response->set_message("Invalid token");
            return ::trpc::kSuccStatus;
        }

        db::Database::Instance().Execute([&](pqxx::work& txn) {
            auto result = txn.exec_params(R"(
                SELECT last_check_in, consecutive_check_ins FROM user_points WHERE user_id = $1
            )", user_opt->id);

            std::string today = GetCurrentDate();
            std::string last_date = "";
            int consecutive = 0;

            if (!result.empty() && !result[0][0].is_null()) {
                last_date = result[0][0].as<std::string>();
                consecutive = result[0][1].as<int>();
            }

            response->set_is_checked_in_today(last_date == today);
            response->set_consecutive_days(consecutive);

            for (int i = 0; i < 7; i++) {
                response->add_last_7_days(false);
            }
        });

        response->set_code(200);
        response->set_message("Success");
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::SendFriendRequest(::trpc::ServerContextPtr context,
                                                   const ::furbbs::SendFriendRequestRequest* request,
                                                   ::furbbs::SendFriendRequestResponse* response) {
    try {
        auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(request->access_token());
        if (!user_opt) {
            response->set_code(401);
            response->set_message("Invalid token");
            return ::trpc::kSuccStatus;
        }

        if (user_opt->id == request->target_user_id()) {
            response->set_code(400);
            response->set_message("Cannot send friend request to yourself");
            return ::trpc::kSuccStatus;
        }

        db::Database::Instance().Execute([&](pqxx::work& txn) {
            txn.exec_params(R"(
                INSERT INTO friend_requests (sender_id, receiver_id, message)
                VALUES ($1, $2, $3)
                ON CONFLICT (sender_id, receiver_id) DO NOTHING
            )", user_opt->id, request->target_user_id(), request->message());

            SendNotification(request->target_user_id(), "friend_request", user_opt->id, 0, "friend_request", "收到好友请求");
        });

        furbbs::common::AuditLogger::Instance().Log(user_opt->id, "FRIEND_REQUEST_SENT", "", "To: " + request->target_user_id());
        
        response->set_code(200);
        response->set_message("Friend request sent");
    } catch (const pqxx::unique_violation&) {
        response->set_code(400);
        response->set_message("Friend request already sent");
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::AcceptFriendRequest(::trpc::ServerContextPtr context,
                                                     const ::furbbs::AcceptFriendRequestRequest* request,
                                                     ::furbbs::AcceptFriendRequestResponse* response) {
    try {
        auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(request->access_token());
        if (!user_opt) {
            response->set_code(401);
            response->set_message("Invalid token");
            return ::trpc::kSuccStatus;
        }

        db::Database::Instance().Execute([&](pqxx::work& txn) {
            auto result = txn.exec_params(R"(
                SELECT sender_id FROM friend_requests 
                WHERE id = $1 AND receiver_id = $2 AND status = 'PENDING'
            )", request->request_id(), user_opt->id);

            if (result.empty()) {
                response->set_code(404);
                response->set_message("Friend request not found");
                return;
            }

            std::string sender_id = result[0][0].as<std::string>();

            txn.exec_params(R"(
                UPDATE friend_requests SET status = 'ACCEPTED' WHERE id = $1
            )", request->request_id());

            txn.exec_params(R"(
                INSERT INTO friendships (user1_id, user2_id) VALUES ($1, $2)
                ON CONFLICT DO NOTHING
            )", sender_id, user_opt->id);

            AddPoints(txn, sender_id, furbbs::common::PointSystem::POINTS_FOLLOW, "friend", "添加好友");
            AddPoints(txn, user_opt->id, furbbs::common::PointSystem::POINTS_FOLLOW, "friend", "添加好友");

            SendNotification(sender_id, "friend_accepted", user_opt->id, 0, "friend", "好友请求已通过");
        });

        furbbs::common::AuditLogger::Instance().Log(user_opt->id, "FRIEND_ACCEPTED", "", "Request ID: " + std::to_string(request->request_id()));
        
        response->set_code(200);
        response->set_message("Friend request accepted");
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::GetFriendRequests(::trpc::ServerContextPtr context,
                                                   const ::furbbs::GetFriendRequestsRequest* request,
                                                   ::furbbs::GetFriendRequestsResponse* response) {
    try {
        auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(request->access_token());
        if (!user_opt) {
            response->set_code(401);
            response->set_message("Invalid token");
            return ::trpc::kSuccStatus;
        }

        db::Database::Instance().Execute([&](pqxx::work& txn) {
            auto received_result = txn.exec_params(R"(
                SELECT r.id, r.sender_id, r.message, r.created_at, u.name, u.display_name, u.avatar
                FROM friend_requests r
                LEFT JOIN users u ON r.sender_id = u.id
                WHERE r.receiver_id = $1 AND r.status = 'PENDING'
                ORDER BY r.created_at DESC
            )", user_opt->id);

            for (const auto& row : received_result) {
                auto* req = response->add_received();
                req->set_id(row[0].as<int64_t>());
                req->set_user_id(row[1].as<std::string>());
                if (!row[2].is_null()) req->set_message(row[2].as<std::string>());
                req->set_created_at(row[3].as<int64_t>());
                
                auto* user = req->mutable_user();
                user->set_id(row[1].as<std::string>());
                user->set_name(row[4].as<std::string>());
                user->set_display_name(row[5].as<std::string>());
                user->set_avatar(row[6].as<std::string>());
            }

            auto sent_result = txn.exec_params(R"(
                SELECT r.id, r.receiver_id, r.message, r.created_at, u.name, u.display_name, u.avatar
                FROM friend_requests r
                LEFT JOIN users u ON r.receiver_id = u.id
                WHERE r.sender_id = $1 AND r.status = 'PENDING'
                ORDER BY r.created_at DESC
            )", user_opt->id);

            for (const auto& row : sent_result) {
                auto* req = response->add_sent();
                req->set_id(row[0].as<int64_t>());
                req->set_user_id(row[1].as<std::string>());
                if (!row[2].is_null()) req->set_message(row[2].as<std::string>());
                req->set_created_at(row[3].as<int64_t>());
                
                auto* user = req->mutable_user();
                user->set_id(row[1].as<std::string>());
                user->set_name(row[4].as<std::string>());
                user->set_display_name(row[5].as<std::string>());
                user->set_avatar(row[6].as<std::string>());
            }

            response->set_total(received_result.size() + sent_result.size());
        });

        response->set_code(200);
        response->set_message("Success");
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::GetFriends(::trpc::ServerContextPtr context,
                                            const ::furbbs::GetFriendsRequest* request,
                                            ::furbbs::GetFriendsResponse* response) {
    try {
        std::string target_user_id = request->user_id();
        std::string current_user_id;

        if (!request->access_token().empty()) {
            auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(request->access_token());
            if (user_opt) {
                current_user_id = user_opt->id;
                if (target_user_id.empty()) {
                    target_user_id = user_opt->id;
                }
            }
        }

        if (target_user_id.empty()) {
            response->set_code(400);
            response->set_message("User ID required");
            return ::trpc::kSuccStatus;
        }

        db::Database::Instance().Execute([&](pqxx::work& txn) {
            auto result = txn.exec_params(R"(
                SELECT CASE WHEN user1_id = $1 THEN user2_id ELSE user1_id END AS friend_id
                FROM friendships
                WHERE user1_id = $1 OR user2_id = $1
            )", target_user_id);

            response->set_total(result.size());

            for (const auto& row : result) {
                std::string friend_id = row[0].as<std::string>();
                auto user_result = txn.exec_params("SELECT id, name, display_name, avatar FROM users WHERE id = $1", friend_id);
                
                if (!user_result.empty()) {
                    auto* user = response->add_friends();
                    const auto& urow = user_result[0];
                    user->set_id(urow[0].as<std::string>());
                    user->set_name(urow[1].as<std::string>());
                    user->set_display_name(urow[2].as<std::string>());
                    user->set_avatar(urow[3].as<std::string>());
                }
            }

            if (!current_user_id.empty() && current_user_id != target_user_id) {
                bool following = false, followed = false;
                
                auto follow_result = txn.exec_params(R"(
                    SELECT EXISTS(SELECT 1 FROM follows WHERE follower_id = $1 AND following_id = $2),
                           EXISTS(SELECT 1 FROM follows WHERE follower_id = $2 AND following_id = $1)
                )", current_user_id, target_user_id);

                if (!follow_result.empty()) {
                    following = follow_result[0][0].as<bool>();
                    followed = follow_result[0][1].as<bool>();
                }

                if (following && followed) {
                    response->set_status(::furbbs::FRIEND_MUTUAL);
                } else if (following) {
                    response->set_status(::furbbs::FRIEND_FOLLOWING);
                } else if (followed) {
                    response->set_status(::furbbs::FRIEND_FOLLOWED);
                } else {
                    response->set_status(::furbbs::FRIEND_NONE);
                }
            } else {
                response->set_status(::furbbs::FRIEND_MUTUAL);
            }
        });

        response->set_code(200);
        response->set_message("Success");
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::RemoveFriend(::trpc::ServerContextPtr context,
                                              const ::furbbs::RemoveFriendRequest* request,
                                              ::furbbs::RemoveFriendResponse* response) {
    try {
        auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(request->access_token());
        if (!user_opt) {
            response->set_code(401);
            response->set_message("Invalid token");
            return ::trpc::kSuccStatus;
        }

        db::Database::Instance().Execute([&](pqxx::work& txn) {
            txn.exec_params(R"(
                DELETE FROM friendships 
                WHERE (user1_id = $1 AND user2_id = $2) OR (user1_id = $2 AND user2_id = $1)
            )", user_opt->id, request->target_user_id());
        });

        furbbs::common::AuditLogger::Instance().Log(user_opt->id, "FRIEND_REMOVED", "", "Removed: " + request->target_user_id());
        
        response->set_code(200);
        response->set_message("Friend removed");
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::CreateOpenApp(::trpc::ServerContextPtr context,
                                               const ::furbbs::CreateAppRequest* request,
                                               ::furbbs::CreateAppResponse* response) {
    try {
        auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(request->access_token());
        if (!user_opt) {
            response->set_code(401);
            response->set_message("Invalid token");
            return ::trpc::kSuccStatus;
        }

        std::string client_id = furbbs::common::ApiKeyGenerator::GenerateClientId();
        std::string client_secret = furbbs::common::ApiKeyGenerator::GenerateClientSecret();

        int64_t app_id = db::Database::Instance().Execute([&](pqxx::work& txn) {
            std::vector<std::string> scopes_vec;
            for (const auto& s : request->scopes()) scopes_vec.push_back(s);
            
            auto result = txn.exec_params(R"(
                INSERT INTO open_apps (owner_id, name, description, icon, website, callback_url, client_id, client_secret, scopes)
                VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9)
                RETURNING id
            )", user_opt->id, request->name(), request->description(), 
                request->icon().empty() ? nullptr : request->icon(),
                request->website().empty() ? nullptr : request->website(),
                request->callback_url().empty() ? nullptr : request->callback_url(),
                client_id, client_secret, scopes_vec);
            
            return result[0][0].as<int64_t>();
        });

        db::Database::Instance().Execute([&](pqxx::work& txn) {
            auto result = txn.exec_params(R"(
                SELECT id, name, description, icon, website, callback_url, 
                       client_id, client_secret, scopes, is_active, rate_limit, created_at
                FROM open_apps WHERE id = $1
            )", app_id);

            const auto& row = result[0];
            auto* app = response->mutable_app();
            app->set_id(row[0].as<int64_t>());
            app->set_name(row[1].as<std::string>());
            if (!row[2].is_null()) app->set_description(row[2].as<std::string>());
            if (!row[3].is_null()) app->set_icon(row[3].as<std::string>());
            if (!row[4].is_null()) app->set_website(row[4].as<std::string>());
            if (!row[5].is_null()) app->set_callback_url(row[5].as<std::string>());
            app->set_client_id(row[6].as<std::string>());
            app->set_client_secret(row[7].as<std::string>());
            
            auto scopes = row[8].as_array();
            for (size_t i = 0; i < scopes.size(); i++) {
                if (!scopes[i].is_null()) {
                    app->add_scopes(scopes[i].as<std::string>());
                }
            }
            
            app->set_is_active(row[9].as<bool>());
            app->set_rate_limit(row[10].as<int64_t>());
            app->set_owner_id(user_opt->id);
            app->set_created_at(row[11].as<int64_t>());
        });

        furbbs::common::AuditLogger::Instance().Log(user_opt->id, "CREATE_APP", "", "App ID: " + std::to_string(app_id));

        response->set_code(200);
        response->set_message("Open app created successfully");
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::GetOpenApp(::trpc::ServerContextPtr context,
                                            const ::furbbs::GetAppRequest* request,
                                            ::furbbs::GetAppResponse* response) {
    try {
        auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(request->access_token());
        if (!user_opt) {
            response->set_code(401);
            response->set_message("Invalid token");
            return ::trpc::kSuccStatus;
        }

        db::Database::Instance().Execute([&](pqxx::work& txn) {
            auto result = txn.exec_params(R"(
                SELECT id, name, description, icon, website, callback_url, 
                       client_id, client_secret, scopes, is_active, rate_limit, created_at
                FROM open_apps WHERE id = $1 AND owner_id = $2
            )", request->app_id(), user_opt->id);

            if (result.empty()) {
                response->set_code(404);
                response->set_message("App not found");
                return;
            }

            const auto& row = result[0];
            auto* app = response->mutable_app();
            app->set_id(row[0].as<int64_t>());
            app->set_name(row[1].as<std::string>());
            if (!row[2].is_null()) app->set_description(row[2].as<std::string>());
            if (!row[3].is_null()) app->set_icon(row[3].as<std::string>());
            if (!row[4].is_null()) app->set_website(row[4].as<std::string>());
            if (!row[5].is_null()) app->set_callback_url(row[5].as<std::string>());
            app->set_client_id(row[6].as<std::string>());
            app->set_client_secret(row[7].as<std::string>());
            
            auto scopes = row[8].as_array();
            for (size_t i = 0; i < scopes.size(); i++) {
                if (!scopes[i].is_null()) {
                    app->add_scopes(scopes[i].as<std::string>());
                }
            }
            
            app->set_is_active(row[9].as<bool>());
            app->set_rate_limit(row[10].as<int64_t>());
            app->set_owner_id(user_opt->id);
            app->set_created_at(row[11].as<int64_t>());
        });

        response->set_code(200);
        response->set_message("Success");
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::ListOpenApps(::trpc::ServerContextPtr context,
                                              const ::furbbs::ListAppsRequest* request,
                                              ::furbbs::ListAppsResponse* response) {
    try {
        auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(request->access_token());
        if (!user_opt) {
            response->set_code(401);
            response->set_message("Invalid token");
            return ::trpc::kSuccStatus;
        }

        int32_t page = request->page() > 0 ? request->page() : 1;
        int32_t page_size = request->page_size() > 0 ? request->page_size() : 20;
        int32_t offset = (page - 1) * page_size;

        db::Database::Instance().Execute([&](pqxx::work& txn) {
            auto count_result = txn.exec_params("SELECT COUNT(*) FROM open_apps WHERE owner_id = $1", user_opt->id);
            response->set_total(count_result[0][0].as<int32_t>());

            auto result = txn.exec_params(R"(
                SELECT id, name, description, icon, website, callback_url, 
                       client_id, is_active, rate_limit, created_at
                FROM open_apps WHERE owner_id = $1
                ORDER BY created_at DESC
                LIMIT $2 OFFSET $3
            )", user_opt->id, page_size, offset);

            for (const auto& row : result) {
                auto* app = response->add_apps();
                app->set_id(row[0].as<int64_t>());
                app->set_name(row[1].as<std::string>());
                if (!row[2].is_null()) app->set_description(row[2].as<std::string>());
                if (!row[3].is_null()) app->set_icon(row[3].as<std::string>());
                if (!row[4].is_null()) app->set_website(row[4].as<std::string>());
                if (!row[5].is_null()) app->set_callback_url(row[5].as<std::string>());
                app->set_client_id(row[6].as<std::string>());
                app->set_is_active(row[7].as<bool>());
                app->set_rate_limit(row[8].as<int64_t>());
                app->set_owner_id(user_opt->id);
                app->set_created_at(row[9].as<int64_t>());
            }
        });

        response->set_code(200);
        response->set_message("Success");
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::RegenerateAppSecret(::trpc::ServerContextPtr context,
                                                     const ::furbbs::RegenerateSecretRequest* request,
                                                     ::furbbs::RegenerateSecretResponse* response) {
    try {
        auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(request->access_token());
        if (!user_opt) {
            response->set_code(401);
            response->set_message("Invalid token");
            return ::trpc::kSuccStatus;
        }

        std::string new_secret = furbbs::common::ApiKeyGenerator::GenerateClientSecret();

        db::Database::Instance().Execute([&](pqxx::work& txn) {
            auto result = txn.exec_params(R"(
                UPDATE open_apps SET client_secret = $1 
                WHERE id = $2 AND owner_id = $3
                RETURNING id
            )", new_secret, request->app_id(), user_opt->id);

            if (result.empty()) {
                response->set_code(404);
                response->set_message("App not found");
                return;
            }
        });

        furbbs::common::AuditLogger::Instance().Log(user_opt->id, "REGENERATE_SECRET", "", "App ID: " + std::to_string(request->app_id()));

        response->set_code(200);
        response->set_message("Secret regenerated");
        response->set_client_secret(new_secret);
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::GetApiStats(::trpc::ServerContextPtr context,
                                             const ::furbbs::GetApiStatsRequest* request,
                                             ::furbbs::GetApiStatsResponse* response) {
    try {
        auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(request->access_token());
        if (!user_opt) {
            response->set_code(401);
            response->set_message("Invalid token");
            return ::trpc::kSuccStatus;
        }

        db::Database::Instance().Execute([&](pqxx::work& txn) {
            auto app_result = txn.exec_params(R"(
                SELECT client_id, rate_limit FROM open_apps WHERE id = $1 AND owner_id = $2
            )", request->app_id(), user_opt->id);

            if (app_result.empty()) {
                response->set_code(404);
                response->set_message("App not found");
                return;
            }

            std::string client_id = app_result[0][0].as<std::string>();
            int64_t rate_limit = app_result[0][1].as<int64_t>();

            auto* stats = response->mutable_stats();
            stats->set_today_calls(furbbs::common::RateLimiter::Instance().GetTodayCalls(client_id));
            stats->set_rate_limit_remaining(furbbs::common::RateLimiter::Instance().GetRemaining(client_id, rate_limit));

            auto total_result = txn.exec_params(R"(
                SELECT COALESCE(SUM(call_count), 0) FROM api_stats WHERE client_id = $1
            )", client_id);
            stats->set_total_calls(total_result[0][0].as<int64_t>());

            auto endpoint_result = txn.exec_params(R"(
                SELECT endpoint, SUM(call_count) FROM api_stats 
                WHERE client_id = $1 AND call_date = CURRENT_DATE
                GROUP BY endpoint
            )", client_id);

            for (const auto& row : endpoint_result) {
                (*stats->mutable_endpoint_calls())[row[0].as<std::string>()] = row[1].as<int64_t>();
            }
        });

        response->set_code(200);
        response->set_message("Success");
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::CreateWebhook(::trpc::ServerContextPtr context,
                                               const ::furbbs::CreateWebhookRequest* request,
                                               ::furbbs::CreateWebhookResponse* response) {
    try {
        auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(request->access_token());
        if (!user_opt) {
            response->set_code(401);
            response->set_message("Invalid token");
            return ::trpc::kSuccStatus;
        }

        std::string secret = furbbs::common::ApiKeyGenerator::GenerateWebhookSecret();
        std::vector<std::string> events_vec;
        for (const auto& e : request->events()) events_vec.push_back(e);

        int64_t webhook_id = db::Database::Instance().Execute([&](pqxx::work& txn) {
            auto result = txn.exec_params(R"(
                INSERT INTO webhooks (app_id, endpoint, events, secret)
                SELECT $1, $2, $3, $4
                FROM open_apps WHERE id = $1 AND owner_id = $5
                RETURNING id
            )", request->app_id(), request->endpoint(), events_vec, secret, user_opt->id);

            if (result.empty()) {
                throw std::runtime_error("App not found or no permission");
            }
            return result[0][0].as<int64_t>();
        });

        furbbs::common::WebhookDispatcher::Instance().AddWebhook(request->app_id(), request->endpoint(), events_vec, secret);

        auto* webhook = response->mutable_webhook();
        webhook->set_id(webhook_id);
        webhook->set_app_id(request->app_id());
        webhook->set_endpoint(request->endpoint());
        for (const auto& e : events_vec) webhook->add_events(e);
        webhook->set_secret(secret);
        webhook->set_is_active(true);

        furbbs::common::AuditLogger::Instance().Log(user_opt->id, "CREATE_WEBHOOK", "", "App ID: " + std::to_string(request->app_id()));

        response->set_code(200);
        response->set_message("Webhook created successfully");
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::GetAnnouncements(::trpc::ServerContextPtr context,
                                                  const ::furbbs::GetAnnouncementsRequest* request,
                                                  ::furbbs::GetAnnouncementsResponse* response) {
    try {
        int32_t page = request->page() > 0 ? request->page() : 1;
        int32_t page_size = request->page_size() > 0 ? request->page_size() : 10;
        int32_t offset = (page - 1) * page_size;

        db::Database::Instance().Execute([&](pqxx::work& txn) {
            auto count_result = txn.exec("SELECT COUNT(*) FROM announcements WHERE is_active = TRUE");
            response->set_total(count_result[0][0].as<int32_t>());

            auto result = txn.exec_params(R"(
                SELECT id, title, content, type, is_active, created_at, expire_at
                FROM announcements WHERE is_active = TRUE
                ORDER BY created_at DESC
                LIMIT $1 OFFSET $2
            )", page_size, offset);

            for (const auto& row : result) {
                auto* ann = response->add_announcements();
                ann->set_id(row[0].as<int64_t>());
                ann->set_title(row[1].as<std::string>());
                if (!row[2].is_null()) ann->set_content(row[2].as<std::string>());
                ann->set_type(row[3].as<std::string>());
                ann->set_is_active(row[4].as<bool>());
                ann->set_created_at(row[5].as<int64_t>());
                if (!row[6].is_null()) ann->set_expire_at(row[6].as<int64_t>());
            }
        });

        response->set_code(200);
        response->set_message("Success");
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::GetBanners(::trpc::ServerContextPtr context,
                                            const ::furbbs::GetBannersResponse* request,
                                            ::furbbs::GetBannersResponse* response) {
    try {
        db::Database::Instance().Execute([&](pqxx::work& txn) {
            auto result = txn.exec(R"(
                SELECT id, title, image, link, "order", is_active, created_at
                FROM banners WHERE is_active = TRUE
                ORDER BY "order" ASC
            )");

            for (const auto& row : result) {
                auto* banner = response->add_banners();
                banner->set_id(row[0].as<int64_t>());
                if (!row[1].is_null()) banner->set_title(row[1].as<std::string>());
                banner->set_image(row[2].as<std::string>());
                if (!row[3].is_null()) banner->set_link(row[3].as<std::string>());
                banner->set_order(row[4].as<int32_t>());
                banner->set_is_active(row[5].as<bool>());
                banner->set_created_at(row[6].as<int64_t>());
            }
        });

        response->set_code(200);
        response->set_message("Success");
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::GetHotTopics(::trpc::ServerContextPtr context,
                                              const ::furbbs::GetHotTopicsResponse* request,
                                              ::furbbs::GetHotTopicsResponse* response) {
    try {
        db::Database::Instance().Execute([&](pqxx::work& txn) {
            auto result = txn.exec(R"(
                SELECT t.id, t.name, t.icon, COUNT(DISTINCT pt.post_id) as post_count,
                       SUM(p.view_count) as view_count,
                       (COUNT(DISTINCT pt.post_id) * 10 + SUM(p.view_count) * 0.1) as heat
                FROM tags t
                LEFT JOIN post_tags pt ON t.id = pt.tag_id
                LEFT JOIN posts p ON pt.post_id = p.id
                WHERE p.created_at > EXTRACT(EPOCH FROM NOW() - INTERVAL '7 days') * 1000
                GROUP BY t.id, t.name, t.icon
                HAVING COUNT(DISTINCT pt.post_id) > 0
                ORDER BY heat DESC
                LIMIT 20
            )");

            int rank = 1;
            for (const auto& row : result) {
                auto* topic = response->add_topics();
                topic->set_id(row[0].as<int64_t>());
                topic->set_name(row[1].as<std::string>());
                if (!row[2].is_null()) topic->set_icon(row[2].as<std::string>());
                topic->set_post_count(row[3].as<int64_t>());
                topic->set_view_count(row[4].as<int64_t>());
                topic->set_heat_score(row[5].as<int64_t>());
                rank++;
            }
        });

        response->set_code(200);
        response->set_message("Success");
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::GetRankings(::trpc::ServerContextPtr context,
                                             const ::furbbs::GetRankingsRequest* request,
                                             ::furbbs::GetRankingsResponse* response) {
    try {
        int32_t limit = request->limit() > 0 ? request->limit() : 10;
        std::string type = request->type().empty() ? "points" : request->type();

        db::Database::Instance().Execute([&](pqxx::work& txn) {
            pqxx::result result;

            if (type == "points") {
                result = txn.exec_params(R"(
                    SELECT up.user_id, u.name, u.display_name, u.avatar, up.points
                    FROM user_points up
                    JOIN users u ON up.user_id = u.id
                    ORDER BY up.points DESC
                    LIMIT $1
                )", limit);
            } else if (type == "posts") {
                result = txn.exec_params(R"(
                    SELECT p.author_id, u.name, u.display_name, u.avatar, COUNT(*) as cnt
                    FROM posts p
                    JOIN users u ON p.author_id = u.id
                    GROUP BY p.author_id, u.name, u.display_name, u.avatar
                    ORDER BY cnt DESC
                    LIMIT $1
                )", limit);
            } else if (type == "followers") {
                result = txn.exec_params(R"(
                    SELECT f.following_id, u.name, u.display_name, u.avatar, COUNT(*) as cnt
                    FROM follows f
                    JOIN users u ON f.following_id = u.id
                    GROUP BY f.following_id, u.name, u.display_name, u.avatar
                    ORDER BY cnt DESC
                    LIMIT $1
                )", limit);
            }

            int rank = 1;
            for (const auto& row : result) {
                auto* item = response->add_items();
                item->set_id(row[0].as<std::string>());
                item->set_name(row[1].as<std::string>());
                if (!row[2].is_null()) item->set_name(row[2].as<std::string>());
                if (!row[3].is_null()) item->set_avatar(row[3].as<std::string>());
                item->set_score(row[4].as<int64_t>());
                item->set_rank(rank);
                rank++;
            }
        });

        response->set_code(200);
        response->set_message("Success");
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::UploadFile(::trpc::ServerContextPtr context,
                                            const ::furbbs::UploadFileRequest* request,
                                            ::furbbs::UploadFileResponse* response) {
    try {
        auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(request->access_token());
        if (!user_opt) {
            response->set_code(401);
            response->set_message("Invalid token");
            return ::trpc::kSuccStatus;
        }

        if (!furbbs::common::FileStorage::Instance().ValidateSize(request->content().size())) {
            response->set_code(400);
            response->set_message("File too large (max 10MB)");
            return ::trpc::kSuccStatus;
        }

        if (!request->content_type().empty() && 
            !furbbs::common::FileStorage::Instance().ValidateMimeType(request->content_type())) {
            response->set_code(400);
            response->set_message("File type not allowed");
            return ::trpc::kSuccStatus;
        }

        auto file_info = furbbs::common::FileStorage::Instance().Save(
            request->content(),
            request->filename(),
            request->content_type(),
            user_opt->id,
            request->purpose()
        );

        response->set_file_id(file_info.id);
        response->set_url(file_info.url);
        response->set_size(file_info.size);
        response->set_mime_type(file_info.mime_type);

        furbbs::common::AuditLogger::Instance().Log(user_opt->id, "UPLOAD_FILE", "", 
            "File: " + request->filename() + ", Size: " + std::to_string(file_info.size));

        response->set_code(200);
        response->set_message("File uploaded successfully");
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::GetFile(::trpc::ServerContextPtr context,
                                         const ::furbbs::GetFileRequest* request,
                                         ::furbbs::GetFileResponse* response) {
    try {
        auto file_info = furbbs::common::FileStorage::Instance().Get(request->file_id());
        if (!file_info) {
            response->set_code(404);
            response->set_message("File not found");
            return ::trpc::kSuccStatus;
        }

        response->set_url(file_info->url);
        response->set_filename(file_info->original_name);
        response->set_size(file_info->size);
        response->set_mime_type(file_info->mime_type);

        response->set_code(200);
        response->set_message("Success");
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::DeleteFile(::trpc::ServerContextPtr context,
                                            const ::furbbs::DeleteFileRequest* request,
                                            ::furbbs::DeleteFileResponse* response) {
    try {
        auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(request->access_token());
        if (!user_opt) {
            response->set_code(401);
            response->set_message("Invalid token");
            return ::trpc::kSuccStatus;
        }

        bool success = furbbs::common::FileStorage::Instance().Delete(
            request->file_id(), 
            user_opt->id
        );

        if (!success) {
            response->set_code(404);
            response->set_message("File not found or no permission");
            return ::trpc::kSuccStatus;
        }

        furbbs::common::AuditLogger::Instance().Log(user_opt->id, "DELETE_FILE", "", 
            "File ID: " + request->file_id());

        response->set_code(200);
        response->set_message("File deleted successfully");
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::SendEmailCode(::trpc::ServerContextPtr context,
                                               const ::furbbs::SendEmailCodeRequest* request,
                                               ::furbbs::SendEmailCodeResponse* response) {
    try {
        bool success = furbbs::common::EmailCodeManager::Instance().SendCode(
            request->email(),
            request->type()
        );

        if (!success) {
            response->set_code(500);
            response->set_message("Failed to send email code");
            return ::trpc::kSuccStatus;
        }

        response->set_expire_seconds(furbbs::common::EmailCodeManager::EXPIRE_SECONDS);
        response->set_code(200);
        response->set_message("Verification code sent");
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::VerifyEmailCode(::trpc::ServerContextPtr context,
                                                 const ::furbbs::VerifyEmailCodeRequest* request,
                                                 ::furbbs::VerifyEmailCodeResponse* response) {
    try {
        bool valid = furbbs::common::EmailCodeManager::Instance().VerifyCode(
            request->email(),
            request->code(),
            request->type()
        );

        response->set_is_valid(valid);
        
        if (valid) {
            response->set_code(200);
            response->set_message("Verification successful");
        } else {
            response->set_code(400);
            response->set_message("Invalid or expired code");
        }
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::GetCacheStats(::trpc::ServerContextPtr context,
                                               const ::furbbs::GetSystemMetricsResponse* request,
                                               ::furbbs::GetCacheStatsResponse* response) {
    try {
        auto stats = furbbs::common::Cache::Instance().GetStats();
        
        auto* metrics = response->mutable_metrics();
        metrics->set_hits(stats.hits);
        metrics->set_misses(stats.misses);
        metrics->set_hit_rate(stats.hits + stats.misses > 0 
            ? static_cast<double>(stats.hits) / (stats.hits + stats.misses) 
            : 0.0);
        metrics->set_evictions(stats.evictions);

        response->set_code(200);
        response->set_message("Success");
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::GetSystemMetrics(::trpc::ServerContextPtr context,
                                                  const ::furbbs::GetSystemMetricsResponse* request,
                                                  ::furbbs::GetSystemMetricsResponse* response) {
    try {
        auto* metrics = response->mutable_metrics();
        metrics->set_cpu_usage(furbbs::common::SystemMonitor::Instance().GetCpuUsage());
        metrics->set_memory_usage_mb(furbbs::common::SystemMonitor::Instance().GetMemoryUsageMB());
        metrics->set_disk_usage_gb(furbbs::common::SystemMonitor::Instance().GetDiskUsageGB());
        metrics->set_database_connections(furbbs::common::SystemMonitor::Instance().GetDatabaseConnections());
        metrics->set_active_users(furbbs::common::SystemMonitor::Instance().GetActiveUsers());

        response->set_code(200);
        response->set_message("Success");
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::GetForumSections(::trpc::ServerContextPtr context,
                                                   const ::furbbs::GetForumSectionsRequest* request,
                                                   ::furbbs::GetForumSectionsResponse* response) {
    try {
        db::Database::Instance().Execute([&](pqxx::work& txn) {
            pqxx::result result;
            if (request->parent_id() > 0) {
                result = txn.exec_params(R"(
                    SELECT id, name, description, icon, sort_order, parent_id, 
                           post_count, thread_count, is_active, created_at
                    FROM forum_sections 
                    WHERE parent_id = $1 AND is_active = TRUE
                    ORDER BY sort_order ASC
                )", request->parent_id());
            } else {
                result = txn.exec(R"(
                    SELECT id, name, description, icon, sort_order, parent_id, 
                           post_count, thread_count, is_active, created_at
                    FROM forum_sections 
                    WHERE parent_id IS NULL AND is_active = TRUE
                    ORDER BY sort_order ASC
                )");
            }

            for (const auto& row : result) {
                auto* section = response->add_sections();
                section->set_id(row[0].as<int64_t>());
                section->set_name(row[1].as<std::string>());
                if (!row[2].is_null()) section->set_description(row[2].as<std::string>());
                if (!row[3].is_null()) section->set_icon(row[3].as<std::string>());
                section->set_sort_order(row[4].as<int64_t>());
                if (!row[5].is_null()) section->set_parent_id(row[5].as<int64_t>());
                section->set_post_count(row[6].as<int32_t>());
                section->set_thread_count(row[7].as<int32_t>());
                section->set_is_active(row[8].as<bool>());
            }
        });

        response->set_code(200);
        response->set_message("Success");
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::CreateForumSection(::trpc::ServerContextPtr context,
                                                     const ::furbbs::CreateForumSectionRequest* request,
                                                     ::furbbs::CreateForumSectionResponse* response) {
    try {
        auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(request->access_token());
        if (!user_opt || user_opt->role != "admin") {
            response->set_code(403);
            response->set_message("Permission denied");
            return ::trpc::kSuccStatus;
        }

        int64_t section_id = db::Database::Instance().Execute([&](pqxx::work& txn) {
            auto result = txn.exec_params(R"(
                INSERT INTO forum_sections (name, description, icon, sort_order, parent_id)
                VALUES ($1, $2, $3, $4, $5)
                RETURNING id
            )", request->name(), 
                request->description().empty() ? nullptr : request->description(),
                request->icon().empty() ? nullptr : request->icon(),
                static_cast<int>(request->sort_order()),
                request->parent_id() > 0 ? static_cast<int>(request->parent_id()) : nullptr);
            return result[0][0].as<int64_t>();
        });

        response->set_section_id(section_id);
        response->set_code(200);
        response->set_message("Forum section created successfully");
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::GetTags(::trpc::ServerContextPtr context,
                                          const ::furbbs::GetTagsRequest* request,
                                          ::furbbs::GetTagsResponse* response) {
    try {
        db::Database::Instance().Execute([&](pqxx::work& txn) {
            auto result = txn.exec(R"(
                SELECT id, name, color, use_count FROM tags 
                ORDER BY use_count DESC, name ASC
            )");

            for (const auto& row : result) {
                auto* tag = response->add_tags();
                tag->set_id(row[0].as<int64_t>());
                tag->set_name(row[1].as<std::string>());
                tag->set_color(row[2].as<std::string>());
                tag->set_use_count(row[3].as<int32_t>());
            }
        });

        response->set_code(200);
        response->set_message("Success");
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::SearchPosts(::trpc::ServerContextPtr context,
                                              const ::furbbs::SearchPostsRequest* request,
                                              ::furbbs::SearchPostsResponse* response) {
    try {
        int page = std::max(1, request->page());
        int page_size = std::min(100, std::max(1, request->page_size()));
        int offset = (page - 1) * page_size;

        db::Database::Instance().Execute([&](pqxx::work& txn) {
            std::string sql = R"(
                SELECT p.id, p.title, p.content, p.author_id, p.author_name, 
                       p.section_id, p.view_count, p.like_count, p.comment_count,
                       p.rating, p.is_pinned, p.is_essence, p.created_at, p.updated_at
                FROM posts p
                WHERE p.status = 'published'
            )";
            std::vector<std::string> params;
            int param_count = 1;

            if (!request->keyword().empty()) {
                sql += " AND (p.title ILIKE $" + std::to_string(param_count++) + 
                       " OR p.content ILIKE $" + std::to_string(param_count++) + ")";
                params.push_back("%" + request->keyword() + "%");
                params.push_back("%" + request->keyword() + "%");
            }

            if (request->section_id() > 0) {
                sql += " AND p.section_id = $" + std::to_string(param_count++);
                params.push_back(std::to_string(request->section_id()));
            }

            if (!request->user_id().empty()) {
                sql += " AND p.author_id = $" + std::to_string(param_count++);
                params.push_back(request->user_id());
            }

            if (!request->tag().empty()) {
                sql += " AND EXISTS (SELECT 1 FROM post_tags pt JOIN tags t ON pt.tag_id = t.id "
                       "WHERE pt.post_id = p.id AND t.name = $" + std::to_string(param_count++) + ")";
                params.push_back(request->tag());
            }

            std::string order_by = "p.created_at DESC";
            if (request->sort_by() == "hot") {
                order_by = "(p.like_count * 5 + p.comment_count * 3 + p.view_count) DESC";
            } else if (request->sort_by() == "views") {
                order_by = "p.view_count DESC";
            }
            sql += " ORDER BY " + order_by + " LIMIT $" + std::to_string(param_count++) + 
                   " OFFSET $" + std::to_string(param_count++);
            params.push_back(std::to_string(page_size));
            params.push_back(std::to_string(offset));

            pqxx::result result;
            if (params.empty()) {
                result = txn.exec(sql);
            } else {
                result = txn.exec_params(sql, params[0], params[1], params[2], params[3], params[4], params[5]);
            }

            for (const auto& row : result) {
                auto* post = response->add_posts();
                post->set_id(row[0].as<int64_t>());
                post->set_title(row[1].as<std::string>());
                post->set_content(row[2].as<std::string>());
                post->set_author_id(row[3].as<std::string>());
                post->set_author_name(row[4].as<std::string>());
                post->set_view_count(row[6].as<int32_t>());
                post->set_like_count(row[7].as<int32_t>());
                post->set_comment_count(row[8].as<int32_t>());
                post->set_created_at(row[11].as<int64_t>());
            }

            std::string count_sql = "SELECT COUNT(*) FROM posts p WHERE p.status = 'published'";
            auto count_result = txn.exec(count_sql);
            response->set_total(count_result[0][0].as<int32_t>());
        });

        response->set_code(200);
        response->set_message("Success");
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::GetUserBadges(::trpc::ServerContextPtr context,
                                                const ::furbbs::GetUserBadgesRequest* request,
                                                ::furbbs::GetUserBadgesResponse* response) {
    try {
        db::Database::Instance().Execute([&](pqxx::work& txn) {
            auto result = txn.exec_params(R"(
                SELECT b.id, b.name, b.description, b.icon, b.rarity, 
                       b.requirement_type, b.requirement_value
                FROM badges b
                JOIN user_badges ub ON b.id = ub.badge_id
                WHERE ub.user_id = $1
                ORDER BY ub.created_at DESC
            )", request->user_id());

            for (const auto& row : result) {
                auto* badge = response->add_badges();
                badge->set_id(row[0].as<int64_t>());
                badge->set_name(row[1].as<std::string>());
                badge->set_description(row[2].as<std::string>());
                badge->set_icon(row[3].as<std::string>());
                badge->set_rarity(row[4].as<std::string>());
                badge->set_requirement_type(row[5].as<int32_t>());
                badge->set_requirement_value(row[6].as<int32_t>());
            }
        });

        response->set_code(200);
        response->set_message("Success");
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::GetAllBadges(::trpc::ServerContextPtr context,
                                               const ::furbbs::GetSystemMetricsResponse* request,
                                               ::furbbs::GetAllBadgesResponse* response) {
    try {
        db::Database::Instance().Execute([&](pqxx::work& txn) {
            auto result = txn.exec(R"(
                SELECT id, name, description, icon, rarity, requirement_type, requirement_value
                FROM badges ORDER BY id ASC
            )");

            for (const auto& row : result) {
                auto* badge = response->add_badges();
                badge->set_id(row[0].as<int64_t>());
                badge->set_name(row[1].as<std::string>());
                badge->set_description(row[2].as<std::string>());
                badge->set_icon(row[3].as<std::string>());
                badge->set_rarity(row[4].as<std::string>());
                badge->set_requirement_type(row[5].as<int32_t>());
                badge->set_requirement_value(row[6].as<int32_t>());
            }
        });

        response->set_code(200);
        response->set_message("Success");
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::UpdateOnlineStatus(::trpc::ServerContextPtr context,
                                                     const ::furbbs::UpdateOnlineStatusRequest* request,
                                                     ::furbbs::UpdateOnlineStatusResponse* response) {
    try {
        auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(request->access_token());
        if (!user_opt) {
            response->set_code(401);
            response->set_message("Invalid token");
            return ::trpc::kSuccStatus;
        }

        auto now = std::chrono::system_clock::now();
        auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()).count();

        db::Database::Instance().Execute([&](pqxx::work& txn) {
            txn.exec_params(R"(
                INSERT INTO user_online_status (user_id, is_online, last_active, status_text)
                VALUES ($1, TRUE, $2, $3)
                ON CONFLICT (user_id) DO UPDATE
                SET is_online = TRUE, last_active = $2, status_text = $3
            )", user_opt->id, timestamp, 
                request->status_text().empty() ? nullptr : request->status_text());
        });

        response->set_code(200);
        response->set_message("Online status updated");
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::GetOnlineUsers(::trpc::ServerContextPtr context,
                                                 const ::furbbs::GetOnlineUsersRequest* request,
                                                 ::furbbs::GetOnlineUsersResponse* response) {
    try {
        auto now = std::chrono::system_clock::now();
        auto five_minutes_ago = now - std::chrono::minutes(5);
        auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            five_minutes_ago.time_since_epoch()).count();
        int limit = request->limit() > 0 ? request->limit() : 50;

        db::Database::Instance().Execute([&](pqxx::work& txn) {
            auto result = txn.exec_params(R"(
                SELECT user_id, is_online, last_active, status_text
                FROM user_online_status
                WHERE last_active > $1 AND is_online = TRUE
                ORDER BY last_active DESC
                LIMIT $2
            )", timestamp, limit);

            for (const auto& row : result) {
                auto* user_status = response->add_users();
                user_status->set_user_id(row[0].as<std::string>());
                user_status->set_is_online(row[1].as<bool>());
                user_status->set_last_active(row[2].as<int64_t>());
                if (!row[3].is_null()) user_status->set_status_text(row[3].as<std::string>());
            }

            auto count_result = txn.exec_params(R"(
                SELECT COUNT(*) FROM user_online_status
                WHERE last_active > $1 AND is_online = TRUE
            )", timestamp);
            response->set_total_online(count_result[0][0].as<int32_t>());
        });

        response->set_code(200);
        response->set_message("Success");
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::GetReadingHistory(::trpc::ServerContextPtr context,
                                                    const ::furbbs::GetReadingHistoryRequest* request,
                                                    ::furbbs::GetReadingHistoryResponse* response) {
    try {
        auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(request->access_token());
        if (!user_opt) {
            response->set_code(401);
            response->set_message("Invalid token");
            return ::trpc::kSuccStatus;
        }

        int page = std::max(1, request->page());
        int page_size = std::min(100, std::max(1, request->page_size()));
        int offset = (page - 1) * page_size;

        db::Database::Instance().Execute([&](pqxx::work& txn) {
            auto result = txn.exec_params(R"(
                SELECT rh.post_id, p.title, rh.read_at
                FROM reading_history rh
                JOIN posts p ON rh.post_id = p.id
                WHERE rh.user_id = $1
                ORDER BY rh.read_at DESC
                LIMIT $2 OFFSET $3
            )", user_opt->id, page_size, offset);

            for (const auto& row : result) {
                auto* history = response->add_history();
                history->set_post_id(row[0].as<int64_t>());
                history->set_post_title(row[1].as<std::string>());
                history->set_read_at(row[2].as<int64_t>());
            }

            auto count_result = txn.exec_params(R"(
                SELECT COUNT(*) FROM reading_history WHERE user_id = $1
            )", user_opt->id);
            response->set_total(count_result[0][0].as<int32_t>());
        });

        response->set_code(200);
        response->set_message("Success");
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::GetForumStats(::trpc::ServerContextPtr context,
                                                const ::furbbs::GetSystemMetricsResponse* request,
                                                ::furbbs::GetForumStatsResponse* response) {
    try {
        auto now = std::chrono::system_clock::now();
        auto today = std::chrono::floor<std::chrono::days>(now);
        auto today_start = std::chrono::duration_cast<std::chrono::milliseconds>(
            today.time_since_epoch()).count();

        db::Database::Instance().Execute([&](pqxx::work& txn) {
            auto* stats = response->mutable_stats();

            auto users_result = txn.exec("SELECT COUNT(*) FROM users");
            stats->set_total_users(users_result[0][0].as<int64_t>());

            auto posts_result = txn.exec("SELECT COUNT(*) FROM posts WHERE status = 'published'");
            stats->set_total_posts(posts_result[0][0].as<int64_t>());

            auto comments_result = txn.exec("SELECT COUNT(*) FROM comments");
            stats->set_total_comments(comments_result[0][0].as<int64_t>());

            auto today_posts_result = txn.exec_params(R"(
                SELECT COUNT(*) FROM posts 
                WHERE status = 'published' AND created_at > $1
            )", today_start);
            stats->set_today_posts(today_posts_result[0][0].as<int64_t>());

            auto today_users_result = txn.exec_params(R"(
                SELECT COUNT(*) FROM users WHERE created_at > $1
            )", today_start);
            stats->set_today_registrations(today_users_result[0][0].as<int64_t>());

            auto newest_result = txn.exec(R"(
                SELECT id, username FROM users 
                ORDER BY created_at DESC LIMIT 1
            )");
            if (!newest_result.empty()) {
                response->set_newest_user_id(newest_result[0][0].as<std::string>());
                response->set_newest_username(newest_result[0][1].as<std::string>());
            }
        });

        response->set_code(200);
        response->set_message("Success");
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::CreatePoll(::trpc::ServerContextPtr context,
                                             const ::furbbs::CreatePollRequest* request,
                                             ::furbbs::CreatePollResponse* response) {
    try {
        auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(request->access_token());
        if (!user_opt) {
            response->set_code(401);
            response->set_message("Invalid token");
            return ::trpc::kSuccStatus;
        }

        if (request->options_size() < 2) {
            response->set_code(400);
            response->set_message("Poll must have at least 2 options");
            return ::trpc::kSuccStatus;
        }

        int64_t poll_id = db::Database::Instance().Execute([&](pqxx::work& txn) {
            auto poll_result = txn.exec_params(R"(
                INSERT INTO polls (post_id, question, is_multiple, end_time)
                VALUES ($1, $2, $3, $4)
                RETURNING id
            )", request->post_id() > 0 ? request->post_id() : nullptr,
                request->question(), request->is_multiple(),
                request->end_time() > 0 ? request->end_time() : nullptr);
            
            int64_t pid = poll_result[0][0].as<int64_t>();

            for (const auto& option : request->options()) {
                txn.exec_params(R"(
                    INSERT INTO poll_options (poll_id, text)
                    VALUES ($1, $2)
                )", static_cast<int>(pid), option);
            }
            return pid;
        });

        response->set_poll_id(poll_id);
        response->set_code(200);
        response->set_message("Poll created successfully");
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::VotePoll(::trpc::ServerContextPtr context,
                                           const ::furbbs::VotePollRequest* request,
                                           ::furbbs::VotePollResponse* response) {
    try {
        auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(request->access_token());
        if (!user_opt) {
            response->set_code(401);
            response->set_message("Invalid token");
            return ::trpc::kSuccStatus;
        }

        db::Database::Instance().Execute([&](pqxx::work& txn) {
            auto poll_result = txn.exec_params(R"(
                SELECT is_multiple, is_closed, end_time FROM polls WHERE id = $1
            )", static_cast<int>(request->poll_id()));

            if (poll_result.empty()) {
                throw std::runtime_error("Poll not found");
            }

            bool is_multiple = poll_result[0][0].as<bool>();
            bool is_closed = poll_result[0][1].as<bool>();
            int64_t end_time = poll_result[0][2].is_null() ? 0 : poll_result[0][2].as<int64_t>();

            auto now = std::chrono::system_clock::now();
            auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                now.time_since_epoch()).count();

            if (is_closed || (end_time > 0 && end_time < timestamp)) {
                throw std::runtime_error("Poll is closed");
            }

            if (!is_multiple && request->option_ids_size() > 1) {
                throw std::runtime_error("This poll only allows single choice");
            }

            auto existing_vote = txn.exec_params(R"(
                SELECT 1 FROM poll_votes WHERE poll_id = $1 AND user_id = $2
            )", static_cast<int>(request->poll_id()), user_opt->id);

            if (!existing_vote.empty()) {
                throw std::runtime_error("Already voted");
            }

            for (const auto& option_id : request->option_ids()) {
                txn.exec_params(R"(
                    INSERT INTO poll_votes (poll_id, option_id, user_id)
                    VALUES ($1, $2, $3)
                )", static_cast<int>(request->poll_id()), static_cast<int>(option_id), user_opt->id);

                txn.exec_params(R"(
                    UPDATE poll_options SET vote_count = vote_count + 1 WHERE id = $1
                )", static_cast<int>(option_id));
            }

            txn.exec_params(R"(
                UPDATE polls SET total_votes = total_votes + $1 WHERE id = $2
            )", request->option_ids_size(), static_cast<int>(request->poll_id()));
        });

        response->set_code(200);
        response->set_message("Vote recorded successfully");
    } catch (const std::exception& e) {
        response->set_code(400);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::GetPoll(::trpc::ServerContextPtr context,
                                          const ::furbbs::GetPollRequest* request,
                                          ::furbbs::GetPollResponse* response) {
    try {
        db::Database::Instance().Execute([&](pqxx::work& txn) {
            auto poll_result = txn.exec_params(R"(
                SELECT id, question, is_multiple, end_time, is_closed, total_votes
                FROM polls WHERE id = $1
            )", static_cast<int>(request->poll_id()));

            if (poll_result.empty()) {
                throw std::runtime_error("Poll not found");
            }

            const auto& row = poll_result[0];
            auto* poll = response->mutable_poll();
            poll->set_id(row[0].as<int64_t>());
            poll->set_question(row[1].as<std::string>());
            poll->set_is_multiple(row[2].as<bool>());
            if (!row[3].is_null()) poll->set_end_time(row[3].as<int64_t>());
            poll->set_is_closed(row[4].as<bool>());
            poll->set_total_votes(row[5].as<int32_t>());

            auto options_result = txn.exec_params(R"(
                SELECT id, text, vote_count FROM poll_options WHERE poll_id = $1
            )", static_cast<int>(request->poll_id()));

            for (const auto& opt_row : options_result) {
                auto* option = poll->add_options();
                option->set_id(opt_row[0].as<int64_t>());
                option->set_text(opt_row[1].as<std::string>());
                option->set_vote_count(opt_row[2].as<int32_t>());
            }

            if (!request->access_token().empty()) {
                auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(request->access_token());
                if (user_opt) {
                    auto vote_result = txn.exec_params(R"(
                        SELECT option_id FROM poll_votes 
                        WHERE poll_id = $1 AND user_id = $2
                    )", static_cast<int>(request->poll_id()), user_opt->id);

                    for (const auto& vote_row : vote_result) {
                        poll->add_user_votes(vote_row[0].as<int64_t>());
                    }
                }
            }
        });

        response->set_code(200);
        response->set_message("Success");
    } catch (const std::exception& e) {
        response->set_code(404);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::GetGiftList(::trpc::ServerContextPtr context,
                                              const ::furbbs::GetSystemMetricsResponse* request,
                                              ::furbbs::GetGiftListResponse* response) {
    try {
        db::Database::Instance().Execute([&](pqxx::work& txn) {
            auto result = txn.exec(R"(
                SELECT id, name, description, image, price, rarity, is_available
                FROM gifts WHERE is_available = TRUE
                ORDER BY price ASC
            )");

            for (const auto& row : result) {
                auto* gift = response->add_gifts();
                gift->set_id(row[0].as<int64_t>());
                gift->set_name(row[1].as<std::string>());
                gift->set_description(row[2].as<std::string>());
                gift->set_image(row[3].as<std::string>());
                gift->set_price(row[4].as<int32_t>());
                gift->set_rarity(row[5].as<std::string>());
                gift->set_is_available(row[6].as<bool>());
            }
        });

        response->set_code(200);
        response->set_message("Success");
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::SendGift(::trpc::ServerContextPtr context,
                                           const ::furbbs::SendGiftRequest* request,
                                           ::furbbs::SendGiftResponse* response) {
    try {
        auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(request->access_token());
        if (!user_opt) {
            response->set_code(401);
            response->set_message("Invalid token");
            return ::trpc::kSuccStatus;
        }

        int quantity = std::max(1, request->quantity());

        db::Database::Instance().Execute([&](pqxx::work& txn) {
            auto gift_result = txn.exec_params(R"(
                SELECT price FROM gifts WHERE id = $1 AND is_available = TRUE
            )", static_cast<int>(request->gift_id()));

            if (gift_result.empty()) {
                throw std::runtime_error("Gift not available");
            }

            int price = gift_result[0][0].as<int>();
            int total_cost = price * quantity;

            auto points_result = txn.exec_params(R"(
                SELECT points FROM user_stats WHERE user_id = $1
            )", user_opt->id);

            int user_points = points_result.empty() ? 0 : points_result[0][0].as<int>();
            if (user_points < total_cost) {
                throw std::runtime_error("Insufficient points");
            }

            txn.exec_params(R"(
                UPDATE user_stats SET points = points - $1 WHERE user_id = $2
            )", total_cost, user_opt->id);

            auto now = std::chrono::system_clock::now();
            auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                now.time_since_epoch()).count();

            txn.exec_params(R"(
                INSERT INTO user_gifts (from_user_id, to_user_id, post_id, gift_id, 
                                       quantity, message, is_anonymous, sent_at)
                VALUES ($1, $2, $3, $4, $5, $6, $7, $8)
            )", user_opt->id, request->to_user_id(),
                request->post_id() > 0 ? request->post_id() : nullptr,
                static_cast<int>(request->gift_id()), quantity,
                request->message().empty() ? nullptr : request->message(),
                request->is_anonymous(), timestamp);

            furbbs::common::NotificationSender::Instance().Send(
                request->to_user_id(), "gift_received",
                "收到了礼物",
                user_opt->username + " 送给你一个礼物！"
            );
        });

        response->set_code(200);
        response->set_message("Gift sent successfully");
    } catch (const std::exception& e) {
        response->set_code(400);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::GetUserGifts(::trpc::ServerContextPtr context,
                                               const ::furbbs::GetUserGiftsRequest* request,
                                               ::furbbs::GetUserGiftsResponse* response) {
    try {
        int page = std::max(1, request->page());
        int page_size = std::min(50, std::max(1, request->page_size()));
        int offset = (page - 1) * page_size;

        db::Database::Instance().Execute([&](pqxx::work& txn) {
            auto result = txn.exec_params(R"(
                SELECT g.id, g.name, g.description, g.image, g.price, g.rarity,
                       ug.quantity, ug.from_user_id, u.username, ug.message, 
                       ug.sent_at, ug.is_anonymous
                FROM user_gifts ug
                JOIN gifts g ON ug.gift_id = g.id
                LEFT JOIN users u ON ug.from_user_id = u.id
                WHERE ug.to_user_id = $1
                ORDER BY ug.sent_at DESC
                LIMIT $2 OFFSET $3
            )", request->user_id(), page_size, offset);

            int total_value = 0;
            for (const auto& row : result) {
                auto* received = response->add_gifts();
                auto* gift = received->mutable_gift();
                gift->set_id(row[0].as<int64_t>());
                gift->set_name(row[1].as<std::string>());
                gift->set_description(row[2].as<std::string>());
                gift->set_image(row[3].as<std::string>());
                gift->set_price(row[4].as<int32_t>());
                gift->set_rarity(row[5].as<std::string>());
                
                received->set_quantity(row[6].as<int32_t>());
                if (!row[11].as<bool>()) {
                    received->set_from_username(row[8].is_null() ? "匿名用户" : row[8].as<std::string>());
                } else {
                    received->set_from_username("匿名用户");
                }
                received->set_is_anonymous(row[11].as<bool>());
                if (!row[9].is_null()) received->set_message(row[9].as<std::string>());
                received->set_sent_at(row[10].as<int64_t>());
                
                total_value += row[4].as<int>() * row[6].as<int>();
            }

            auto count_result = txn.exec_params(R"(
                SELECT COUNT(*) FROM user_gifts WHERE to_user_id = $1
            )", request->user_id());
            response->set_total(count_result[0][0].as<int32_t>());
            response->set_total_value(total_value);
        });

        response->set_code(200);
        response->set_message("Success");
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::BanUser(::trpc::ServerContextPtr context,
                                          const ::furbbs::BanUserRequest* request,
                                          ::furbbs::BanUserResponse* response) {
    try {
        auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(request->access_token());
        if (!user_opt || (user_opt->role != "admin" && user_opt->role != "moderator")) {
            response->set_code(403);
            response->set_message("Permission denied");
            return ::trpc::kSuccStatus;
        }

        auto now = std::chrono::system_clock::now();
        auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()).count();
        int64_t expire_at = request->duration_seconds() > 0 
            ? timestamp + request->duration_seconds() * 1000 
            : 0;
        bool is_permanent = request->duration_seconds() <= 0;

        db::Database::Instance().Execute([&](pqxx::work& txn) {
            txn.exec_params(R"(
                INSERT INTO user_bans (user_id, moderator_id, reason, type, expire_at, is_permanent)
                VALUES ($1, $2, $3, $4, $5, $6)
            )", request->user_id(), user_opt->id, request->reason(), request->type(),
                expire_at > 0 ? expire_at : nullptr, is_permanent);

            txn.exec_params(R"(
                INSERT INTO moderator_logs (moderator_id, action, target_id, target_type, reason)
                VALUES ($1, $2, $3, $4, $5)
            )", user_opt->id, "ban_user", 0, "user", 
                "Banned user: " + request->user_id() + " - " + request->reason());
        });

        response->set_code(200);
        response->set_message("User banned successfully");
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::UnbanUser(::trpc::ServerContextPtr context,
                                            const ::furbbs::UnbanUserRequest* request,
                                            ::furbbs::UnbanUserResponse* response) {
    try {
        auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(request->access_token());
        if (!user_opt || (user_opt->role != "admin" && user_opt->role != "moderator")) {
            response->set_code(403);
            response->set_message("Permission denied");
            return ::trpc::kSuccStatus;
        }

        db::Database::Instance().Execute([&](pqxx::work& txn) {
            txn.exec_params(R"(
                DELETE FROM user_bans WHERE user_id = $1
            )", request->user_id());

            txn.exec_params(R"(
                INSERT INTO moderator_logs (moderator_id, action, target_id, target_type, reason)
                VALUES ($1, $2, $3, $4, $5)
            )", user_opt->id, "unban_user", 0, "user", 
                "Unbanned user: " + request->user_id());
        });

        response->set_code(200);
        response->set_message("User unbanned successfully");
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::GetUserBan(::trpc::ServerContextPtr context,
                                             const ::furbbs::GetUserBanRequest* request,
                                             ::furbbs::GetUserBanResponse* response) {
    try {
        auto now = std::chrono::system_clock::now();
        auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()).count();

        db::Database::Instance().Execute([&](pqxx::work& txn) {
            auto result = txn.exec_params(R"(
                SELECT id, user_id, moderator_id, reason, type, expire_at, is_permanent, created_at
                FROM user_bans 
                WHERE user_id = $1 AND (is_permanent = TRUE OR expire_at > $2)
                ORDER BY created_at DESC LIMIT 1
            )", request->user_id(), timestamp);

            if (!result.empty()) {
                const auto& row = result[0];
                auto* ban = response->mutable_ban();
                ban->set_id(row[0].as<int64_t>());
                ban->set_user_id(row[1].as<std::string>());
                ban->set_moderator_id(row[2].as<std::string>());
                ban->set_reason(row[3].as<std::string>());
                ban->set_type(row[4].as<std::string>());
                if (!row[5].is_null()) ban->set_expire_at(row[5].as<int64_t>());
                ban->set_is_permanent(row[6].as<bool>());
                ban->set_created_at(row[7].as<int64_t>());

                response->set_is_banned(ban->type() == "ban");
                response->set_is_muted(ban->type() == "mute");
            } else {
                response->set_is_banned(false);
                response->set_is_muted(false);
            }
        });

        response->set_code(200);
        response->set_message("Success");
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::GetModeratorLogs(::trpc::ServerContextPtr context,
                                                   const ::furbbs::GetModeratorLogsRequest* request,
                                                   ::furbbs::GetModeratorLogsResponse* response) {
    try {
        auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(request->access_token());
        if (!user_opt || (user_opt->role != "admin" && user_opt->role != "moderator")) {
            response->set_code(403);
            response->set_message("Permission denied");
            return ::trpc::kSuccStatus;
        }

        int page = std::max(1, request->page());
        int page_size = std::min(100, std::max(1, request->page_size()));
        int offset = (page - 1) * page_size;

        db::Database::Instance().Execute([&](pqxx::work& txn) {
            auto result = txn.exec_params(R"(
                SELECT id, moderator_id, action, target_id, target_type, reason, created_at
                FROM moderator_logs
                ORDER BY created_at DESC
                LIMIT $1 OFFSET $2
            )", page_size, offset);

            for (const auto& row : result) {
                auto* log = response->add_logs();
                log->set_id(row[0].as<int64_t>());
                log->set_moderator_id(row[1].as<std::string>());
                log->set_action(row[2].as<std::string>());
                if (!row[3].is_null()) log->set_target_id(row[3].as<int64_t>());
                if (!row[4].is_null()) log->set_target_type(row[4].as<std::string>());
                if (!row[5].is_null()) log->set_reason(row[5].as<std::string>());
                log->set_created_at(row[6].as<int64_t>());
            }

            auto count_result = txn.exec("SELECT COUNT(*) FROM moderator_logs");
            response->set_total(count_result[0][0].as<int32_t>());
        });

        response->set_code(200);
        response->set_message("Success");
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::ManagePost(::trpc::ServerContextPtr context,
                                             const ::furbbs::ManagePostRequest* request,
                                             ::furbbs::ManagePostResponse* response) {
    try {
        auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(request->access_token());
        if (!user_opt || (user_opt->role != "admin" && user_opt->role != "moderator")) {
            response->set_code(403);
            response->set_message("Permission denied");
            return ::trpc::kSuccStatus;
        }

        db::Database::Instance().Execute([&](pqxx::work& txn) {
            if (request->action() == "pin") {
                txn.exec_params("UPDATE posts SET is_pinned = NOT is_pinned WHERE id = $1", 
                    static_cast<int>(request->post_id()));
            } else if (request->action() == "essence") {
                txn.exec_params("UPDATE posts SET is_essence = NOT is_essence WHERE id = $1", 
                    static_cast<int>(request->post_id()));
            } else if (request->action() == "move" && request->target_section_id() > 0) {
                txn.exec_params("UPDATE posts SET section_id = $1 WHERE id = $2", 
                    static_cast<int>(request->target_section_id()), 
                    static_cast<int>(request->post_id()));
            } else if (request->action() == "delete") {
                txn.exec_params("UPDATE posts SET status = 'deleted' WHERE id = $1", 
                    static_cast<int>(request->post_id()));
            } else if (request->action() == "restore") {
                txn.exec_params("UPDATE posts SET status = 'published' WHERE id = $1", 
                    static_cast<int>(request->post_id()));
            }

            txn.exec_params(R"(
                INSERT INTO moderator_logs (moderator_id, action, target_id, target_type, reason)
                VALUES ($1, $2, $3, $4, $5)
            )", user_opt->id, request->action(), request->post_id(), "post", request->reason());
        });

        response->set_code(200);
        response->set_message("Post " + request->action() + " successfully");
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::GetShopItems(::trpc::ServerContextPtr context,
                                               const ::furbbs::GetSystemMetricsResponse* request,
                                               ::furbbs::GetShopItemsResponse* response) {
    try {
        db::Database::Instance().Execute([&](pqxx::work& txn) {
            auto result = txn.exec(R"(
                SELECT id, name, description, image, type, price, stock, is_available
                FROM shop_items WHERE is_available = TRUE
                ORDER BY price ASC
            )");

            for (const auto& row : result) {
                auto* item = response->add_items();
                item->set_id(row[0].as<int64_t>());
                item->set_name(row[1].as<std::string>());
                if (!row[2].is_null()) item->set_description(row[2].as<std::string>());
                if (!row[3].is_null()) item->set_image(row[3].as<std::string>());
                item->set_type(row[4].as<std::string>());
                item->set_price(row[5].as<int32_t>());
                item->set_stock(row[6].as<int32_t>());
                item->set_is_available(row[7].as<bool>());
            }
        });

        response->set_code(200);
        response->set_message("Success");
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::PurchaseItem(::trpc::ServerContextPtr context,
                                                const ::furbbs::PurchaseItemRequest* request,
                                                ::furbbs::PurchaseItemResponse* response) {
    try {
        auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(request->access_token());
        if (!user_opt) {
            response->set_code(401);
            response->set_message("Invalid token");
            return ::trpc::kSuccStatus;
        }

        int quantity = std::max(1, request->quantity());

        db::Database::Instance().Execute([&](pqxx::work& txn) {
            auto item_result = txn.exec_params(R"(
                SELECT price, stock FROM shop_items 
                WHERE id = $1 AND is_available = TRUE
            )", static_cast<int>(request->item_id()));

            if (item_result.empty()) {
                throw std::runtime_error("Item not available");
            }

            int price = item_result[0][0].as<int>();
            int stock = item_result[0][1].as<int>();
            int total_cost = price * quantity;

            if (stock > 0 && stock < quantity) {
                throw std::runtime_error("Insufficient stock");
            }

            auto points_result = txn.exec_params(R"(
                SELECT points FROM user_stats WHERE user_id = $1
            )", user_opt->id);

            int user_points = points_result.empty() ? 0 : points_result[0][0].as<int>();
            if (user_points < total_cost) {
                throw std::runtime_error("Insufficient points");
            }

            txn.exec_params(R"(
                UPDATE user_stats SET points = points - $1 WHERE user_id = $2
            )", total_cost, user_opt->id);

            txn.exec_params(R"(
                INSERT INTO user_inventory (user_id, item_id, quantity)
                VALUES ($1, $2, $3)
                ON CONFLICT (user_id, item_id, is_used) DO UPDATE
                SET quantity = user_inventory.quantity + $3
            )", user_opt->id, static_cast<int>(request->item_id()), quantity);

            if (stock > 0) {
                txn.exec_params(R"(
                    UPDATE shop_items SET stock = stock - $1 WHERE id = $2
                )", quantity, static_cast<int>(request->item_id()));
            }
        });

        response->set_code(200);
        response->set_message("Item purchased successfully");
    } catch (const std::exception& e) {
        response->set_code(400);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::GetUserInventory(::trpc::ServerContextPtr context,
                                                   const ::furbbs::GetUserInventoryRequest* request,
                                                   ::furbbs::GetUserInventoryResponse* response) {
    try {
        auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(request->access_token());
        if (!user_opt) {
            response->set_code(401);
            response->set_message("Invalid token");
            return ::trpc::kSuccStatus;
        }

        db::Database::Instance().Execute([&](pqxx::work& txn) {
            auto result = txn.exec_params(R"(
                SELECT si.id, si.name, si.description, si.image, si.type, si.price, ui.quantity
                FROM user_inventory ui
                JOIN shop_items si ON ui.item_id = si.id
                WHERE ui.user_id = $1 AND ui.is_used = FALSE
            )", user_opt->id);

            for (const auto& row : result) {
                auto* item = response->add_items();
                item->set_id(row[0].as<int64_t>());
                item->set_name(row[1].as<std::string>());
                if (!row[2].is_null()) item->set_description(row[2].as<std::string>());
                if (!row[3].is_null()) item->set_image(row[3].as<std::string>());
                item->set_type(row[4].as<std::string>());
                item->set_price(row[5].as<int32_t>());
            }
        });

        response->set_code(200);
        response->set_message("Success");
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::UpdateProfile(::trpc::ServerContextPtr context,
                                                const ::furbbs::UpdateProfileRequest* request,
                                                ::furbbs::UpdateProfileResponse* response) {
    try {
        auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(request->access_token());
        if (!user_opt) {
            response->set_code(401);
            response->set_message("Invalid token");
            return ::trpc::kSuccStatus;
        }

        db::Database::Instance().Execute([&](pqxx::work& txn) {
            if (!request->signature().empty()) {
                txn.exec_params(R"(
                    UPDATE users SET signature = $1 WHERE id = $2
                )", request->signature().substr(0, 500), user_opt->id);
            }

            if (request->active_avatar_frame_id() > 0) {
                auto has_frame = txn.exec_params(R"(
                    SELECT 1 FROM user_inventory ui
                    JOIN shop_items si ON ui.item_id = si.id
                    WHERE ui.user_id = $1 AND si.type = 'avatar_frame' 
                      AND si.properties->>'frame_id' = $2 AND ui.is_used = FALSE
                )", user_opt->id, static_cast<int>(request->active_avatar_frame_id()));

                if (has_frame.empty() && request->active_avatar_frame_id() != 1) {
                    throw std::runtime_error("Avatar frame not in inventory");
                }

                txn.exec_params(R"(
                    UPDATE users SET active_avatar_frame_id = $1 WHERE id = $2
                )", static_cast<int>(request->active_avatar_frame_id()), user_opt->id);
            }
        });

        response->set_code(200);
        response->set_message("Profile updated successfully");
    } catch (const std::exception& e) {
        response->set_code(400);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::CreateFavoriteFolder(::trpc::ServerContextPtr context,
                                                       const ::furbbs::CreateFavoriteFolderRequest* request,
                                                       ::furbbs::CreateFavoriteFolderResponse* response) {
    try {
        auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(request->access_token());
        if (!user_opt) {
            response->set_code(401);
            response->set_message("Invalid token");
            return ::trpc::kSuccStatus;
        }

        int64_t folder_id = db::Database::Instance().Execute([&](pqxx::work& txn) {
            auto result = txn.exec_params(R"(
                INSERT INTO favorite_folders (user_id, name, is_public)
                VALUES ($1, $2, $3)
                RETURNING id
            )", user_opt->id, request->name(), request->is_public());
            return result[0][0].as<int64_t>();
        });

        response->set_folder_id(folder_id);
        response->set_code(200);
        response->set_message("Folder created successfully");
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::GetFavoriteFolders(::trpc::ServerContextPtr context,
                                                      const ::furbbs::GetFavoriteFoldersRequest* request,
                                                      ::furbbs::GetFavoriteFoldersResponse* response) {
    try {
        std::string target_user = request->user_id().empty() ? "" : request->user_id();
        
        if (!request->access_token().empty() && target_user.empty()) {
            auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(request->access_token());
            if (user_opt) {
                target_user = user_opt->id;
            }
        }

        if (target_user.empty()) {
            response->set_code(400);
            response->set_message("User ID required");
            return ::trpc::kSuccStatus;
        }

        db::Database::Instance().Execute([&](pqxx::work& txn) {
            pqxx::result result;
            
            auto user_opt = !request->access_token().empty() 
                ? auth::CasdoorAuth::Instance().VerifyToken(request->access_token())
                : std::nullopt;
            bool is_owner = user_opt && user_opt->id == target_user;

            if (is_owner) {
                result = txn.exec_params(R"(
                    SELECT f.id, f.name, f.is_public, f.created_at, COUNT(fp.id)
                    FROM favorite_folders f
                    LEFT JOIN favorites fp ON f.id = fp.folder_id
                    WHERE f.user_id = $1
                    GROUP BY f.id
                    ORDER BY f.created_at ASC
                )", target_user);
            } else {
                result = txn.exec_params(R"(
                    SELECT f.id, f.name, f.is_public, f.created_at, COUNT(fp.id)
                    FROM favorite_folders f
                    LEFT JOIN favorites fp ON f.id = fp.folder_id
                    WHERE f.user_id = $1 AND f.is_public = TRUE
                    GROUP BY f.id
                    ORDER BY f.created_at ASC
                )", target_user);
            }

            for (const auto& row : result) {
                auto* folder = response->add_folders();
                folder->set_id(row[0].as<int64_t>());
                folder->set_name(row[1].as<std::string>());
                folder->set_is_public(row[2].as<bool>());
                folder->set_created_at(row[3].as<int64_t>());
                folder->set_post_count(row[4].as<int32_t>());
            }
        });

        response->set_code(200);
        response->set_message("Success");
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::SubscribeUser(::trpc::ServerContextPtr context,
                                                 const ::furbbs::SubscribeUserRequest* request,
                                                 ::furbbs::SubscribeUserResponse* response) {
    try {
        auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(request->access_token());
        if (!user_opt) {
            response->set_code(401);
            response->set_message("Invalid token");
            return ::trpc::kSuccStatus;
        }

        if (user_opt->id == request->target_user_id()) {
            response->set_code(400);
            response->set_message("Cannot subscribe to yourself");
            return ::trpc::kSuccStatus;
        }

        db::Database::Instance().Execute([&](pqxx::work& txn) {
            auto existing = txn.exec_params(R"(
                SELECT 1 FROM user_subscriptions 
                WHERE user_id = $1 AND target_user_id = $2
            )", user_opt->id, request->target_user_id());

            if (!existing.empty()) {
                txn.exec_params(R"(
                    DELETE FROM user_subscriptions 
                    WHERE user_id = $1 AND target_user_id = $2
                )", user_opt->id, request->target_user_id());
                response->set_message("Unsubscribed successfully");
            } else {
                txn.exec_params(R"(
                    INSERT INTO user_subscriptions (user_id, target_user_id)
                    VALUES ($1, $2)
                )", user_opt->id, request->target_user_id());
                response->set_message("Subscribed successfully");
            }
        });

        response->set_code(200);
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::GetDailyTasks(::trpc::ServerContextPtr context,
                                                 const ::furbbs::GetDailyTasksRequest* request,
                                                 ::furbbs::GetDailyTasksResponse* response) {
    try {
        auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(request->access_token());
        if (!user_opt) {
            response->set_code(401);
            response->set_message("Invalid token");
            return ::trpc::kSuccStatus;
        }

        auto today = std::chrono::floor<std::chrono::days>(std::chrono::system_clock::now());

        db::Database::Instance().Execute([&](pqxx::work& txn) {
            auto user_result = txn.exec_params(R"(
                SELECT consecutive_checkins, max_consecutive_checkins
                FROM users WHERE id = $1
            )", user_opt->id);

            if (!user_result.empty()) {
                response->set_consecutive_days(user_result[0][0].as<int32_t>());
                response->set_max_consecutive_days(user_result[0][1].as<int32_t>());
            }

            auto result = txn.exec_params(R"(
                SELECT t.id, t.name, t.description, t.reward_points, t.target_value,
                       COALESCE(p.progress, 0), COALESCE(p.is_completed, FALSE)
                FROM daily_tasks t
                LEFT JOIN user_task_progress p ON t.id = p.task_id 
                    AND p.user_id = $1 AND p.task_date = CURRENT_DATE
            )", user_opt->id);

            for (const auto& row : result) {
                auto* task = response->add_tasks();
                task->set_id(row[0].as<int32_t>());
                task->set_name(row[1].as<std::string>());
                task->set_description(row[2].as<std::string>());
                task->set_reward_points(row[3].as<int32_t>());
                task->set_target(row[4].as<int32_t>());
                task->set_progress(row[5].as<int32_t>());
                task->set_is_completed(row[6].as<bool>());
            }
        });

        response->set_code(200);
        response->set_message("Success");
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::SendPrivateMessage(::trpc::ServerContextPtr context,
                                                     const ::furbbs::SendPrivateMessageRequest* request,
                                                     ::furbbs::SendPrivateMessageResponse* response) {
    try {
        auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(request->access_token());
        if (!user_opt) {
            response->set_code(401);
            response->set_message("Invalid token");
            return ::trpc::kSuccStatus;
        }

        if (user_opt->id == request->to_user_id()) {
            response->set_code(400);
            response->set_message("Cannot send message to yourself");
            return ::trpc::kSuccStatus;
        }

        std::string filtered_content = furbbs::common::SensitiveWordFilter::Instance().Filter(
            request->content().substr(0, 2000));

        db::Database::Instance().Execute([&](pqxx::work& txn) {
            txn.exec_params(R"(
                INSERT INTO private_messages (from_user_id, to_user_id, content)
                VALUES ($1, $2, $3)
            )", user_opt->id, request->to_user_id(), filtered_content);
        });

        furbbs::common::NotificationSender::Instance().Send(
            request->to_user_id(), "private_message",
            "收到私信",
            user_opt->username + " 给你发送了一条私信！"
        );

        response->set_code(200);
        response->set_message("Message sent successfully");
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::GetPrivateMessages(::trpc::ServerContextPtr context,
                                                      const ::furbbs::GetPrivateMessagesRequest* request,
                                                      ::furbbs::GetPrivateMessagesResponse* response) {
    try {
        auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(request->access_token());
        if (!user_opt) {
            response->set_code(401);
            response->set_message("Invalid token");
            return ::trpc::kSuccStatus;
        }

        int page = std::max(1, request->page());
        int page_size = std::min(100, std::max(1, request->page_size()));
        int offset = (page - 1) * page_size;

        db::Database::Instance().Execute([&](pqxx::work& txn) {
            auto result = txn.exec_params(R"(
                SELECT id, from_user_id, to_user_id, content, is_read, created_at
                FROM private_messages
                WHERE (from_user_id = $1 AND to_user_id = $2)
                   OR (from_user_id = $2 AND to_user_id = $1)
                ORDER BY created_at DESC
                LIMIT $3 OFFSET $4
            )", user_opt->id, request->with_user_id(), page_size, offset);

            for (const auto& row : result) {
                auto* msg = response->add_messages();
                msg->set_id(row[0].as<int64_t>());
                msg->set_from_user_id(row[1].as<std::string>());
                msg->set_to_user_id(row[2].as<std::string>());
                msg->set_content(row[3].as<std::string>());
                msg->set_is_read(row[4].as<bool>());
                msg->set_created_at(row[5].as<int64_t>());
            }

            auto count_result = txn.exec_params(R"(
                SELECT COUNT(*) FROM private_messages
                WHERE (from_user_id = $1 AND to_user_id = $2)
                   OR (from_user_id = $2 AND to_user_id = $1)
            )", user_opt->id, request->with_user_id());
            response->set_total(count_result[0][0].as<int32_t>());

            auto unread_result = txn.exec_params(R"(
                SELECT COUNT(*) FROM private_messages
                WHERE to_user_id = $1 AND from_user_id = $2 AND is_read = FALSE
            )", user_opt->id, request->with_user_id());
            response->set_unread_count(unread_result[0][0].as<int32_t>());
        });

        response->set_code(200);
        response->set_message("Success");
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::GetMessageConversations(::trpc::ServerContextPtr context,
                                                          const ::furbbs::GetMessageConversationsRequest* request,
                                                          ::furbbs::GetMessageConversationsResponse* response) {
    try {
        auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(request->access_token());
        if (!user_opt) {
            response->set_code(401);
            response->set_message("Invalid token");
            return ::trpc::kSuccStatus;
        }

        db::Database::Instance().Execute([&](pqxx::work& txn) {
            auto result = txn.exec_params(R"(
                WITH conversation_partners AS (
                    SELECT DISTINCT CASE 
                        WHEN from_user_id = $1 THEN to_user_id 
                        ELSE from_user_id 
                    END AS partner_id
                    FROM private_messages
                    WHERE from_user_id = $1 OR to_user_id = $1
                )
                SELECT 
                    cp.partner_id,
                    u.username,
                    u.avatar,
                    (SELECT content FROM private_messages 
                     WHERE (from_user_id = $1 AND to_user_id = cp.partner_id)
                        OR (from_user_id = cp.partner_id AND to_user_id = $1)
                     ORDER BY created_at DESC LIMIT 1) as last_msg,
                    (SELECT created_at FROM private_messages 
                     WHERE (from_user_id = $1 AND to_user_id = cp.partner_id)
                        OR (from_user_id = cp.partner_id AND to_user_id = $1)
                     ORDER BY created_at DESC LIMIT 1) as last_time,
                    (SELECT COUNT(*) FROM private_messages 
                     WHERE to_user_id = $1 AND from_user_id = cp.partner_id AND is_read = FALSE) as unread
                FROM conversation_partners cp
                JOIN users u ON cp.partner_id = u.id
                ORDER BY last_time DESC
            )", user_opt->id);

            for (const auto& row : result) {
                auto* conv = response->add_conversations();
                conv->set_user_id(row[0].as<std::string>());
                conv->set_username(row[1].as<std::string>());
                if (!row[2].is_null()) conv->set_avatar(row[2].as<std::string>());
                if (!row[3].is_null()) conv->set_last_message(row[3].as<std::string>());
                if (!row[4].is_null()) conv->set_last_message_time(row[4].as<int64_t>());
                conv->set_unread_count(row[5].as<int32_t>());
            }
        });

        response->set_code(200);
        response->set_message("Success");
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::MarkMessageRead(::trpc::ServerContextPtr context,
                                                  const ::furbbs::MarkMessageReadRequest* request,
                                                  ::furbbs::MarkMessageReadResponse* response) {
    try {
        auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(request->access_token());
        if (!user_opt) {
            response->set_code(401);
            response->set_message("Invalid token");
            return ::trpc::kSuccStatus;
        }

        db::Database::Instance().Execute([&](pqxx::work& txn) {
            txn.exec_params(R"(
                UPDATE private_messages 
                SET is_read = TRUE 
                WHERE to_user_id = $1 AND from_user_id = $2 AND is_read = FALSE
            )", user_opt->id, request->with_user_id());
        });

        response->set_code(200);
        response->set_message("Messages marked as read");
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::GenerateInviteCode(::trpc::ServerContextPtr context,
                                                     const ::furbbs::GenerateInviteCodeRequest* request,
                                                     ::furbbs::GenerateInviteCodeResponse* response) {
    try {
        auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(request->access_token());
        if (!user_opt) {
            response->set_code(401);
            response->set_message("Invalid token");
            return ::trpc::kSuccStatus;
        }

        int quantity = std::min(10, std::max(1, request->quantity()));

        auto gen_code = []() {
            static const char alphanum[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
            std::string code;
            for (int i = 0; i < 8; ++i) {
                code += alphanum[rand() % (sizeof(alphanum) - 1)];
            }
            return code;
        };

        db::Database::Instance().Execute([&](pqxx::work& txn) {
            for (int i = 0; i < quantity; ++i) {
                std::string code = gen_code();
                txn.exec_params(R"(
                    INSERT INTO invite_codes (code, creator_id)
                    VALUES ($1, $2)
                )", code, user_opt->id);
                response->add_codes(code);
            }
        });

        response->set_code(200);
        response->set_message("Invite codes generated successfully");
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::GetMyInviteCodes(::trpc::ServerContextPtr context,
                                                   const ::furbbs::GetMyInviteCodesRequest* request,
                                                   ::furbbs::GetMyInviteCodesResponse* response) {
    try {
        auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(request->access_token());
        if (!user_opt) {
            response->set_code(401);
            response->set_message("Invalid token");
            return ::trpc::kSuccStatus;
        }

        db::Database::Instance().Execute([&](pqxx::work& txn) {
            auto result = txn.exec_params(R"(
                SELECT code, creator_id, used_by_id, is_used, created_at, used_at, reward_points
                FROM invite_codes
                WHERE creator_id = $1
                ORDER BY created_at DESC
            )", user_opt->id);

            for (const auto& row : result) {
                auto* code = response->add_codes();
                code->set_code(row[0].as<std::string>());
                code->set_creator_id(row[1].as<std::string>());
                if (!row[2].is_null()) code->set_used_by_id(row[2].as<std::string>());
                code->set_is_used(row[3].as<bool>());
                code->set_created_at(row[4].as<int64_t>());
                if (!row[5].is_null()) code->set_used_at(row[5].as<int64_t>());
                code->set_reward_points(row[6].as<int32_t>());
            }
        });

        response->set_code(200);
        response->set_message("Success");
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::GetModerationQueue(::trpc::ServerContextPtr context,
                                                     const ::furbbs::GetModerationQueueRequest* request,
                                                     ::furbbs::GetModerationQueueResponse* response) {
    try {
        auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(request->access_token());
        if (!user_opt || (user_opt->role != "admin" && user_opt->role != "moderator")) {
            response->set_code(403);
            response->set_message("Permission denied");
            return ::trpc::kSuccStatus;
        }

        int page = std::max(1, request->page());
        int page_size = std::min(100, std::max(1, request->page_size()));
        int offset = (page - 1) * page_size;

        db::Database::Instance().Execute([&](pqxx::work& txn) {
            pqxx::result result;
            if (request->status() > 0) {
                result = txn.exec_params(R"(
                    SELECT p.id, p.title, p.content, p.author_id, u.username, p.moderation_status, p.created_at, p.risk_level
                    FROM posts p
                    JOIN users u ON p.author_id = u.id
                    WHERE p.moderation_status = $1
                    ORDER BY p.created_at DESC
                    LIMIT $2 OFFSET $3
                )", request->status(), page_size, offset);
            } else {
                result = txn.exec_params(R"(
                    SELECT p.id, p.title, p.content, p.author_id, u.username, p.moderation_status, p.created_at, p.risk_level
                    FROM posts p
                    JOIN users u ON p.author_id = u.id
                    ORDER BY p.created_at DESC
                    LIMIT $1 OFFSET $2
                )", page_size, offset);
            }

            for (const auto& row : result) {
                auto* post = response->add_posts();
                post->set_post_id(row[0].as<int64_t>());
                post->set_title(row[1].as<std::string>());
                post->set_content(row[2].as<std::string>());
                post->set_author_id(row[3].as<std::string>());
                post->set_author_name(row[4].as<std::string>());
                post->set_status(row[5].as<int32_t>());
                post->set_submitted_at(row[6].as<int64_t>());
                post->set_risk_level(row[7].as<std::string>());
            }

            auto count_result = txn.exec("SELECT COUNT(*) FROM posts WHERE moderation_status = 0");
            response->set_total(count_result[0][0].as<int32_t>());
        });

        response->set_code(200);
        response->set_message("Success");
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::ReviewPost(::trpc::ServerContextPtr context,
                                             const ::furbbs::ReviewPostRequest* request,
                                             ::furbbs::ReviewPostResponse* response) {
    try {
        auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(request->access_token());
        if (!user_opt || (user_opt->role != "admin" && user_opt->role != "moderator")) {
            response->set_code(403);
            response->set_message("Permission denied");
            return ::trpc::kSuccStatus;
        }

        auto now = std::chrono::system_clock::now();
        auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()).count();

        db::Database::Instance().Execute([&](pqxx::work& txn) {
            txn.exec_params(R"(
                UPDATE posts 
                SET moderation_status = $1, reviewed_by = $2, reviewed_at = $3, review_reason = $4
                WHERE id = $5
            )", request->approved() ? 1 : 2, user_opt->id, timestamp, 
                request->reason(), static_cast<int>(request->post_id()));

            auto author_result = txn.exec_params(R"(
                SELECT author_id, title FROM posts WHERE id = $1
            )", static_cast<int>(request->post_id()));
            
            if (!author_result.empty()) {
                std::string author_id = author_result[0][0].as<std::string>();
                std::string title = author_result[0][1].as<std::string>();
                
                if (request->approved()) {
                    furbbs::common::NotificationSender::Instance().Send(
                        author_id, "post_approved",
                        "帖子审核通过",
                        "你的帖子《" + title.substr(0, 20) + "》已通过审核"
                    );
                } else {
                    furbbs::common::NotificationSender::Instance().Send(
                        author_id, "post_rejected",
                        "帖子审核未通过",
                        "你的帖子《" + title.substr(0, 20) + "》未通过审核: " + request->reason()
                    );
                }
            }
        });

        response->set_code(200);
        response->set_message(request->approved() ? "Post approved" : "Post rejected");
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::GetUserActivity(::trpc::ServerContextPtr context,
                                                  const ::furbbs::GetUserActivityRequest* request,
                                                  ::furbbs::GetUserActivityResponse* response) {
    try {
        std::string target_user = request->user_id();
        if (target_user.empty() && !request->access_token().empty()) {
            auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(request->access_token());
            if (user_opt) target_user = user_opt->id;
        }

        if (target_user.empty()) {
            response->set_code(400);
            response->set_message("User ID required");
            return ::trpc::kSuccStatus;
        }

        db::Database::Instance().Execute([&](pqxx::work& txn) {
            auto login_result = txn.exec_params(R"(
                SELECT COUNT(DISTINCT DATE(to_timestamp(created_at/1000)))
                FROM user_activity_logs
                WHERE user_id = $1 AND action_type = 'login'
                  AND created_at > EXTRACT(EPOCH FROM NOW() - INTERVAL '7 days') * 1000
            )", target_user);
            int login_days = login_result[0][0].as<int>();

            auto post_result = txn.exec_params(R"(
                SELECT COUNT(*) FROM user_activity_logs
                WHERE user_id = $1 AND action_type = 'create_post'
                  AND created_at > EXTRACT(EPOCH FROM NOW() - INTERVAL '7 days') * 1000
            )", target_user);
            int post_count = post_result[0][0].as<int>();

            auto comment_result = txn.exec_params(R"(
                SELECT COUNT(*) FROM user_activity_logs
                WHERE user_id = $1 AND action_type = 'create_comment'
                  AND created_at > EXTRACT(EPOCH FROM NOW() - INTERVAL '7 days') * 1000
            )", target_user);
            int comment_count = comment_result[0][0].as<int>();

            auto like_result = txn.exec_params(R"(
                SELECT COUNT(*) FROM user_activity_logs
                WHERE user_id = $1 AND action_type = 'like_post'
                  AND created_at > EXTRACT(EPOCH FROM NOW() - INTERVAL '7 days') * 1000
            )", target_user);
            int like_count = like_result[0][0].as<int>();

            int score = login_days * 10 + post_count * 5 + comment_count * 2 + like_count;
            std::string level = "inactive";
            if (score >= 100) level = "very_active";
            else if (score >= 50) level = "active";
            else if (score >= 20) level = "moderate";

            auto last_active = txn.exec_params(R"(
                SELECT MAX(created_at) FROM user_activity_logs WHERE user_id = $1
            )", target_user);

            auto* activity = response->mutable_activity();
            activity->set_user_id(target_user);
            activity->set_login_days_7d(login_days);
            activity->set_post_count_7d(post_count);
            activity->set_comment_count_7d(comment_count);
            activity->set_like_count_7d(like_count);
            activity->set_activity_score(score);
            activity->set_level(level);
            if (!last_active[0][0].is_null()) {
                activity->set_last_active_time(last_active[0][0].as<int64_t>());
            }
        });

        response->set_code(200);
        response->set_message("Success");
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::GetActivityRanking(::trpc::ServerContextPtr context,
                                                     const ::furbbs::GetActivityRankingRequest* request,
                                                     ::furbbs::GetActivityRankingResponse* response) {
    try {
        int days = std::max(1, std::min(30, request->days()));
        int limit = std::max(1, std::min(100, request->limit() > 0 ? request->limit() : 20));

        db::Database::Instance().Execute([&](pqxx::work& txn) {
            auto result = txn.exec_params(R"(
                SELECT 
                    al.user_id,
                    u.username,
                    u.avatar,
                    COUNT(*) as score
                FROM user_activity_logs al
                JOIN users u ON al.user_id = u.id
                WHERE al.created_at > EXTRACT(EPOCH FROM NOW() - ($1 || ' days')::INTERVAL) * 1000
                GROUP BY al.user_id, u.username, u.avatar
                ORDER BY score DESC
                LIMIT $2
            )", days, limit);

            for (const auto& row : result) {
                auto* rank = response->add_ranks();
                rank->set_user_id(row[0].as<std::string>());
                rank->set_username(row[1].as<std::string>());
                if (!row[2].is_null()) rank->set_avatar(row[2].as<std::string>());
                rank->set_score(row[3].as<int32_t>());
            }
        });

        response->set_code(200);
        response->set_message("Success");
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::DoLuckyDraw(::trpc::ServerContextPtr context,
                                              const ::furbbs::DoLuckyDrawRequest* request,
                                              ::furbbs::DoLuckyDrawResponse* response) {
    try {
        auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(request->access_token());
        if (!user_opt) {
            response->set_code(401);
            response->set_message("Invalid token");
            return ::trpc::kSuccStatus;
        }

        int times = std::min(10, std::max(1, request->times()));
        int cost_per_draw = 20;
        int total_cost = times * cost_per_draw;

        db::Database::Instance().Execute([&](pqxx::work& txn) {
            auto points_result = txn.exec_params(R"(
                SELECT points FROM user_stats WHERE user_id = $1
            )", user_opt->id);

            int user_points = points_result.empty() ? 0 : points_result[0][0].as<int>();
            if (user_points < total_cost) {
                throw std::runtime_error("Insufficient points");
            }

            auto rewards_result = txn.exec(R"(
                SELECT id, name, image, points, rarity, probability
                FROM lucky_draw_rewards WHERE is_available = TRUE
            )");

            std::vector<std::tuple<int, std::string, std::string, int, std::string>> rewards;
            int total_prob = 0;
            for (const auto& row : rewards_result) {
                rewards.emplace_back(
                    row[0].as<int>(),
                    row[1].as<std::string>(),
                    row[2].as<std::string>(),
                    row[3].as<int>(),
                    row[4].as<std::string>()
                );
                total_prob += row[5].as<int>();
            }

            srand(time(nullptr));
            int total_points_awarded = 0;

            for (int t = 0; t < times; ++t) {
                int r = rand() % total_prob;
                int cumulative = 0;
                for (const auto& reward : rewards) {
                    cumulative += std::get<5>(rewards_result[std::get<0>(reward) - 1][5].as<int>());
                    if (r < cumulative) {
                        auto* rew = response->add_rewards();
                        rew->set_id(std::get<0>(reward));
                        rew->set_name(std::get<1>(reward));
                        rew->set_image(std::get<2>(reward));
                        rew->set_points(std::get<3>(reward));
                        rew->set_rarity(std::get<4>(reward));
                        total_points_awarded += std::get<3>(reward);
                        break;
                    }
                }
            }

            txn.exec_params(R"(
                UPDATE user_stats SET points = points - $1 + $2 WHERE user_id = $3
            )", total_cost, total_points_awarded, user_opt->id);

            response->set_points_spent(total_cost);
        });

        response->set_code(200);
        response->set_message("Lucky draw completed");
    } catch (const std::exception& e) {
        response->set_code(400);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::UseCheckinCard(::trpc::ServerContextPtr context,
                                                 const ::furbbs::UseCheckinCardRequest* request,
                                                 ::furbbs::UseCheckinCardResponse* response) {
    try {
        auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(request->access_token());
        if (!user_opt) {
            response->set_code(401);
            response->set_message("Invalid token");
            return ::trpc::kSuccStatus;
        }

        db::Database::Instance().Execute([&](pqxx::work& txn) {
            auto has_card = txn.exec_params(R"(
                SELECT 1 FROM user_inventory ui
                JOIN shop_items si ON ui.item_id = si.id
                WHERE ui.user_id = $1 AND si.type = 'checkin_card' AND ui.is_used = FALSE
                LIMIT 1
            )", user_opt->id);

            if (has_card.empty()) {
                throw std::runtime_error("No checkin card in inventory");
            }

            txn.exec_params(R"(
                UPDATE user_inventory SET is_used = TRUE
                WHERE id = (
                    SELECT ui.id FROM user_inventory ui
                    JOIN shop_items si ON ui.item_id = si.id
                    WHERE ui.user_id = $1 AND si.type = 'checkin_card' AND ui.is_used = FALSE
                    LIMIT 1
                )
            )", user_opt->id);

            txn.exec_params(R"(
                INSERT INTO checkin_records (user_id, checkin_date, is_repaired)
                VALUES ($1, $2, TRUE)
                ON CONFLICT DO NOTHING
            )", user_opt->id, request->target_date());

            auto now = std::chrono::system_clock::now();
            auto today = std::chrono::floor<std::chrono::days>(now);
            
            txn.exec_params(R"(
                UPDATE users SET consecutive_checkins = consecutive_checkins + 1 WHERE id = $1
            )", user_opt->id);

            auto result = txn.exec_params(R"(
                SELECT consecutive_checkins, max_consecutive_checkins FROM users WHERE id = $1
            )", user_opt->id);

            if (!result.empty()) {
                int consecutive = result[0][0].as<int>();
                int max_consecutive = result[0][1].as<int>();
                response->set_new_consecutive_days(consecutive);
                
                if (consecutive > max_consecutive) {
                    txn.exec_params(R"(
                        UPDATE users SET max_consecutive_checkins = $1 WHERE id = $2
                    )", consecutive, user_opt->id);
                }
            }
        });

        response->set_code(200);
        response->set_message("Checkin card used successfully");
    } catch (const std::exception& e) {
        response->set_code(400);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::GetFAQList(::trpc::ServerContextPtr context,
                                             const ::furbbs::GetFAQListRequest* request,
                                             ::furbbs::GetFAQListResponse* response) {
    try {
        db::Database::Instance().Execute([&](pqxx::work& txn) {
            pqxx::result result;
            if (!request->category().empty()) {
                result = txn.exec_params(R"(
                    SELECT id, question, answer, category, sort_order, view_count, is_active, created_at
                    FROM faqs
                    WHERE category = $1 AND is_active = TRUE
                    ORDER BY sort_order ASC, created_at DESC
                )", request->category());
            } else {
                result = txn.exec(R"(
                    SELECT id, question, answer, category, sort_order, view_count, is_active, created_at
                    FROM faqs
                    WHERE is_active = TRUE
                    ORDER BY sort_order ASC, created_at DESC
                )");
            }

            for (const auto& row : result) {
                auto* faq = response->add_faqs();
                faq->set_id(row[0].as<int64_t>());
                faq->set_question(row[1].as<std::string>());
                faq->set_answer(row[2].as<std::string>());
                faq->set_category(row[3].as<std::string>());
                faq->set_sort_order(row[4].as<int32_t>());
                faq->set_view_count(row[5].as<int32_t>());
                faq->set_is_active(row[6].as<bool>());
                faq->set_created_at(row[7].as<int64_t>());
            }

            auto cat_result = txn.exec(R"(
                SELECT DISTINCT category FROM faqs WHERE is_active = TRUE ORDER BY category
            )");
            for (const auto& row : cat_result) {
                response->add_categories(row[0].as<std::string>());
            }
        });

        response->set_code(200);
        response->set_message("Success");
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::ManageFAQ(::trpc::ServerContextPtr context,
                                            const ::furbbs::ManageFAQRequest* request,
                                            ::furbbs::ManageFAQResponse* response) {
    try {
        auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(request->access_token());
        if (!user_opt || user_opt->role != "admin") {
            response->set_code(403);
            response->set_message("Permission denied");
            return ::trpc::kSuccStatus;
        }

        db::Database::Instance().Execute([&](pqxx::work& txn) {
            if (request->action() == "create") {
                txn.exec_params(R"(
                    INSERT INTO faqs (question, answer, category, sort_order, is_active)
                    VALUES ($1, $2, $3, $4, $5)
                )", request->question(), request->answer(), request->category(),
                    request->sort_order(), request->is_active());
            } else if (request->action() == "update") {
                txn.exec_params(R"(
                    UPDATE faqs SET question = $1, answer = $2, category = $3, 
                    sort_order = $4, is_active = $5 WHERE id = $6
                )", request->question(), request->answer(), request->category(),
                    request->sort_order(), request->is_active(), 
                    static_cast<int>(request->faq_id()));
            } else if (request->action() == "delete") {
                txn.exec_params(R"(
                    DELETE FROM faqs WHERE id = $1
                )", static_cast<int>(request->faq_id()));
            }
        });

        response->set_code(200);
        response->set_message("FAQ " + request->action() + "d successfully");
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::GetHelpArticles(::trpc::ServerContextPtr context,
                                                   const ::furbbs::GetHelpArticlesRequest* request,
                                                   ::furbbs::GetHelpArticlesResponse* response) {
    try {
        int page = std::max(1, request->page());
        int page_size = std::min(100, std::max(1, request->page_size() > 0 ? request->page_size() : 20));
        int offset = (page - 1) * page_size;

        db::Database::Instance().Execute([&](pqxx::work& txn) {
            pqxx::result result;
            std::string sql = R"(
                SELECT id, title, content, summary, category, cover_image, 
                       view_count, like_count, is_published, created_at, updated_at
                FROM help_articles WHERE is_published = TRUE
            )";
            std::vector<std::string> params;

            if (!request->category().empty()) {
                sql += " AND category = $" + std::to_string(params.size() + 1);
                params.push_back(request->category());
            }
            if (!request->keyword().empty()) {
                sql += " AND (title ILIKE $" + std::to_string(params.size() + 1) + 
                       " OR content ILIKE $" + std::to_string(params.size() + 1) + ")";
                params.push_back("%" + request->keyword() + "%");
            }
            sql += " ORDER BY created_at DESC LIMIT $" + std::to_string(params.size() + 1) + 
                   " OFFSET $" + std::to_string(params.size() + 2);

            pqxx::prepare::invocation inv = txn.prepare(sql);
            for (const auto& p : params) inv(p);
            result = inv(page_size)(offset).exec();

            for (const auto& row : result) {
                auto* article = response->add_articles();
                article->set_id(row[0].as<int64_t>());
                article->set_title(row[1].as<std::string>());
                article->set_content(row[2].as<std::string>());
                if (!row[3].is_null()) article->set_summary(row[3].as<std::string>());
                article->set_category(row[4].as<std::string>());
                if (!row[5].is_null()) article->set_cover_image(row[5].as<std::string>());
                article->set_view_count(row[6].as<int32_t>());
                article->set_like_count(row[7].as<int32_t>());
                article->set_is_published(row[8].as<bool>());
                article->set_created_at(row[9].as<int64_t>());
                if (!row[10].is_null()) article->set_updated_at(row[10].as<int64_t>());
            }

            auto count_result = txn.exec("SELECT COUNT(*) FROM help_articles WHERE is_published = TRUE");
            response->set_total(count_result[0][0].as<int32_t>());

            auto cat_result = txn.exec(R"(
                SELECT DISTINCT category FROM help_articles WHERE is_published = TRUE ORDER BY category
            )");
            for (const auto& row : cat_result) {
                response->add_categories(row[0].as<std::string>());
            }
        });

        response->set_code(200);
        response->set_message("Success");
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::ManageHelpArticle(::trpc::ServerContextPtr context,
                                                     const ::furbbs::ManageHelpArticleRequest* request,
                                                     ::furbbs::ManageHelpArticleResponse* response) {
    try {
        auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(request->access_token());
        if (!user_opt || user_opt->role != "admin") {
            response->set_code(403);
            response->set_message("Permission denied");
            return ::trpc::kSuccStatus;
        }

        auto now = std::chrono::system_clock::now();
        auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()).count();

        db::Database::Instance().Execute([&](pqxx::work& txn) {
            if (request->action() == "create") {
                txn.exec_params(R"(
                    INSERT INTO help_articles (title, content, summary, category, cover_image, is_published)
                    VALUES ($1, $2, $3, $4, $5, $6)
                )", request->title(), request->content(), 
                    request->summary().empty() ? nullptr : request->summary(),
                    request->category(), 
                    request->cover_image().empty() ? nullptr : request->cover_image(),
                    request->is_published());
            } else if (request->action() == "update") {
                txn.exec_params(R"(
                    UPDATE help_articles SET title = $1, content = $2, summary = $3, 
                    category = $4, cover_image = $5, is_published = $6, updated_at = $7
                    WHERE id = $8
                )", request->title(), request->content(), 
                    request->summary().empty() ? nullptr : request->summary(),
                    request->category(), 
                    request->cover_image().empty() ? nullptr : request->cover_image(),
                    request->is_published(), timestamp,
                    static_cast<int>(request->article_id()));
            } else if (request->action() == "delete") {
                txn.exec_params(R"(
                    DELETE FROM help_articles WHERE id = $1
                )", static_cast<int>(request->article_id()));
            }
        });

        response->set_code(200);
        response->set_message("Article " + request->action() + "d successfully");
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::SubmitFeedback(::trpc::ServerContextPtr context,
                                                  const ::furbbs::SubmitFeedbackRequest* request,
                                                  ::furbbs::SubmitFeedbackResponse* response) {
    try {
        auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(request->access_token());
        if (!user_opt) {
            response->set_code(401);
            response->set_message("Invalid token");
            return ::trpc::kSuccStatus;
        }

        db::Database::Instance().Execute([&](pqxx::work& txn) {
            std::vector<std::string> images(request->images().begin(), request->images().end());
            std::string images_array = "{}";
            if (!images.empty()) {
                images_array = "{";
                for (size_t i = 0; i < images.size(); ++i) {
                    if (i > 0) images_array += ",";
                    images_array += "\"" + images[i] + "\"";
                }
                images_array += "}";
            }

            txn.exec_params(R"(
                INSERT INTO feedbacks (user_id, type, title, content, images, contact)
                VALUES ($1, $2, $3, $4, $5, $6)
            )", user_opt->id, request->type(),
                request->title().substr(0, 200), request->content().substr(0, 5000),
                images.empty() ? nullptr : images_array,
                request->contact().empty() ? nullptr : request->contact());
        });

        response->set_code(200);
        response->set_message("Feedback submitted successfully");
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::GetFeedbackList(::trpc::ServerContextPtr context,
                                                   const ::furbbs::GetFeedbackListRequest* request,
                                                   ::furbbs::GetFeedbackListResponse* response) {
    try {
        auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(request->access_token());
        if (!user_opt || user_opt->role != "admin") {
            response->set_code(403);
            response->set_message("Permission denied");
            return ::trpc::kSuccStatus;
        }

        int page = std::max(1, request->page());
        int page_size = std::min(100, std::max(1, request->page_size() > 0 ? request->page_size() : 20));
        int offset = (page - 1) * page_size;

        db::Database::Instance().Execute([&](pqxx::work& txn) {
            pqxx::result result;
            if (request->status() > 0) {
                result = txn.exec_params(R"(
                    SELECT f.id, f.user_id, u.username, f.type, f.title, f.content, 
                           f.images, f.contact, f.status, f.admin_reply, f.created_at, f.replied_at
                    FROM feedbacks f
                    LEFT JOIN users u ON f.user_id = u.id
                    WHERE f.status = $1
                    ORDER BY f.created_at DESC
                    LIMIT $2 OFFSET $3
                )", request->status(), page_size, offset);
            } else {
                result = txn.exec_params(R"(
                    SELECT f.id, f.user_id, u.username, f.type, f.title, f.content, 
                           f.images, f.contact, f.status, f.admin_reply, f.created_at, f.replied_at
                    FROM feedbacks f
                    LEFT JOIN users u ON f.user_id = u.id
                    ORDER BY f.created_at DESC
                    LIMIT $1 OFFSET $2
                )", page_size, offset);
            }

            for (const auto& row : result) {
                auto* fb = response->add_feedbacks();
                fb->set_id(row[0].as<int64_t>());
                if (!row[1].is_null()) fb->set_user_id(row[1].as<std::string>());
                if (!row[2].is_null()) fb->set_username(row[2].as<std::string>());
                fb->set_type(row[3].as<std::string>());
                fb->set_title(row[4].as<std::string>());
                fb->set_content(row[5].as<std::string>());
                fb->set_contact(row[7].is_null() ? "" : row[7].as<std::string>());
                fb->set_status(row[8].as<int32_t>());
                if (!row[9].is_null()) fb->set_admin_reply(row[9].as<std::string>());
                fb->set_created_at(row[10].as<int64_t>());
                if (!row[11].is_null()) fb->set_replied_at(row[11].as<int64_t>());
            }

            auto count_result = txn.exec("SELECT COUNT(*) FROM feedbacks");
            response->set_total(count_result[0][0].as<int32_t>());
        });

        response->set_code(200);
        response->set_message("Success");
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::ReplyFeedback(::trpc::ServerContextPtr context,
                                                 const ::furbbs::ReplyFeedbackRequest* request,
                                                 ::furbbs::ReplyFeedbackResponse* response) {
    try {
        auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(request->access_token());
        if (!user_opt || user_opt->role != "admin") {
            response->set_code(403);
            response->set_message("Permission denied");
            return ::trpc::kSuccStatus;
        }

        auto now = std::chrono::system_clock::now();
        auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()).count();

        db::Database::Instance().Execute([&](pqxx::work& txn) {
            txn.exec_params(R"(
                UPDATE feedbacks SET admin_reply = $1, status = $2, replied_at = $3
                WHERE id = $4
            )", request->reply(), request->status(), timestamp, 
                static_cast<int>(request->feedback_id()));
        });

        response->set_code(200);
        response->set_message("Feedback replied successfully");
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::GetAdvertisements(::trpc::ServerContextPtr context,
                                                    const ::furbbs::GetAdvertisementsRequest* request,
                                                    ::furbbs::GetAdvertisementsResponse* response) {
    try {
        auto now = std::chrono::system_clock::now();
        auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()).count();

        db::Database::Instance().Execute([&](pqxx::work& txn) {
            pqxx::result result;
            if (!request->position().empty()) {
                result = txn.exec_params(R"(
                    SELECT id, name, position, image_url, link_url, sort_order, 
                           start_time, end_time, is_active, click_count
                    FROM advertisements
                    WHERE position = $1 AND is_active = TRUE
                      AND (start_time IS NULL OR start_time <= $2)
                      AND (end_time IS NULL OR end_time >= $2)
                    ORDER BY sort_order ASC
                )", request->position(), timestamp);
            } else {
                result = txn.exec_params(R"(
                    SELECT id, name, position, image_url, link_url, sort_order, 
                           start_time, end_time, is_active, click_count
                    FROM advertisements
                    WHERE is_active = TRUE
                      AND (start_time IS NULL OR start_time <= $1)
                      AND (end_time IS NULL OR end_time >= $1)
                    ORDER BY sort_order ASC
                )", timestamp);
            }

            for (const auto& row : result) {
                auto* ad = response->add_ads();
                ad->set_id(row[0].as<int64_t>());
                ad->set_name(row[1].as<std::string>());
                ad->set_position(row[2].as<std::string>());
                ad->set_image_url(row[3].as<std::string>());
                if (!row[4].is_null()) ad->set_link_url(row[4].as<std::string>());
                ad->set_sort_order(row[5].as<int32_t>());
                if (!row[6].is_null()) ad->set_start_time(row[6].as<int64_t>());
                if (!row[7].is_null()) ad->set_end_time(row[7].as<int64_t>());
                ad->set_is_active(row[8].as<bool>());
                ad->set_click_count(row[9].as<int32_t>());
            }
        });

        response->set_code(200);
        response->set_message("Success");
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::ManageAdvertisement(::trpc::ServerContextPtr context,
                                                       const ::furbbs::ManageAdvertisementRequest* request,
                                                       ::furbbs::ManageAdvertisementResponse* response) {
    try {
        auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(request->access_token());
        if (!user_opt || user_opt->role != "admin") {
            response->set_code(403);
            response->set_message("Permission denied");
            return ::trpc::kSuccStatus;
        }

        db::Database::Instance().Execute([&](pqxx::work& txn) {
            if (request->action() == "create") {
                txn.exec_params(R"(
                    INSERT INTO advertisements (name, position, image_url, link_url, 
                                               sort_order, start_time, end_time, is_active)
                    VALUES ($1, $2, $3, $4, $5, $6, $7, $8)
                )", request->name(), request->position(), request->image_url(),
                    request->link_url().empty() ? nullptr : request->link_url(),
                    request->sort_order(),
                    request->start_time() > 0 ? request->start_time() : nullptr,
                    request->end_time() > 0 ? request->end_time() : nullptr,
                    request->is_active());
            } else if (request->action() == "update") {
                txn.exec_params(R"(
                    UPDATE advertisements SET name = $1, position = $2, image_url = $3, 
                    link_url = $4, sort_order = $5, start_time = $6, end_time = $7, is_active = $8
                    WHERE id = $9
                )", request->name(), request->position(), request->image_url(),
                    request->link_url().empty() ? nullptr : request->link_url(),
                    request->sort_order(),
                    request->start_time() > 0 ? request->start_time() : nullptr,
                    request->end_time() > 0 ? request->end_time() : nullptr,
                    request->is_active(), static_cast<int>(request->ad_id()));
            } else if (request->action() == "delete") {
                txn.exec_params(R"(
                    DELETE FROM advertisements WHERE id = $1
                )", static_cast<int>(request->ad_id()));
            }
        });

        response->set_code(200);
        response->set_message("Advertisement " + request->action() + "d successfully");
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::GetFriendLinks(::trpc::ServerContextPtr context,
                                                 const ::furbbs::GetSystemMetricsResponse* request,
                                                 ::furbbs::GetFriendLinksResponse* response) {
    try {
        db::Database::Instance().Execute([&](pqxx::work& txn) {
            auto result = txn.exec(R"(
                SELECT id, name, url, logo, description, sort_order, is_active, created_at
                FROM friend_links
                WHERE is_active = TRUE
                ORDER BY sort_order ASC, created_at DESC
            )");

            for (const auto& row : result) {
                auto* link = response->add_links();
                link->set_id(row[0].as<int64_t>());
                link->set_name(row[1].as<std::string>());
                link->set_url(row[2].as<std::string>());
                if (!row[3].is_null()) link->set_logo(row[3].as<std::string>());
                if (!row[4].is_null()) link->set_description(row[4].as<std::string>());
                link->set_sort_order(row[5].as<int32_t>());
                link->set_is_active(row[6].as<bool>());
                link->set_created_at(row[7].as<int64_t>());
            }
        });

        response->set_code(200);
        response->set_message("Success");
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::ManageFriendLink(::trpc::ServerContextPtr context,
                                                   const ::furbbs::ManageFriendLinkRequest* request,
                                                   ::furbbs::ManageFriendLinkResponse* response) {
    try {
        auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(request->access_token());
        if (!user_opt || user_opt->role != "admin") {
            response->set_code(403);
            response->set_message("Permission denied");
            return ::trpc::kSuccStatus;
        }

        db::Database::Instance().Execute([&](pqxx::work& txn) {
            if (request->action() == "create") {
                txn.exec_params(R"(
                    INSERT INTO friend_links (name, url, logo, description, sort_order, is_active)
                    VALUES ($1, $2, $3, $4, $5, $6)
                )", request->name(), request->url(),
                    request->logo().empty() ? nullptr : request->logo(),
                    request->description().empty() ? nullptr : request->description(),
                    request->sort_order(), request->is_active());
            } else if (request->action() == "update") {
                txn.exec_params(R"(
                    UPDATE friend_links SET name = $1, url = $2, logo = $3, 
                    description = $4, sort_order = $5, is_active = $6
                    WHERE id = $7
                )", request->name(), request->url(),
                    request->logo().empty() ? nullptr : request->logo(),
                    request->description().empty() ? nullptr : request->description(),
                    request->sort_order(), request->is_active(),
                    static_cast<int>(request->link_id()));
            } else if (request->action() == "delete") {
                txn.exec_params(R"(
                    DELETE FROM friend_links WHERE id = $1
                )", static_cast<int>(request->link_id()));
            }
        });

        response->set_code(200);
        response->set_message("Friend link " + request->action() + "d successfully");
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status FurBBSServiceImpl::GetStatistics(::trpc::ServerContextPtr context,
                                                 const ::furbbs::GetSystemMetricsResponse* request,
                                                 ::furbbs::GetStatisticsResponse* response) {
    try {
        db::Database::Instance().Execute([&](pqxx::work& txn) {
            auto* stats = response->mutable_statistics();

            auto users_result = txn.exec("SELECT COUNT(*) FROM users");
            stats->set_total_users(users_result[0][0].as<int32_t>());

            auto today_users_result = txn.exec(R"(
                SELECT COUNT(*) FROM users 
                WHERE created_at > EXTRACT(EPOCH FROM CURRENT_DATE) * 1000
            )");
            stats->set_today_users(today_users_result[0][0].as<int32_t>());

            auto posts_result = txn.exec("SELECT COUNT(*) FROM posts");
            stats->set_total_posts(posts_result[0][0].as<int32_t>());

            auto today_posts_result = txn.exec(R"(
                SELECT COUNT(*) FROM posts 
                WHERE created_at > EXTRACT(EPOCH FROM CURRENT_DATE) * 1000
            )");
            stats->set_today_posts(today_posts_result[0][0].as<int32_t>());

            auto comments_result = txn.exec("SELECT COUNT(*) FROM comments");
            stats->set_total_comments(comments_result[0][0].as<int32_t>());

            auto today_comments_result = txn.exec(R"(
                SELECT COUNT(*) FROM comments 
                WHERE created_at > EXTRACT(EPOCH FROM CURRENT_DATE) * 1000
            )");
            stats->set_today_comments(today_comments_result[0][0].as<int32_t>());

            auto online_result = txn.exec(R"(
                SELECT COUNT(DISTINCT user_id) FROM user_activity_logs
                WHERE created_at > EXTRACT(EPOCH FROM NOW() - INTERVAL '5 minutes') * 1000
            )");
            stats->set_online_users(online_result[0][0].as<int32_t>());

            auto daily_result = txn.exec(R"(
                SELECT 
                    DATE(to_timestamp(created_at/1000)) as dt,
                    COUNT(*) FILTER (WHERE tableoid::regclass::text = 'users') as regs,
                    COUNT(*) FILTER (WHERE tableoid::regclass::text = 'posts') as posts,
                    COUNT(*) FILTER (WHERE tableoid::regclass::text = 'comments') as comments
                FROM (
                    SELECT created_at, 'users' as tbl FROM users
                    UNION ALL
                    SELECT created_at, 'posts' as tbl FROM posts
                    UNION ALL
                    SELECT created_at, 'comments' as tbl FROM comments
                ) t
                WHERE created_at > EXTRACT(EPOCH FROM NOW() - INTERVAL '7 days') * 1000
                GROUP BY dt
                ORDER BY dt ASC
            )");

            for (const auto& row : daily_result) {
                response->add_daily_registrations(row[1].as<int32_t>());
                response->add_daily_posts(row[2].as<int32_t>());
                response->add_daily_comments(row[3].as<int32_t>());
                response->add_date_labels(row[0].as<std::string>());
            }
        });

        response->set_code(200);
        response->set_message("Success");
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(e.what());
    }
    return ::trpc::kSuccStatus;
}

} // namespace furbbs::service
