# Configuration de vcpkg (à ajouter au début du fichier)
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "C:/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")
endif()

cmake_minimum_required(VERSION 3.20)

# Policy pour supprimer l'avertissement FindBoost
if(POLICY CMP0167)
    cmake_policy(SET CMP0167 NEW)
endif()

project(crowJourney VERSION 1.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Forcer UTF-8 pour tous les compilateurs
if(MSVC)
    add_compile_options(/utf-8)  # Force UTF-8 pour MSVC
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-finput-charset=UTF-8)
endif()

# Option for shared libs
option(BUILD_SHARED_LIBS "Build using shared libraries" ON)

# Dependencies
find_package(Boost REQUIRED COMPONENTS system)
find_package(Threads REQUIRED)

# Recherche OpenSSL via vcpkg
find_package(OpenSSL REQUIRED)
if(NOT OpenSSL_FOUND)
    message(FATAL_ERROR "OpenSSL not found. Please install it via vcpkg: vcpkg install openssl:x64-windows")
endif()

# nlohmann_json fallback
find_package(nlohmann_json QUIET)
if(NOT nlohmann_json_FOUND)
    find_path(NLOHMANN_JSON_INCLUDE_DIR nlohmann/json.hpp
            PATHS /usr/include /usr/local/include ${CMAKE_SOURCE_DIR}/external/json/include)
    if(NOT NLOHMANN_JSON_INCLUDE_DIR)
        message(FATAL_ERROR "Could not find nlohmann/json.hpp")
    endif()
    include_directories(${NLOHMANN_JSON_INCLUDE_DIR})
endif()

# jwt-cpp (header-only)
find_path(JWT_CPP_INCLUDE_DIR jwt-cpp/jwt.h
        PATHS /usr/include /usr/local/include ${CMAKE_SOURCE_DIR}/external/jwt-cpp/include)
if(NOT JWT_CPP_INCLUDE_DIR)
    message(FATAL_ERROR "Could not find jwt-cpp/jwt.h")
endif()

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${JWT_CPP_INCLUDE_DIR}
    ${OPENSSL_INCLUDE_DIR}
)

# Common library
add_library(crowJourney STATIC 
    src/library.cpp 
    src/UserManager.cpp 
    src/JWTAuthMiddleware.cpp
    src/utils.h
)

target_include_directories(crowJourney PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(crowJourney 
    PUBLIC
    Boost::system  # Utilisation de la cible moderne Boost::system
    Threads::Threads
    OpenSSL::SSL
    OpenSSL::Crypto
)

# Définition de la cible par défaut si TARGET_TO_BUILD n'est pas défini
if(NOT DEFINED TARGET_TO_BUILD)
    set(TARGET_TO_BUILD "book_api_server" CACHE STRING "Target to build")
endif()

# Define executable based on TARGET_TO_BUILD
if(TARGET_TO_BUILD STREQUAL "book_api_server")
    add_executable(book_api_server src/main.cpp src/utils.h)
    target_link_libraries(book_api_server 
        PRIVATE 
        crowJourney 
        Boost::system  # Utilisation de la cible moderne Boost::system
        Threads::Threads
        OpenSSL::SSL
        OpenSSL::Crypto
    )
    set(TARGET_BIN book_api_server)
endif()

# Windows specific
if(WIN32)
    if(DEFINED TARGET_BIN)
        target_compile_definitions(${TARGET_BIN} PRIVATE -D_WIN32_WINNT=0x0601)
    endif()
    target_compile_definitions(crowJourney PRIVATE -D_WIN32_WINNT=0x0601)
    add_compile_options(/wd4996)  # Disable deprecated warnings
endif()

# Installation
if(DEFINED TARGET_BIN)
    install(TARGETS ${TARGET_BIN}
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
    )
endif()

install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/
    DESTINATION include/crowJourney
    FILES_MATCHING PATTERN "*.h"
)

# Option pour désactiver l'authentification
option(DISABLE_AUTH "Désactiver l'authentification pour le développement" OFF)
# set(DISABLE_AUTH OFF CACHE BOOL "Désactiver l'authentification pour le développement" FORCE)
if(DISABLE_AUTH)
    # Définir au niveau global pour s'assurer qu'il est appliqué partout
    add_compile_definitions(DISABLE_AUTH)
    message(STATUS "Authentification DÉSACTIVÉE (mode développement)")
else()
    message(STATUS "Authentification ACTIVÉE (mode production)")
endif()

# Afficher l'état explicite de DISABLE_AUTH
message(STATUS "État actuel de DISABLE_AUTH: ${DISABLE_AUTH}")

# Afficher des informations utiles pour le débogage
message(STATUS "OpenSSL Include: ${OPENSSL_INCLUDE_DIR}")
message(STATUS "OpenSSL Libraries: ${OPENSSL_LIBRARIES}")
message(STATUS "OpenSSL Version: ${OPENSSL_VERSION}")
