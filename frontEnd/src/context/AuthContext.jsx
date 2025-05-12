import React, { createContext, useState, useEffect, useContext } from 'react';

// Création du contexte
const AuthContext = createContext();

// Hook personnalisé pour utiliser le contexte
export const useAuth = () => useContext(AuthContext);

export const AuthProvider = ({ children }) => {
  const [user, setUser] = useState(null);
  const [token, setToken] = useState(localStorage.getItem('token'));
  const [loading, setLoading] = useState(true);
  
  // URL de base de l'API
  const API_URL = 'http://localhost:8080';

  // Vérifier le token au chargement
    // Vérifier le token au chargement
    useEffect(() => {
        const verifyToken = async () => {
            if (!token) {
                setLoading(false);
                return;
            }

            try {
                console.log("Vérification du token:", token);

                // Appel à une route protégée pour vérifier si le token est valide
                const response = await fetch(`${API_URL}/protected`, {
                    headers: {
                        'Authorization': `Bearer ${token}`
                    }
                });

                console.log("Statut de la réponse:", response.status);

                if (response.ok) {
                    const data = await response.json();
                    console.log("Données utilisateur récupérées:", data);

                    // Assurez-vous que toutes les données utilisateur nécessaires sont présentes
                    setUser({
                        id: data.userId,
                        role: data.role,
                        email: data.email || "", // Ajoutez l'email si disponible
                        nom: data.nom || "Utilisateur"  // Ajoutez le nom si disponible
                    });
                } else {
                    console.error("Échec de la vérification du token:", await response.text());
                    // Ne pas se déconnecter immédiatement pour les erreurs 401
                    // Particulièrement si vous venez juste de vous connecter
                    if (response.status !== 401) {
                        logout();
                    }
                }
            } catch (error) {
                console.error("Erreur lors de la vérification du token:", error);
                // Ne pas se déconnecter en cas d'erreur réseau
            } finally {
                setLoading(false);
            }
        };

        verifyToken();
    }, [token]);


  // Fonction de login
    // Dans AuthContext.jsx, assurez-vous que le token est correctement stocké
    const login = async (email, password) => {
        try {
            const response = await fetch(`${API_URL}/login`, {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ email, password })
            });

            if (!response.ok) {
                const errorData = await response.json();
                throw new Error(errorData.error || 'Erreur de connexion');
            }

            const data = await response.json();

            // Déboguer le token reçu
            console.log("Token reçu:", data.token);

            // Stocker le token dans localStorage
            localStorage.setItem('token', data.token);
            setToken(data.token);
            setUser(data.user);

            return { success: true };
        } catch (error) {
            return { success: false, error: error.message };
        }
    };

  // Fonction de déconnexion
  const logout = () => {
    localStorage.removeItem('token');
    setToken(null);
    setUser(null);
  };

  // Fonction d'enregistrement
  const register = async (userData) => {
    try {
      const response = await fetch(`${API_URL}/users`, {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json'
        },
        body: JSON.stringify(userData)
      });

      if (!response.ok) {
        const errorData = await response.json();
        throw new Error(errorData.error || "Erreur lors de l'enregistrement");
      }

      return { success: true };
    } catch (error) {
      return { success: false, error: error.message };
    }
  };

  // Valeur du contexte
  const value = {
    user,
    token,
    loading,
    login,
    logout,
    register,
    isAuthenticated: !!user
  };

  return (
    <AuthContext.Provider value={value}>
      {children}
    </AuthContext.Provider>
  );
};
