import React, { useState, useEffect } from 'react';
import Header from './Components/Header.jsx';
import Footer from './Components/Footer.jsx';

function App() {
    // État pour la section active (livres, utilisateurs, etc.)
    const [activeSection, setActiveSection] = useState('books');

    // État pour stocker les livres
    const [books, setBooks] = useState([]);

    // État pour le chargement
    const [loading, setLoading] = useState(false);

    // État pour les erreurs
    const [error, setError] = useState(null);

    // URL de base de l'API
    const API_URL = 'http://localhost:8080';

    // Fonction pour récupérer les livres depuis le serveur
    const fetchBooks = async () => {
        setLoading(true);

        try {
            const response = await fetch(`${API_URL}/books`);

            if (!response.ok) {
                const errorData = await response.json();

                alert("PB avec fetchBooks");

                throw new Error(errorData.error || 'Une erreur est survenue lors du chargement des livres');
            }

            const data = await response.json();
            setBooks(data);
            setError(null);

        } catch (err) {
            console.error("Erreur lors du chargement de l'API livre:", err.message);
            setError(err.message);
        } finally {
            setLoading(false);
        }
    };

    //// Fonction pour ajouter un nouveau livre
    //const addBook = async (bookData) => {
    //    setLoading(true);


    //    try {
    //        const response = await fetch(`${API_URL}/books`, {
    //            method: 'POST',
    //            headers: {
    //                'Content-Type': 'application/json'
    //            },
    //            body: JSON.stringify(bookData)
    //        });

    //        if (!response.ok) {
    //            const errorData = await response.json();
    //            throw new Error(errorData.error || "Erreur lors de l'ajout du livre");
    //        }

    //        const newBook = await response.json();
    //        setBooks([...books, newBook]);
    //        setError(null);

    //    } catch (err) {
    //        console.error("Erreur lors de l'ajout du livre:", err.message);
    //        setError(err.message);
    //    } finally {
    //        setLoading(false);
    //    }
    //};

    // Fonction pour ajouter un nouveau livre
    const addBook = async (bookData) => {
        setLoading(true);

        // Ajouter ces logs pour le débogage
        console.log("Données envoyées:", bookData);
        console.log("URL de la requête:", `${API_URL}/books`);
        console.log("JSON envoyé:", JSON.stringify(bookData));

        try {
            const response = await fetch(`${API_URL}/books`, {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json'
                },
                body: JSON.stringify(bookData)
            });

            // Ajouter ce log pour voir le code de statut
            console.log("Code de statut:", response.status);

            // Récupérer la réponse brute pour l'inspecter
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

            // Convertir la réponse textuelle en JSON
            const newBook = rawResponse ? JSON.parse(rawResponse) : {};
            setBooks([...books, newBook]);
            setError(null);

        } catch (err) {
            console.error("Erreur lors de l'ajout du livre:", err.message);
            setError(err.message);
        } finally {
            setLoading(false);
        }
    };


    // Fonction pour supprimer un livre
    const deleteBook = async (bookId) => {
        setLoading(true);
        console.log("Suppression du livre avec l'ID:", bookId);
        console.log(API_URL + "/books/" + bookId);
        try {
            const response = await fetch(`${API_URL}/books/${bookId}`, {
                method: 'DELETE'
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
            setLoading(false);
        }
    };

    // Utiliser useEffect pour charger les livres au chargement du composant
    useEffect(() => {
        fetchBooks();
    }, []);

    // Rendu conditionnel selon la section active
    const renderContent = () => {
        if (loading) {
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
                                    <h3>{book.titre}</h3>
                                    <p>Auteur: {book.auteur}</p>
                                    <p>Année: {book.annee || 'Non spécifiée'}</p>
                                    <p>Genre: {book.genre || 'Non spécifié'}</p>                                    
                                    <p>ID: {book.id}</p>
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
            <main className="col-12 container border-top" >
                <nav className="app-nav">
                    <button
                        className={activeSection === 'books' ? 'active' : ''}
                        onClick={() => setActiveSection('books')}
                    >
                        Livres
                    </button>
                    {/* Ajoutez d'autres sections ici */}
                </nav>

                {renderContent()}

                {/* Formulaire simple pour ajouter un livre */}
                {activeSection === 'books' && (
                    <div className="col-4 add-book-form mt-5">
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
            </main>
            <Footer />
        </div>
    );
}

export default App;
