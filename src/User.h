#pragma once
struct User {
    int id;
    std::string nom;
    std::string email;
    std::string password_hash; // Mot de passe haché pour la sécurité
    std::string role;          // Rôle facultatif (admin, user, etc.)
    std::string date_creation; // Date de création du compte
};
