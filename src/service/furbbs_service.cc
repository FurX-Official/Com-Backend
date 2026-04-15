#include "furbbs_service.h"
#include <spdlog/spdlog.h>
#include <atomic>
#include "../db/database.h"
#include "../auth/casdoor_auth.h"
#include "../common/security.h"

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

} // namespace furbbs::service
