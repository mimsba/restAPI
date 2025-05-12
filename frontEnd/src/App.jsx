import React, { useState, useEffect } from 'react';
import Header from './Components/Header.jsx';
import Footer from './Components/Footer.jsx';
import Login from './Components/Login.jsx';
import Register from './Components/Register.jsx';
import { useAuth } from './context/AuthContext.jsx';

function App() {
    // État pour la section active (livres, utilisateurs, etc.)
    const [activeSection, setActiveSection] = useState('books');

    // Utiliser un alias pour le "loading" d'authentification
    const { user, isAuthenticated, loading: authLoading, token, logout } = useAuth();

    // État pour stocker les livres
    const [books, setBooks] = useState([]);

    // État pour le chargement des données (renommé pour éviter le conflit)
    const [dataLoading, setDataLoading] = useState(false);

    // État pour les erreurs
    const [error, setError] = useState(null);

    // URL de base de l'API
    const API_URL = 'http://localhost:8080';

    // Fonction pour récupérer les livres depuis le serveur
    const fetchBooks = async () => {
        setDataLoading(true);

        try {

            // Assurez-vous que le token est disponible et correctement formaté
            console.log("Token utilisé pour la requête:", token);

            // Ajout du token d'authentification si disponible
            const headers = {};
            if (token) {
                headers['Authorization'] = `Bearer ${token}`;
            }

            const response = await fetch(`${API_URL}/books`, {
                headers
            });

            console.log("Statut de la réponse:", response.status);

            if (!response.ok) {
                const errorData = await response.json();
                throw new Error(errorData.error || 'Une erreur est survenue lors du chargement des livres');
            }

            const data = await response.json();
            setBooks(data);
            setError(null);

        } catch (err) {
            console.error("Erreur lors du chargement de l'API livre:", err.message);
            setError(err.message);
        } finally {
            setDataLoading(false);
        }
    };

    // Fonction pour ajouter un nouveau livre
    const addBook = async (bookData) => {
        setDataLoading(true);

        // Ajouter ces logs pour le débogage
        console.log("Données envoyées:", bookData);
        console.log("URL de la requête:", `${API_URL}/books`);
        console.log("JSON envoyé:", JSON.stringify(bookData));

        try {
            // Ajout du token d'authentification si disponible
            const headers = {
                'Content-Type': 'application/json'
            };
            if (token) {
                headers['Authorization'] = `Bearer ${token}`;
            }

            const response = await fetch(`${API_URL}/books`, {
                method: 'POST',
                headers,
                body: JSON.stringify(bookData)
            });

            console.log("Code de statut:", response.status);
            const rawResponse = await response.text();
            console.log("Réponse brute:", rawResponse);

            if (!response.ok) {
                let errorMessage = "Erreur lors de l'ajout du livre";
                try {
                    const errorData = JSON.parse(rawResponse);
                    errorMessage = errorData.error || errorMessage;
                } catch (e) {
                    console.error("Erreur lors du parse de l'erreur:", e);
                }
                throw new Error(errorMessage);
            }

            const newBook = rawResponse ? JSON.parse(rawResponse) : {};
            setBooks([...books, newBook]);
            setError(null);

        } catch (err) {
            console.error("Erreur lors de l'ajout du livre:", err.message);
            setError(err.message);
        } finally {
            setDataLoading(false);
        }
    };

    // Fonction pour supprimer un livre
    const deleteBook = async (bookId) => {
        setDataLoading(true);
        console.log("Suppression du livre avec l'ID:", bookId);
        console.log(API_URL + "/books/" + bookId);
        try {
            // Ajout du token d'authentification si disponible
            const headers = {};
            if (token) {
                headers['Authorization'] = `Bearer ${token}`;
            }

            const response = await fetch(`${API_URL}/books/${bookId}`, {
                method: 'DELETE',
                headers
            });

            if (!response.ok) {
                const errorData = await response.json();
                throw new Error(errorData.error || 'Erreur lors de la suppression du livre');
            }

            // Filtrer les livres pour retirer celui qui a été supprimé
            setBooks(books.filter(book => book.id !== bookId));
            setError(null);

        } catch (err) {
            console.error("Erreur lors de la suppression du livre:", err.message);
            setError(err.message);
        } finally {
            setDataLoading(false);
        }
    };

    // Utiliser useEffect pour charger les livres au chargement du composant
    useEffect(() => {
        if (isAuthenticated) {
            fetchBooks();
        }
    }, [isAuthenticated]);

    // Si l'authentification est en cours de chargement
    if (authLoading) {
        return <div>Chargement de l'authentification...</div>;
    }

    // Rendu pour les utilisateurs non authentifiés
    if (!isAuthenticated) {
        return (
            <div className="App">
                <Header />
                <main className="container">
                    <nav className="app-nav">
                        <button
                            className={activeSection === 'login' ? 'active' : ''}
                            onClick={() => setActiveSection('login')}
                        >
                            Connexion
                        </button>
                        <button
                            className={activeSection === 'register' ? 'active' : ''}
                            onClick={() => setActiveSection('register')}
                        >
                            Inscription
                        </button>
                    </nav>

                    {activeSection === 'login' && <Login />}
                    {activeSection === 'register' && <Register />}
                </main>
                <Footer />
            </div>
        );
    }

    // Rendu conditionnel selon la section active pour les utilisateurs authentifiés
    const renderContent = () => {
        if (dataLoading) {
            return <p>Chargement en cours...</p>;
        }

        if (error) {
            return <p className="error">Erreur: {error}</p>;
        }

        if (activeSection === 'books') {
            return (
                <div className="books-container">
                    <h2>Liste des Livres</h2>
                    {books.length === 0 ? (
                        <p>Aucun livre disponible</p>
                    ) : (
                        <ul className="books-list">
                            {books.map(book => (
                                <li key={book.id} className="book-item">
                                    <div className="book-details">
                                        <h3>{book.titre}</h3>
                                        <p><strong>Auteur:</strong> {book.auteur}</p>
                                        <p><strong>Année:</strong> {book.annee || 'Non spécifiée'}</p>
                                        <p><strong>Genre:</strong> {book.genre || 'Non spécifié'}</p>
                                        <p><strong>ID:</strong> {book.id}</p>
                                    </div>
                                    <button
                                        className="delete-button"
                                        onClick={() => deleteBook(book.id)}
                                    >
                                        Supprimer
                                    </button>
                                </li>
                            ))}
                        </ul>
                    )}
                </div>
            );
        }

        return <p>Sélectionnez une section</p>;
    };


    return (
        <div className="App">
            <Header />
            <main className="container border-top" >
                <div className="user-info">
                    <p>Connecté en tant que: {user.nom} ({user.email})</p>
                    <button onClick={logout}>Se déconnecter</button>
                </div>
                <nav className="app-nav">
                    <button
                        className={activeSection === 'books' ? 'active' : ''}
                        onClick={() => setActiveSection('books')}
                    >
                        Livres
                    </button>
                    {/* Afficher le bouton Admin uniquement pour les utilisateurs avec ce rôle */}
                    {user && user.role === 'admin' && (
                        <button
                            className={activeSection === 'admin' ? 'active' : ''}
                            onClick={() => setActiveSection('admin')}
                        >
                            Administration
                        </button>
                    )}
                </nav>

                <div className="main-layout">
                    <div className="content-area">
                        {renderContent()}
                    </div>

                    {/* Formulaire d'ajout de livre sur le côté */}
                    {activeSection === 'books' && (
                        <div className="add-book-form">
                            <h3>Ajouter un nouveau livre</h3>
                            <form onSubmit={(e) => {
                                e.preventDefault();
                                const formData = new FormData(e.target);
                                addBook({
                                    titre: formData.get('titre'),
                                    auteur: formData.get('auteur'),
                                    annee: parseInt(formData.get('annee')) || 0,
                                    genre: formData.get('genre')
                                });
                                e.target.reset();
                            }}>
                                <div className="form-group">
                                    <label htmlFor="titre">Titre:</label>
                                    <input
                                        type="text"
                                        id="titre"
                                        name="titre"
                                        required
                                    />
                                </div>

                                <div className="form-group">
                                    <label htmlFor="auteur">Auteur:</label>
                                    <input
                                        type="text"
                                        id="auteur"
                                        name="auteur"
                                        required
                                    />
                                </div>

                                <div className="form-group">
                                    <label htmlFor="annee">Année:</label>
                                    <input
                                        type="number"
                                        id="annee"
                                        name="annee"
                                    />
                                </div>

                                <div className="form-group">
                                    <label htmlFor="genre">Genre:</label>
                                    <input
                                        type="text"
                                        id="genre"
                                        name="genre"
                                    />
                                </div>

                                <button type="submit">Ajouter</button>
                            </form>
                        </div>
                    )}
                </div>
            </main>
            <Footer />
        </div>
    );
    
}

export default App;
