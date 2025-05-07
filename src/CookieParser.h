// À ajouter dans un nouveau fichier src/CookieParser.h
#pragma once
#include <crow.h>
#include <unordered_map>
#include <string>

namespace crow {
    class CookieParser {
    public:
        struct context {
            std::unordered_map<std::string, std::string> cookies;
        };

        void before_handle(request& req, response& res, context& ctx) {
            // Analyser les cookies depuis l'en-tête de la requête
            std::string cookie_header = req.get_header_value("Cookie");
            if (!cookie_header.empty()) {
                size_t pos = 0;
                while (pos < cookie_header.length()) {
                    size_t end = cookie_header.find(';', pos);
                    if (end == std::string::npos)
                        end = cookie_header.length();
                    
                    size_t equal = cookie_header.find('=', pos);
                    if (equal < end) {
                        std::string key = cookie_header.substr(pos, equal - pos);
                        std::string value = cookie_header.substr(equal + 1, end - equal - 1);
                        
                        // Supprimer les espaces en début et fin
                        key.erase(0, key.find_first_not_of(" \t"));
                        key.erase(key.find_last_not_of(" \t") + 1);
                        value.erase(0, value.find_first_not_of(" \t"));
                        value.erase(value.find_last_not_of(" \t") + 1);
                        
                        ctx.cookies[key] = value;
                    }
                    
                    pos = end + 1;
                }
            }
        }

        void after_handle(request&, response& res, context& ctx) {
            // Facultatif : Ajouter les cookies de réponse
        }
    };
}
