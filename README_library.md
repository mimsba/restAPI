# Système de Gestion de Livres via une API REST en C++

## Vue d’ensemble du projet  
Ce projet implémente une API RESTful pour la gestion de livres en C++ à l’aide du framework Crow et de la bibliothèque nlohmann/json.  
L’API prend en charge l’ensemble des opérations CRUD (Création, Lecture, Mise à jour, Suppression) pour les livres.

## Structure du répertoire
```
book-api/
├── CMakeLists.txt
├── README.md
├── include/
│   └── book.h
├── src/
│   ├── main.cpp          # Implémentation principale du serveur API
│   ├── user.cpp          # Fonctionnalité de gestion des utilisateurs
│   └── lib.cpp           # Fonctionnalités principales de la bibliothèque
└── tests/
    └── test_api.cpp      # Tests de l’API
```

## Prérequis
- Un compilateur C++ prenant en charge C++11 ou supérieur
- CMake version 3.10 ou plus
- Les bibliothèques Boost
- Le framework Crow
- La bibliothèque nlohmann/json

## Installation

### Sur Linux (Ubuntu/Debian)
```bash
# Installer les outils de développement nécessaires
sudo apt-get update
sudo apt-get install -y build-essential cmake libboost-all-dev

# Installer nlohmann/json
sudo apt-get install -y nlohmann-json3-dev

# Installer Crow (depuis les sources)
git clone https://github.com/CrowCpp/Crow.git
cd Crow
mkdir build && cd build
cmake ..
make -j
sudo make install
```

### Sur Windows (avec vcpkg)
```cmd
# Installer vcpkg si ce n’est pas déjà fait
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
bootstrap-vcpkg.bat

# Installer les dépendances
vcpkg install boost:x64-windows
vcpkg install nlohmann-json:x64-windows
vcpkg install crow:x64-windows

# Intégrer avec Visual Studio
vcpkg integrate install
```

## Compilation du projet



### Sous Windows avec Visual Studio
1. Ouvrir Visual Studio  
2. Sélectionner "Ouvrir un dossier local" et naviguer jusqu’au dossier du projet  
3. Visual Studio détectera le fichier CMakeLists.txt et configurera le projet  
4. Compiler via le menu : Build → Build All  
5. Exécuter le projet

### Sous Windows via ligne de commande
```cmd
mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=[chemin\vers\vcpkg]\scripts\buildsystems\vcpkg.cmake
cmake --build .
```

## Cibles du projet

La configuration CMake fournit plusieurs cibles :

- **book_api_server** : Exécutable principal du serveur API (à partir de main.cpp)  
- **user_app** : Application de gestion des utilisateurs (à partir de user.cpp)  
- **book_api_lib** : Bibliothèque centrale du projet (à partir de lib.cpp)  
- **run_tests** : Exécutable pour les tests de l’API (à partir de tests/test_api.cpp)

## Points de terminaison de l’API (Endpoints)

| Méthode | Point d'accès  | Description                        | Codes de retour |
|---------|----------------|------------------------------------|-----------------|
| GET     | /books         | Récupère tous les livres           | 200 OK          |
| POST    | /books         | Ajoute un nouveau livre            | 201 Created, 400 Bad Request |
| GET     | /books/id      | Récupère un livre spécifique par ID| 200 OK, 404 Not Found |
| PUT     | /books/id      | Met à jour un livre spécifique     | 200 OK, 400 Bad Request, 404 Not Found |
| DELETE  | /books/id      | Supprime un livre spécifique       | 200 OK, 404 Not Found |

## Tester l’API

### Avec cURL
```bash
# Récupérer tous les livres
curl -X GET http://localhost:8080/books

# Ajouter un nouveau livre
curl -X POST http://localhost:8080/books \
  -H "Content-Type: application/json" \
  -d '{"title": "1984", "author": "George Orwell"}'

# Récupérer un livre spécifique
curl -X GET http://localhost:8080/books/1

# Mettre à jour un livre
curl -X PUT http://localhost:8080/books/1 \
  -H "Content-Type: application/json" \
  -d '{"title": "The Ultimate Hitchhiker\'s Guide to the Galaxy"}'

# Supprimer un livre
curl -X DELETE http://localhost:8080/books/2
```

### Avec Postman
1. Ouvrir Postman  
2. Sélectionner la méthode (GET, POST, PUT, DELETE)  
3. Entrer l’URL correspondante  
4. Pour POST et PUT, configurer le corps (Body) au format JSON brut  
5. Cliquer sur "Send"

## Dépannage

### Problèmes courants

1. **CMake ne trouve pas Crow** :
   - Vérifier que Crow est bien installé  
   - Définir `CMAKE_PREFIX_PATH` pour inclure le dossier d’installation de Crow

2. **Les bibliothèques Boost sont introuvables** :
   - S'assurer que Boost est installé  
   - Définir la variable d’environnement `BOOST_ROOT` si nécessaire

3. **Erreurs de compilation avec JSON** :
   - Vérifier que nlohmann/json est bien installé  
   - Vérifier les chemins d'inclusion dans CMake

4. **Problèmes de compatibilité sous Windows** :
   - Utiliser `vcpkg` pour gérer les dépendances  
   - S’assurer que le workload "Développement desktop en C++" est installé dans Visual Studio

## Contribuer
1. Forker le dépôt  
2. Créer une branche pour votre fonctionnalité (`git checkout -b feature/amazing-feature`)  
3. Commiter vos modifications (`git commit -m 'Add some amazing feature'`)  
4. Pousser la branche (`git push origin feature/amazing-feature`)  
5. Ouvrir une Pull Request

## Licence
Ce projet est sous licence MIT — voir le fichier LICENSE pour les détails.
