#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <crow.h>
#include <nlohmann/json.hpp>
#include <fstream>

// Vérifier si l'authentification est désactivée
#ifndef DISABLE_AUTH
#include "JWTAuthMiddleware.h"
#include "CookieParser.h"
#include "CORSMiddleware.h"
#define AUTH_ENABLED 1
#else
#define AUTH_ENABLED 0
#endif

// Forward declarations
struct Livre {
    std::string titre;
    std::string auteur;
    int annee;
    std::string genre;
    int id;
};

// Utilisation de json comme alias pour nlohmann::json
using json = nlohmann::json;

// Déclaration pour la sérialisation JSON de Livre
namespace nlohmann {
    template <>
    struct adl_serializer<Livre>;
}

namespace crowjourney {

    // Classe principale pour la gestion de bibliothèque
    class BibliothequeManager {
    private:
        std::vector<Livre> livres_;
        int next_id_;
        std::string filename_; // Fichier pour sauvegarder les livres

    public:
        // Constructeur
        BibliothequeManager(const std::string& filename = "books.json");

        // Méthodes de gestion de fichier
        void saveBooks();
        void loadBooks();

        // Méthodes d'accès aux livres
        const std::vector<Livre>& getTousLivres() const;
        Livre* getLivreParId(int id);

        // Méthodes de modification
        Livre ajouterLivre(const std::string& titre, const std::string& auteur,
            int annee = 0, const std::string& genre = "");
        bool mettreAJourLivre(int id, const json& donnees);
        bool supprimerLivre(int id);
        bool mettreAJourTitre(int id, const std::string& nouveauTitre);

        // Méthode de conversion
        json livreToJson(const Livre& livre) const;
    };

    // Fonction pour obtenir l'instance singleton du gestionnaire
    BibliothequeManager& getBibliotheque();

    // Déclarations des gestionnaires de requêtes API
    crow::response getAllBooks();
    crow::response addBook(const crow::request& req);
    crow::response getBookById(int id);
    crow::response updateBook(const crow::request& req, int id);
    crow::response deleteBook(int id);
    crow::response updateBookTitle(const crow::request& req, int id);

    crow::response registerUser(const crow::request& req);
    crow::response getAllUsers();

    // Gestionnaire pour les préflight OPTIONS (nécessaire pour CORS)
    crow::response handleOptions();

    // Configuration des routes avec support pour les deux modes
#if AUTH_ENABLED
    void setup_routes(crow::App<crow::CookieParser, crowjourney::JWTAuthMiddleware, crowjourney_cors::CORSMiddleware>& app);
#else
    void setup_routes(crow::SimpleApp& app);
#endif

    // Initialisation de la bibliothèque
    void initialize();
}
