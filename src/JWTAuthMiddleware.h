#pragma once

#include <string>
#include <crow.h>
#include <jwt-cpp/jwt.h>
#include "User.h"
#include <chrono>

namespace crowjourney {

    class JWTAuthMiddleware : public crow::ILocalMiddleware
    {
    public:
        struct context
        {
            std::string userId;
            std::string role;
            bool authenticated{ false };
        };

        // Constructeur
        JWTAuthMiddleware(const std::string& secret = "your_super_secret_key_change_me",
            const std::string& issuer = "crowjourney_api",
            int expiration_hours = 24);

        // Méthodes de gestion des requêtes
        void before_handle(crow::request& req, crow::response& res, context& ctx);
        void after_handle(crow::request& req, crow::response& res, context& ctx);

        // Générer un token JWT pour un utilisateur
        std::string generateToken(const User& user);

        // Vérifier si l'utilisateur a un rôle spécifique
        static bool hasRole(const crow::request& req, const std::string& role);

    private:
        std::string secret_;
        std::string issuer_;
        int expiration_hours_;

        // Créer une réponse "unauthorized"
        crow::response unauthorized(const std::string& message);
    };

    // Fonction utilitaire pour obtenir le contexte JWT de la requête
    template<typename App>
    JWTAuthMiddleware::context& get_jwt_context(const crow::request& req) {
        return App::template get_context<JWTAuthMiddleware>(req);
    }

    // Fonction utilitaire pour vérifier si l'utilisateur est authentifié
    template<typename App>
    bool is_authenticated(const crow::request& req) {
        return get_jwt_context<App>(req).authenticated;
    }

} // namespace crowjourney