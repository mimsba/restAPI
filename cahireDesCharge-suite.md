# Création d'une API d'enregistrement d'utilisateur sécurisée

Nous allons créer une API d'enregistrement d'utilisateur sécurisée à l'aide de **C++**, **Crow** et d'une base de données de votre choix (**MySQL**, **PostgreSQL** ou **MongoDB**).

## Objectif

Créer une API REST en **C++** avec **Crow** qui :

1. Répond aux requêtes POST sur le chemin `/users` pour enregistrer un nouvel utilisateur.
2. Valide la saisie de l'utilisateur (nom, e-mail, mot de passe) pour s'assurer qu'elle répond à certains critères (par exemple, le format de l'e-mail, la force du mot de passe).
3. Nettoie la saisie de l'utilisateur pour éviter les failles de sécurité.
4. Hache le mot de passe de l'utilisateur avant de le stocker dans la base de données.
5. Renvoie une réponse appropriée (succès ou erreur) au client.

## Illustration

![Objectif de l'API](./assets/api_objectif.png)
