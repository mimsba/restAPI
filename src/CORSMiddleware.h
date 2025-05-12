#pragma once
#include <crow.h>
#include <iostream>

namespace crowjourney_cors {
    class CORSMiddleware {
    public:
        struct context {};

        // Traitement préliminaire des requêtes
        void before_handle(crow::request& req, crow::response& res, context& /*ctx*/) {
            std::cout << "CORSMiddleware: traitement de " << req.url;
            std::cout << " (méthode: " << (req.method == crow::HTTPMethod::Options ? "OPTIONS" : "AUTRE") << ")" << std::endl;

            // IMPORTANT: toujours ajouter les en-têtes CORS
            res.set_header("Access-Control-Allow-Origin", "*");
            res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, PATCH, OPTIONS");
            res.set_header("Access-Control-Allow-Headers", "Content-Type, Authorization");

            // Vérifier après avoir ajouté les en-têtes
            std::cout << "En-tête CORS ajouté: " << res.get_header_value("Access-Control-Allow-Origin") << std::endl;

            // Pour les requêtes OPTIONS, terminer immédiatement
            if (req.method == crow::HTTPMethod::Options) {
                std::cout << "Requête OPTIONS détectée, terminaison rapide" << std::endl;
                res.code = 204;
                res.end(); // CRUCIAL: terminer la réponse ici!
            }
        }

        void after_handle(crow::request& req, crow::response& res, context& /*ctx*/) {
            // Double vérification - s'assurer que les en-têtes CORS sont toujours présents
            if (res.get_header_value("Access-Control-Allow-Origin").empty()) {
                std::cout << "WARNING: En-tête CORS manquant après traitement! Réajout..." << std::endl;
                res.set_header("Access-Control-Allow-Origin", "*");
                res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, PATCH, OPTIONS");
                res.set_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
            }
        }
    };
}
