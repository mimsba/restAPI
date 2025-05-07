// Implémentation de la bibliothèque crowJourney
#include "library.h"
#include "Utils.h"
#include <iostream>
#include <algorithm>
#include <stdexcept>
#include <memory>

// Implémentation de la sérialisation JSON pour Livre
namespace nlohmann {
    template <>
    struct adl_serializer<Livre> {
        static void to_json(json& j, const Livre& livre) {
            j = json{
                {"id", livre.id},
                {"titre", livre.titre},
                {"auteur", livre.auteur},
                {"annee", livre.annee},
                {"genre", livre.genre}
            };
        }

        static void from_json(const json& j, Livre& livre) {
            j.at("id").get_to(livre.id);
            j.at("titre").get_to(livre.titre);
            j.at("auteur").get_to(livre.auteur);
            j.at("annee").get_to(livre.annee);
            j.at("genre").get_to(livre.genre);
        }
    };
}

namespace crowjourney {

    // Constructeur de BibliothequeManager
    BibliothequeManager::BibliothequeManager(const std::string& filename)
        : next_id_(4), filename_(filename) {
        // Initialisation des livres par défaut
        livres_ = {
            {"Le Petit Prince", "Antoine de Saint-Exupéry", 1943, "Fiction", 1},
            {"L'Étranger", "Albert Camus", 1942, "Philosophie", 2},
            {"Les Misérables", "Victor Hugo", 1862, "Roman historique", 3}
        };

        // Essayer de charger depuis le fichier (si existe)
        loadBooks();
    }

    // Sauvegarder les livres dans un fichier JSON
    void BibliothequeManager::saveBooks() {
        json data;
        data["livres"] = livres_;

        std::ofstream file(filename_, std::ios::binary);
        if (file.is_open()) {
            file << data.dump(4);
            file.close();
        }
        else {
            std::cerr << "Impossible d'ouvrir le fichier pour sauvegarder les livres" << std::endl;
        }
    }

    // Charger les livres depuis un fichier JSON
    void BibliothequeManager::loadBooks() {
        std::ifstream file(filename_, std::ios::binary);
        if (file.is_open()) {
            try {
                json data = json::parse(file);
                if (data.contains("livres")) {
                    livres_ = data["livres"].get<std::vector<Livre>>();

                    // Trouver le plus grand ID
                    for (const auto& livre : livres_) {
                        if (livre.id >= next_id_) {
                            next_id_ = livre.id + 1;
                        }
                    }
                }
            }
            catch (const std::exception& e) {
                std::cerr << "Erreur lors du chargement des livres: " << e.what() << std::endl;
            }
            file.close();
        }
    }

    // Récupérer tous les livres
    const std::vector<Livre>& BibliothequeManager::getTousLivres() const {
        return livres_;
    }

    // Récupérer un livre par son ID
    Livre* BibliothequeManager::getLivreParId(int id) {
        for (auto& livre : livres_) {
            if (livre.id == id) {
                return &livre;
            }
        }
        return nullptr;
    }

    // Ajouter un nouveau livre
    Livre BibliothequeManager::ajouterLivre(const std::string& titre, const std::string& auteur,
        int annee, const std::string& genre) {
        Livre nouveau_livre;
        nouveau_livre.id = next_id_++;
        nouveau_livre.titre = titre;
        nouveau_livre.auteur = auteur;
        nouveau_livre.annee = annee;
        nouveau_livre.genre = genre;

        livres_.push_back(nouveau_livre);

        // Sauvegarder les livres après ajout
        saveBooks();

        return nouveau_livre;
    }

    // Mettre à jour un livre
    bool BibliothequeManager::mettreAJourLivre(int id, const json& donnees) {
        for (auto& livre : livres_) {
            if (livre.id == id) {
                if (donnees.contains("titre")) {
                    livre.titre = donnees["titre"].get<std::string>();
                }
                if (donnees.contains("auteur")) {
                    livre.auteur = donnees["auteur"].get<std::string>();
                }
                if (donnees.contains("annee")) {
                    livre.annee = donnees["annee"].get<int>();
                }
                if (donnees.contains("genre")) {
                    livre.genre = donnees["genre"].get<std::string>();
                }

                // Sauvegarder les livres après mise à jour
                saveBooks();

                return true;
            }
        }
        return false;
    }

    // Supprimer un livre
    bool BibliothequeManager::supprimerLivre(int id) {
        auto it = std::find_if(livres_.begin(), livres_.end(),
            [id](const Livre& livre) { return livre.id == id; });

        if (it != livres_.end()) {
            livres_.erase(it);
            saveBooks();
            return true;
        }
        return false;
    }

    // Mettre à jour uniquement le titre d'un livre
    bool BibliothequeManager::mettreAJourTitre(int id, const std::string& nouveauTitre) {
        for (auto& livre : livres_) {
            if (livre.id == id) {
                livre.titre = nouveauTitre;
                saveBooks();
                return true;
            }
        }
        return false;
    }

    // Conversion d'un livre en JSON
    json BibliothequeManager::livreToJson(const Livre& livre) const {
        return {
            {"id", livre.id},
            {"titre", livre.titre},
            {"auteur", livre.auteur},
            {"annee", livre.annee},
            {"genre", livre.genre}
        };
    }

    // Utilisation d'un singleton pour la gestion de la bibliothèque
    BibliothequeManager& getBibliotheque() {
        static BibliothequeManager instance;
        return instance;
    }

    // Gestionnaires de requêtes pour les routes

    // GET /books - Récupérer tous les livres
    crow::response getAllBooks() {
        auto& biblio = getBibliotheque();
        json result = json::array();
        for (const auto& livre : biblio.getTousLivres()) {
            result.push_back(biblio.livreToJson(livre));
        }
        auto response = crow::response(200, result.dump());
        response.add_header("Content-Type", "application/json; charset=utf-8");
        addCorsHeaders(response);
        return std::move(response);
    }

    // POST /books - Ajouter un nouveau livre
    crow::response addBook(const crow::request& req) {
        auto& biblio = getBibliotheque();
        try {
            auto body = json::parse(req.body);

            // Vérification des champs obligatoires
            if (!body.contains("titre") || !body.contains("auteur")) {
                auto response = crow::response(400, R"({"error": "Le titre et l'auteur sont obligatoires"})");
                response.add_header("Content-Type", "application/json; charset=utf-8");
                addCorsHeaders(response);
                return std::move(response);
            }

            // Extraction des données
            std::string titre = body["titre"].get<std::string>();
            std::string auteur = body["auteur"].get<std::string>();
            int annee = body.contains("annee") ? body["annee"].get<int>() : 0;
            std::string genre = body.contains("genre") ? body["genre"].get<std::string>() : "";

            // Ajout du livre
            Livre nouveau_livre = biblio.ajouterLivre(titre, auteur, annee, genre);

            // Préparation de la réponse
            json response_body = biblio.livreToJson(nouveau_livre);
            response_body["message"] = "Livre ajouté avec succès";

            auto response = crow::response(201, response_body.dump());
            response.add_header("Content-Type", "application/json; charset=utf-8");
            addCorsHeaders(response);
            return std::move(response);
        }
        catch (const std::exception& e) {
            json error = { {"error", std::string("Erreur de format JSON: ") + e.what()} };
            auto response = crow::response(400, error.dump());
            response.add_header("Content-Type", "application/json; charset=utf-8");
            addCorsHeaders(response);
            return std::move(response);
        }
    }

    // GET /books/id - Récupérer un livre spécifique par ID
    crow::response getBookById(int id) {
        auto& biblio = getBibliotheque();
        Livre* livre = biblio.getLivreParId(id);
        if (livre) {
            json result = biblio.livreToJson(*livre);
            auto response = crow::response(200, result.dump());
            response.add_header("Content-Type", "application/json; charset=utf-8");
            addCorsHeaders(response);
            return std::move(response);
        }
        json error = { {"error", "Livre non trouvé"} };
        auto response = crow::response(404, error.dump());
        response.add_header("Content-Type", "application/json; charset=utf-8");
        addCorsHeaders(response);
        return std::move(response);
    }

    // PUT /books/id - Mettre à jour un livre spécifique
    crow::response updateBook(const crow::request& req, int id) {
        auto& biblio = getBibliotheque();
        try {
            auto body = json::parse(req.body);

            if (biblio.mettreAJourLivre(id, body)) {
                Livre* livre = biblio.getLivreParId(id);
                if (livre) {
                    json result = biblio.livreToJson(*livre);
                    result["message"] = "Livre mis à jour avec succès";
                    auto response = crow::response(200, result.dump());
                    response.add_header("Content-Type", "application/json; charset=utf-8");
                    addCorsHeaders(response);
                    return std::move(response);
                }
            }
            json error = { {"error", "Livre non trouvé"} };
            auto response = crow::response(404, error.dump());
            response.add_header("Content-Type", "application/json; charset=utf-8");
            addCorsHeaders(response);
            return std::move(response);
        }
        catch (const std::exception& e) {
            json error = { {"error", std::string("Erreur de format JSON: ") + e.what()} };
            auto response = crow::response(400, error.dump());
            response.add_header("Content-Type", "application/json; charset=utf-8");
            addCorsHeaders(response);
            return std::move(response);
        }
    }

    // DELETE /books/id - Supprimer un livre spécifique
    crow::response deleteBook(int id) {
        auto& biblio = getBibliotheque();
        if (biblio.supprimerLivre(id)) {
            json result = {
                {"message", "Livre supprimé avec succès"},
                {"id", id}
            };
            auto response = crow::response(200, result.dump());
            response.add_header("Content-Type", "application/json; charset=utf-8");
            addCorsHeaders(response);
            return std::move(response);
        }
        json error = { {"error", "Livre non trouvé"} };
        auto response = crow::response(404, error.dump());
        response.add_header("Content-Type", "application/json; charset=utf-8");
        addCorsHeaders(response);
        return std::move(response);
    }

    // PATCH /books/id/titre - Mettre à jour uniquement le titre d'un livre
    crow::response updateBookTitle(const crow::request& req, int id) {
        auto& biblio = getBibliotheque();
        try {
            auto body = json::parse(req.body);

            // Vérification que le titre est présent
            if (!body.contains("titre")) {
                json error = { {"error", "Le titre est obligatoire"} };
                auto response = crow::response(400, error.dump());
                response.add_header("Content-Type", "application/json; charset=utf-8");
                addCorsHeaders(response);
                return std::move(response);
            }

            std::string nouveauTitre = body["titre"].get<std::string>();

            if (biblio.mettreAJourTitre(id, nouveauTitre)) {
                Livre* livre = biblio.getLivreParId(id);
                if (livre) {
                    json result = biblio.livreToJson(*livre);
                    result["message"] = "Titre mis à jour avec succès";
                    auto response = crow::response(200, result.dump());
                    response.add_header("Content-Type", "application/json; charset=utf-8");
                    addCorsHeaders(response);
                    return std::move(response);
                }
            }
            json error = { {"error", "Livre non trouvé"} };
            auto response = crow::response(404, error.dump());
            response.add_header("Content-Type", "application/json; charset=utf-8");
            addCorsHeaders(response);
            return std::move(response);
        }
        catch (const std::exception& e) {
            json error = { {"error", std::string("Erreur de format JSON: ") + e.what()} };
            auto response = crow::response(400, error.dump());
            response.add_header("Content-Type", "application/json; charset=utf-8");
            addCorsHeaders(response);
            return std::move(response);
        }
    }

    // Gestionnaire pour les préflight OPTIONS (nécessaire pour CORS)(sans paramètres)
    crow::response handleOptions() {
        auto response = crow::response(204);
        response.add_header("Content-Type", "application/json; charset=utf-8");
        addCorsHeaders(response);
        return std::move(response);
    }

    // Gestionnaire pour les préflight OPTIONS (nécessaire pour CORS)(avec paramètre ID)
    crow::response handleOptionsWithId(int id) {
        auto response = crow::response(204);
        response.add_header("Content-Type", "application/json; charset=utf-8");
        addCorsHeaders(response);
        return std::move(response);
    }


    // Configuration des routes pour la bibliothèque - versions avec et sans authentification
#if AUTH_ENABLED
	void setup_routes(crow::App<crow::CookieParser, JWTAuthMiddleware>& app) {
#else
	void setup_routes(crow::SimpleApp & app) {
#endif

        // Gestionnaire pour les préflight OPTIONS (nécessaire pour CORS)
        CROW_ROUTE(app, "/books")
            .methods(crow::HTTPMethod::OPTIONS)(handleOptions);

        CROW_ROUTE(app, "/books/<int>")
            .methods(crow::HTTPMethod::OPTIONS)(handleOptionsWithId);

        // GET /books - Récupérer tous les livres
        CROW_ROUTE(app, "/books")
            .methods(crow::HTTPMethod::GET)(getAllBooks);

        // POST /books - Ajouter un nouveau livre
        CROW_ROUTE(app, "/books")
            .methods(crow::HTTPMethod::POST)(addBook);

        // GET /books/id - Récupérer un livre spécifique par ID
        CROW_ROUTE(app, "/books/<int>")
            .methods(crow::HTTPMethod::GET)(getBookById);

        // PUT /books/id - Mettre à jour un livre spécifique
        CROW_ROUTE(app, "/books/<int>")
            .methods(crow::HTTPMethod::PUT)(updateBook);

        // DELETE /books/id - Supprimer un livre spécifique
        CROW_ROUTE(app, "/books/<int>")
            .methods(crow::HTTPMethod::Delete)(deleteBook);

        // PATCH /books/id/titre - Mettre à jour uniquement le titre d'un livre
        CROW_ROUTE(app, "/books/<int>/titre")
            .methods(crow::HTTPMethod::Patch)(updateBookTitle);

    }

    // Initialisation de la bibliothèque
    void initialize() {
#if AUTH_ENABLED
        std::cout << "Initializing crowJourney library with authentication..." << std::endl;
#else
        std::cout << "Initializing crowJourney library WITHOUT authentication (development mode)..." << std::endl;
#endif

        // S'assurer que la bibliothèque est initialisée
        auto& biblio = getBibliotheque();
    }
    }
