#pragma once
#include <crow.h>

namespace crowjourney {
    // Fonction utilitaire pour ajouter des headers CORS à une réponse

#ifdef DISABLE_AUTH
    inline void addCorsHeaders(crow::response& res) {
        // Mode sans authentification - ajout des en-têtes CORS
        res.add_header("Access-Control-Allow-Origin", "*");
        res.add_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, PATCH, OPTIONS");
        res.add_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
        std::cout << "En-têtes CORS ajoutés à la réponse" << std::endl;
    }
}
#else
        // Mode avec authentification - ajout des entetes CORS 
    inline void addCorsHeaders(crow::response& res) {
        // Vérifier si les en-têtes existent déjà
        std::cout << "Valeur actuelle de Access-Control-Allow-Origin: '"
            << res.get_header_value("Access-Control-Allow-Origin") << "'" << std::endl;

        // Supprimer les en-têtes existants pour éviter les conflits
        res.headers.erase("Access-Control-Allow-Origin");
        res.headers.erase("Access-Control-Allow-Methods");
        res.headers.erase("Access-Control-Allow-Headers");

        // Ajouter les en-têtes CORS
        res.add_header("Access-Control-Allow-Origin", "*");
        res.add_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, PATCH, OPTIONS");
        res.add_header("Access-Control-Allow-Headers", "Content-Type, Authorization");

        std::cout << "En-têtes CORS ajoutés à la réponse (après nettoyage)" << std::endl;
    }
#endif

}
