#include <vector>
#include <string>
#include <crow.h>
#include <nlohmann/json.hpp>
#include <algorithm>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <regex>
#include "User.h"
#include "Utils.h"

// Vérifier si l'authentification est désactivée
#ifndef DISABLE_AUTH
#include "JWTAuthMiddleware.h"
#include "CookieParser.h"
#define AUTH_ENABLED 1
#else
#define AUTH_ENABLED 0
#endif

// Conversion entre User et JSON
namespace nlohmann {
    template <>
    struct adl_serializer<User> {
        static void to_json(json& j, const User& user) {
            j = json{
                {"id", user.id},
                {"nom", user.nom},
                {"email", user.email},
                {"password_hash", user.password_hash},
                {"role", user.role},
                {"date_creation", user.date_creation}
            };
        }

        static void from_json(const json& j, User& user) {
            j.at("id").get_to(user.id);
            j.at("nom").get_to(user.nom);
            j.at("email").get_to(user.email);
            j.at("password_hash").get_to(user.password_hash);
            j.at("role").get_to(user.role);
            j.at("date_creation").get_to(user.date_creation);
        }
    };
}

using json = nlohmann::json;

namespace crowjourney {

    // Fonction utilitaire pour hasher un mot de passe avec SHA-256
    std::string hashPassword(const std::string& password) {
        // Solution temporaire - à remplacer par une méthode plus sécurisée
        std::hash<std::string> hasher;
        size_t hash = hasher(password);
        std::stringstream ss;
        ss << std::hex << hash;
        return ss.str();
    }

    // Fonction utilitaire pour valider un email avec regex
    bool isValidEmail(const std::string& email) {
        // Regex simple pour valider les emails
        const std::regex pattern("(\\w+)(\\.|_)?(\\w*)@(\\w+)(\\.(\\w+))+");
        return std::regex_match(email, pattern);
    }

    // Fonction utilitaire pour valider la force d'un mot de passe
    bool isStrongPassword(const std::string& password) {
        // Au moins 8 caractères, une majuscule, une minuscule et un chiffre
        if (password.size() < 8) return false;

        bool hasUpper = false;
        bool hasLower = false;
        bool hasDigit = false;

        for (char c : password) {
            if (std::isupper(c)) hasUpper = true;
            if (std::islower(c)) hasLower = true;
            if (std::isdigit(c)) hasDigit = true;
        }

        return hasUpper && hasLower && hasDigit;
    }

    // Fonction pour obtenir la date et l'heure actuelles formatées
    std::string getCurrentDateTime() {
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);

        std::stringstream ss;
        ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %H:%M:%S");
        return ss.str();
    }

    class UserManager {
    private:
        std::vector<User> users_;
        int next_id_;
        std::string filename_; // Fichier pour sauvegarder les utilisateurs

    public:
        // Constructeur
        UserManager(const std::string& filename = "users.json") : next_id_(1), filename_(filename) {
            loadUsers(); // Charger les utilisateurs depuis le fichier
        }

        // Charger les utilisateurs depuis un fichier JSON
        void loadUsers() {
            std::ifstream file(filename_);
            if (file.is_open()) {
                try {
                    json data = json::parse(file);
                    users_ = data["users"].get<std::vector<User>>();

                    // Trouver le plus grand ID pour initialiser next_id_
                    for (const auto& user : users_) {
                        if (user.id >= next_id_) {
                            next_id_ = user.id + 1;
                        }
                    }
                }
                catch (const std::exception& e) {
                    std::cerr << "Erreur lors du chargement des utilisateurs: " << e.what() << std::endl;
                }
                file.close();
            }
        }

        // Sauvegarder les utilisateurs dans un fichier JSON
        void saveUsers() {
            json data;
            data["users"] = users_;

            std::ofstream file(filename_);
            if (file.is_open()) {
                file << data.dump(4); // Indentation de 4 espaces pour lisibilité
                file.close();
            }
            else {
                std::cerr << "Impossible d'ouvrir le fichier pour sauvegarder les utilisateurs" << std::endl;
            }
        }

        // Récupérer tous les utilisateurs (sans les mots de passe)
        std::vector<User> getUsers() const {
            std::vector<User> safeUsers = users_;
            for (auto& user : safeUsers) {
                user.password_hash = ""; // Ne pas renvoyer les hash de mots de passe
            }
            return safeUsers;
        }

        // Trouver un utilisateur par email
        User* getUserByEmail(const std::string& email) {
            for (auto& user : users_) {
                if (user.email == email) {
                    return &user;
                }
            }
            return nullptr;
        }

        // Trouver un utilisateur par ID
        User* getUserById(int id) {
            for (auto& user : users_) {
                if (user.id == id) {
                    return &user;
                }
            }
            return nullptr;
        }

        // Créer un nouvel utilisateur
        std::pair<User, std::string> createUser(const std::string& nom, const std::string& email,
            const std::string& password, const std::string& role = "user") {
            // Valider l'email
            if (!isValidEmail(email)) {
                return { User(), "Format d'email invalide" };
            }

            // Vérifier si l'email existe déjà
            if (getUserByEmail(email) != nullptr) {
                return { User(), "Cet email est déjà utilisé" };
            }

            // Valider le mot de passe
            if (!isStrongPassword(password)) {
                return { User(), "Le mot de passe doit contenir au moins 8 caractères, une majuscule, une minuscule et un chiffre" };
            }

            // Créer le nouvel utilisateur
            User newUser;
            newUser.id = next_id_++;
            newUser.nom = nom;
            newUser.email = email;
            newUser.password_hash = hashPassword(password);
            newUser.role = role;
            newUser.date_creation = getCurrentDateTime();

            users_.push_back(newUser);

            // Sauvegarder les changements
            saveUsers();

            // Créer une copie sans mot de passe pour le retour
            User safeUser = newUser;
            safeUser.password_hash = "";

            return { safeUser, "Utilisateur créé avec succès" };
        }

        // Convertir un utilisateur en JSON (sans le mot de passe)
        json userToJson(const User& user) const {
            return {
                {"id", user.id},
                {"nom", user.nom},
                {"email", user.email},
                {"role", user.role},
                {"date_creation", user.date_creation}
                // Ne pas inclure le password_hash !
            };
        }

        // Authentifier un utilisateur avec email et mot de passe
        User* authenticateUser(const std::string& email, const std::string& password) {
            User* user = getUserByEmail(email);
            if (user && user->password_hash == hashPassword(password)) {
                return user;
            }
            return nullptr;
        }
    };

    // Instance singleton du gestionnaire d'utilisateurs
    UserManager& getUserManager() {
        static UserManager instance;
        return instance;
    }

    // API Handlers pour les utilisateurs

    // POST /users - Créer un nouvel utilisateur
    crow::response registerUser(const crow::request& req) {
        auto& userManager = getUserManager();

        try {
            auto body = json::parse(req.body);

            // Vérifier les champs obligatoires
            if (!body.contains("nom") || !body.contains("email") || !body.contains("password")) {
                auto response = crow::response(400, R"({"error": "Le nom, l'email et le mot de passe sont obligatoires"})");
                response.add_header("Content-Type", "application/json; charset=utf-8");
                addCorsHeaders(response);
                return std::move(response);
            }

            std::string nom = body["nom"].get<std::string>();
            std::string email = body["email"].get<std::string>();
            std::string password = body["password"].get<std::string>();
            std::string role = body.contains("role") ? body["role"].get<std::string>() : "user";

            // Créer l'utilisateur
            auto [user, message] = userManager.createUser(nom, email, password, role);

            if (user.id == 0) {
                // Erreur lors de la création
                auto response = crow::response(400, json{ {"error", message} }.dump());
                response.add_header("Content-Type", "application/json; charset=utf-8");
                addCorsHeaders(response);
                return std::move(response);
            }

            // Succès
            json response_body = userManager.userToJson(user);
            response_body["message"] = message;

            auto response = crow::response(201, response_body.dump());
            response.add_header("Content-Type", "application/json; charset=utf-8");
            addCorsHeaders(response);
            return std::move(response);
        }
        catch (const std::exception& e) {
            auto response = crow::response(400, json{ {"error", std::string("Erreur de format JSON: ") + e.what()} }.dump());
            response.add_header("Content-Type", "application/json; charset=utf-8");
            addCorsHeaders(response);
            return std::move(response);
        }
    }

    // GET /users - Récupérer tous les utilisateurs (pour les administrateurs)
    crow::response getAllUsers() {
        auto& userManager = getUserManager();

        json result = json::array();
        for (const auto& user : userManager.getUsers()) {
            result.push_back(userManager.userToJson(user));
        }

        auto response = crow::response(200, result.dump());
        response.add_header("Content-Type", "application/json; charset=utf-8");
        addCorsHeaders(response);
        return std::move(response);
    }

    // Gestionnaire pour les préflight OPTIONS (nécessaire pour CORS)
    crow::response handleUserOptions() {
        auto response = crow::response(204);
        response.add_header("Content-Type", "application/json; charset=utf-8");
        addCorsHeaders(response);
        return std::move(response);
    }

    // Configuration des routes utilisateurs avec support pour les deux modes
#if AUTH_ENABLED
    void setup_user_routes(crow::App<crow::CookieParser, JWTAuthMiddleware>& app) {
#else
    void setup_user_routes(crow::SimpleApp & app) {
        // Ajouter une route OPTIONS pour gérer les préflight CORS
        CROW_ROUTE(app, "/users")
            .methods(crow::HTTPMethod::OPTIONS)(handleUserOptions);
#endif
        // POST /users - Créer un nouvel utilisateur
        CROW_ROUTE(app, "/users")
            .methods(crow::HTTPMethod::POST)(registerUser);

        // GET /users - Récupérer tous les utilisateurs (admin)
        CROW_ROUTE(app, "/users")
            .methods(crow::HTTPMethod::GET)(getAllUsers);
    }

    // Configuration des routes d'authentification avec support pour les deux modes
#if AUTH_ENABLED
    void setup_auth_routes(crow::App<crow::CookieParser, JWTAuthMiddleware>&app, JWTAuthMiddleware & jwtMiddleware) {
        CROW_ROUTE(app, "/login")
            .methods(crow::HTTPMethod::POST)([&jwtMiddleware](const crow::request& req) {
            auto& userManager = getUserManager();

            try {
                auto body = json::parse(req.body);

                // Vérifier les champs obligatoires
                if (!body.contains("email") || !body.contains("password")) {
                    auto response = crow::response(400, R"({"error": "L'email et le mot de passe sont obligatoires"})");
                    response.add_header("Content-Type", "application/json; charset=utf-8");
                    addCorsHeaders(response);
                    return std::move(response);
                }

                std::string email = body["email"].get<std::string>();
                std::string password = body["password"].get<std::string>();

                // Authentifier l'utilisateur avec la fonction spécifique
                User* user = userManager.authenticateUser(email, password);
                if (!user) {
                    auto response = crow::response(401, R"({"error": "Email ou mot de passe incorrect"})");
                    response.add_header("Content-Type", "application/json; charset=utf-8");
                    addCorsHeaders(response);
                    return std::move(response);
                }

                // Création du JWT avec l'instance passée en paramètre
                std::string token = jwtMiddleware.generateToken(*user);

                json response_body = {
                    {"token", token},
                    {"user", userManager.userToJson(*user)},
                    {"message", "Connexion réussie"}
                };

                auto response = crow::response(200, response_body.dump());
                response.add_header("Content-Type", "application/json; charset=utf-8");
                addCorsHeaders(response);
                return std::move(response);
            }
            catch (const std::exception& e) {
                auto response = crow::response(400, json{ {"error", std::string("Erreur de format JSON: ") + e.what()} }.dump());
                response.add_header("Content-Type", "application/json; charset=utf-8");
                addCorsHeaders(response);
                return std::move(response);
            }
                });
#else
    void setup_auth_routes(crow::SimpleApp & app) {
        // Ajouter une route OPTIONS pour gérer les préflight CORS
        CROW_ROUTE(app, "/login")
            .methods(crow::HTTPMethod::OPTIONS)(handleUserOptions);

        CROW_ROUTE(app, "/login")
            .methods(crow::HTTPMethod::POST)([](const crow::request& req) {
            // Mode développement: simuler une connexion réussie
            json response_body = {
                {"token", "dev-mode-token"},
                {"user", {
                    {"id", 1},
                    {"nom", "Utilisateur de test"},
                    {"email", "dev@example.com"},
                    {"role", "admin"},
                    {"date_creation", getCurrentDateTime()}
                }},
                {"message", "Connexion réussie (mode développement)"}
            };

            auto response = crow::response(200, response_body.dump());
            response.add_header("Content-Type", "application/json; charset=utf-8");
            addCorsHeaders(response);
            return std::move(response);
                });
#endif
    }
    }
