<style>

.auth-form {
    display: flex;
    flex-direction: column;
    max-width: 320px;
    padding: 2rem;
    background: white;
    border-radius: 12px;
    box-shadow: 0 10px 25px rgba(0,0,0,0.1);
    box-sizing: border-box;
    margin: 20px auto;
}

.auth-form label {
    font-size: 0.8rem;
    font-weight: 700;
    margin-bottom: 6px;
    color: #444;
    display: block;
}

.auth-form input {
    width: 100%;
    padding: 12px;
    margin-bottom: 20px;
    border: 2px solid #eee;
    border-radius: 8px;
    box-sizing: border-box;
    transition: all 0.2s ease;
}

.auth-form input:focus {
    outline: none;
    border-color: #007bff;
    background: #f0f7ff;
}

.auth-form button {
    background: #007bff;
    color: white;
    border: none;
    padding: 14px;
    border-radius: 8px;
    font-weight: 600;
    cursor: pointer;
    transition: background 0.3s;
    width: -webkit-fill-available;
}

.auth-form button:hover {
  background: #0056b3;
}

</style>