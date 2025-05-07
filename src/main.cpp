#include <iostream>
#include <crow.h>
#include "library.h"

// Directive de préprocesseur pour choisir le mode (avec ou sans authentification)
// Définissez cette macro dans les options de compilation pour activer le mode sans authentification
// Par défaut, le mode avec authentification est utilisé
#ifndef DISABLE_AUTH
#include "JWTAuthMiddleware.h"
#include "CookieParser.h" // Inclure explicitement CookieParser
#define AUTH_ENABLED 1
#else
#include <crow/middlewares/cors.h> // Pour le CORS dans le mode sans auth
#define AUTH_ENABLED 0
#endif

// Déclarations des fonctions avec les signatures correctes
namespace crowjourney {
    void initialize();

#if AUTH_ENABLED
    void setup_routes(crow::App<crow::CookieParser, JWTAuthMiddleware>& app);
    void setup_user_routes(crow::App<crow::CookieParser, JWTAuthMiddleware>& app);
    void setup_auth_routes(crow::App<crow::CookieParser, JWTAuthMiddleware>& app, JWTAuthMiddleware& jwtMiddleware);
#else
    void setup_routes(crow::SimpleApp& app);
    void setup_user_routes(crow::SimpleApp& app);
    void setup_auth_routes(crow::SimpleApp& app);  // Sans paramètre jwtMiddleware
#endif
}


int main() {
#if AUTH_ENABLED
    std::cout << "Starting REST API server with authentication..." << std::endl;

    // Créer une application Crow avec JWT middleware
    crow::App<crow::CookieParser, crowjourney::JWTAuthMiddleware> app;

    // Initialiser la bibliothèque
    crowjourney::initialize();

    // Configurer le middleware JWT avec une clé secrète plus sécurisée
    auto& jwtMiddleware = app.get_middleware<crowjourney::JWTAuthMiddleware>();
    jwtMiddleware = crowjourney::JWTAuthMiddleware("votre_clé_secrète_très_longue_et_complexe", "crowjourney_api", 24);

    // Configurer les routes de la bibliothèque
    crowjourney::setup_routes(app);

    // Configuration des routes utilisateur
    crowjourney::setup_user_routes(app);

    // Configuration des routes d'authentification
    // Passer l'instance du middleware JWT
    crowjourney::setup_auth_routes(app, jwtMiddleware);

    // Ajouter d'autres routes protégées
    CROW_ROUTE(app, "/protected")
        ([&](const crow::request& req) {
        // Vérifier si l'utilisateur est authentifié
        auto& ctx = app.get_context<crowjourney::JWTAuthMiddleware>(req);

        if (!ctx.authenticated) {
            crow::json::wvalue result;
            result["status"] = "error";
            result["message"] = "Accès non autorisé";
            return crow::response(401, result);
        }

        // L'utilisateur est authentifié, renvoyer une réponse
        crow::json::wvalue result;
        result["status"] = "success";
        result["message"] = "Accès autorisé";
        result["userId"] = ctx.userId;
        result["role"] = ctx.role;

        return crow::response(200, result);
            });

    // Route admin protégée par rôle
    CROW_ROUTE(app, "/admin")
        ([&](const crow::request& req) {
        auto& ctx = app.get_context<crowjourney::JWTAuthMiddleware>(req);

        if (!ctx.authenticated || ctx.role != "admin") {
            crow::json::wvalue result;
            result["status"] = "error";
            result["message"] = "Accès réservé aux administrateurs";
            return crow::response(403, result);
        }

        // L'utilisateur est un administrateur
        crow::json::wvalue result;
        result["status"] = "success";
        result["message"] = "Bienvenue, administrateur!";

        return crow::response(200, result);
            });

    // Ajouter une route racine pour faciliter les tests
    CROW_ROUTE(app, "/")
        ([]() {
        return "Bienvenue sur l'API CrowJourney - API REST avec authentification JWT";
            });

    // Lancer le serveur sur le port 8080
    std::cout << "Server starting on port 8080..." << std::endl;
    //app.port(8080).multithreaded().run();
    app.port(8080).bindaddr("0.0.0.0").multithreaded().run();
#else
    std::cout << "Starting REST API server WITHOUT authentication (development mode)..." << std::endl;

    // Créer une application Crow simple sans middleware JWT
    crow::SimpleApp app;

    // Initialiser la bibliothèque
    crowjourney::initialize();

    // En mode sans authentification, nous n'ajoutons pas de middleware CORS
    // mais nous ajoutons les en-têtes CORS à chaque réponse (c'est déjà fait dans vos fonctions)

    // Configurer les routes de la bibliothèque (sans authentification)
    crowjourney::setup_routes(app);

    // Configurer les routes utilisateur et d'authentification
    crowjourney::setup_user_routes(app);
    crowjourney::setup_auth_routes(app); // Pas de paramètre jwtMiddleware en mode sans auth

    // Ajouter une route racine pour faciliter les tests
    CROW_ROUTE(app, "/")
        ([]() {
        return "Bienvenue sur l'API CrowJourney - Version développement sans authentification";
            });

    // Lancer le serveur sur le port 8080
    std::cout << "Server starting on port 8080 (development mode)..." << std::endl;
    app.port(8080).bindaddr("0.0.0.0").multithreaded().run();
#endif

    return 0;
}
