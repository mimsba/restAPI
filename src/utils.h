#pragma once
#include <crow.h>

namespace crowjourney {
    // Fonction utilitaire pour ajouter des headers CORS à une réponse
    inline void addCorsHeaders(crow::response& res) {
#ifdef DISABLE_AUTH
        // Mode sans authentification - ajout des en-têtes CORS
        res.add_header("Access-Control-Allow-Origin", "*");
        res.add_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, PATCH, OPTIONS");
        res.add_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
#else
        // Mode avec authentification - pas de CORS spécifiques
        return;
#endif
    }
}
