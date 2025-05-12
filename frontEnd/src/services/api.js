// Service API pour gérer les requêtes authentifiées

const API_URL = 'http://localhost:8080';

// Fonction utilitaire pour les requêtes API
export const apiRequest = async (endpoint, options = {}) => {
  const token = localStorage.getItem('token');
  
  // Ajouter l'en-tête d'autorisation si un token existe
  const headers = {
    'Content-Type': 'application/json',
    ...options.headers
  };
  
  if (token) {
    headers['Authorization'] = `Bearer ${token}`;
  }
  
  // Construire les options de la requête
  const requestOptions = {
    ...options,
    headers
  };
  
  try {
    const response = await fetch(`${API_URL}${endpoint}`, requestOptions);
    
    // Si la réponse indique que le token est invalide (401)
    if (response.status === 401) {
      // Vider le localStorage et rafraîchir la page
      localStorage.removeItem('token');
      window.location.reload();
      return;
    }
    
    // Traitement de la réponse
    const contentType = response.headers.get('Content-Type');
    if (contentType && contentType.includes('application/json')) {
      const data = await response.json();
      
      if (!response.ok) {
        throw new Error(data.error || 'Une erreur est survenue');
      }
      
      return data;
    } else {
      const text = await response.text();
      
      if (!response.ok) {
        throw new Error(text || 'Une erreur est survenue');
      }
      
      return text;
    }
  } catch (error) {
    console.error('API request failed:', error);
    throw error;
  }
};

// Méthodes d'API spécifiques
export const api = {
  // Livres
  getBooks: () => apiRequest('/books'),
  addBook: (book) => apiRequest('/books', {
    method: 'POST',
    body: JSON.stringify(book)
  }),
  deleteBook: (id) => apiRequest(`/books/${id}`, {
    method: 'DELETE'
  }),
  
  // Authentification
  login: (credentials) => apiRequest('/login', {
    method: 'POST',
    body: JSON.stringify(credentials)
  }),
  register: (userData) => apiRequest('/users', {
    method: 'POST',
    body: JSON.stringify(userData)
  }),
  
  // Utilisateurs
  getUsers: () => apiRequest('/users'),
};

export default api;
