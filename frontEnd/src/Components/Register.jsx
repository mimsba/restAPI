import React, { useState } from 'react';
import { useAuth } from '../context/AuthContext';

function Register() {
  const [userData, setUserData] = useState({
    nom: '',
    email: '',
    password: ''
  });
  const [error, setError] = useState('');
  const [success, setSuccess] = useState('');
  const { register } = useAuth();

  const handleChange = (e) => {
    const { name, value } = e.target;
    setUserData({
      ...userData,
      [name]: value
    });
  };

  const handleSubmit = async (e) => {
    e.preventDefault();
    setError('');
    setSuccess('');

    const result = await register(userData);
    if (result.success) {
      setSuccess('Compte créé avec succès. Vous pouvez maintenant vous connecter.');
      setUserData({ nom: '', email: '', password: '' }); // Réinitialiser le formulaire
    } else {
      setError(result.error);
    }
  };

  return (
    <div className="register-form">
      <h2>Créer un compte</h2>
      {error && <div className="error-message">{error}</div>}
      {success && <div className="success-message">{success}</div>}
      
      <form onSubmit={handleSubmit}>
        <div className="form-group">
          <label htmlFor="nom">Nom:</label>
          <input
            type="text"
            id="nom"
            name="nom"
            value={userData.nom}
            onChange={handleChange}
            required
          />
        </div>

        <div className="form-group">
          <label htmlFor="email">Email:</label>
          <input
            type="email"
            id="email"
            name="email"
            value={userData.email}
            onChange={handleChange}
            required
          />
        </div>

        <div className="form-group">
          <label htmlFor="password">Mot de passe:</label>
          <input
            type="password"
            id="password"
            name="password"
            value={userData.password}
            onChange={handleChange}
            required
          />
          <small>Le mot de passe doit contenir au moins 8 caractères, une majuscule, une minuscule et un chiffre.</small>
        </div>

        <button type="submit">Créer un compte</button>
      </form>
    </div>
  );
}

export default Register;
