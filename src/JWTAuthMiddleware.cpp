#include <iostream>
#include <crow.h>
#include "library.h"
#include "JWTAuthMiddleware.h"
#include "CookieParser.h" 
#include <stdexcept>

namespace crowjourney {

    // Constructeur
    JWTAuthMiddleware::JWTAuthMiddleware(const std::string& secret,
        const std::string& issuer,
        int expiration_hours)
        : secret_(secret), issuer_(issuer), expiration_hours_(expiration_hours) {
    }

    // Méthode appelée avant le traitement de la requête
    void JWTAuthMiddleware::before_handle(crow::request& req, crow::response& res, context& ctx)
    {
        // Ignorer certaines routes sans authentification 
        // (par exemple, login et création d'utilisateur)
        if (req.url == "/login" ||
            (req.url == "/users" && req.method == crow::HTTPMethod::POST)) {
            return;
        }

        // Récupérer le token du header Authorization
        std::string auth_header = req.get_header_value("Authorization");
        if (auth_header.empty() || auth_header.substr(0, 7) != "Bearer ") {
            res = unauthorized("Authentification requise");
            return;
        }

        std::string token = auth_header.substr(7);
        try {
            // Vérifier et décoder le token
            auto decoded = jwt::decode(token);
            auto verifier = jwt::verify()
                .allow_algorithm(jwt::algorithm::hs256{ secret_ })
                .with_issuer(issuer_);

            verifier.verify(decoded);

            // Extraire les claims du token
            ctx.userId = decoded.get_payload_claim("sub").as_string();
            ctx.role = decoded.get_payload_claim("role").as_string();
            ctx.authenticated = true;

        }
        catch (const std::exception& e) {
            res = unauthorized("Token invalide: " + std::string(e.what()));
        }
    }

    // Méthode appelée après le traitement de la requête
    void JWTAuthMiddleware::after_handle(crow::request& /*req*/, crow::response& /*res*/, context& /*ctx*/)
    {
        // Aucune action nécessaire après le traitement
    }

    // Générer un token JWT pour un utilisateur
    std::string JWTAuthMiddleware::generateToken(const User& user) {
        auto now = std::chrono::system_clock::now();
        auto expiration = now + std::chrono::hours(expiration_hours_);

        return jwt::create()
            .set_issuer(issuer_)
            .set_issued_at(now)
            .set_expires_at(expiration)
            .set_subject(std::to_string(user.id))
            .set_payload_claim("role", jwt::claim(user.role))
            .set_payload_claim("email", jwt::claim(user.email))
            .set_payload_claim("nom", jwt::claim(user.nom))
            .sign(jwt::algorithm::hs256{ secret_ });
    }

    bool JWTAuthMiddleware::hasRole(const crow::request& req, const std::string& role) {
        
		// On ne peut pas utiliser get_context directement ici car c'est une méthode template
        if (!req.middleware_context) {
            return false;
        }

		// On suppose que le middleware_context contient notre contexte
        try {
            auto* ctx = static_cast<context*>(req.middleware_context);
            return ctx && ctx->authenticated && ctx->role == role;
        }
        catch (...) {
            return false; // Gestion d'erreur supplémentaire
        }
    }



    // Créer une réponse "unauthorized"
    crow::response JWTAuthMiddleware::unauthorized(const std::string& message) {
        crow::json::wvalue result;
        result["status"] = "error";
        result["message"] = message;
        crow::response res(401, result);
        res.add_header("Content-Type", "application/json; charset=utf-8");
        return res;
    }

} // namespace crowjourney
