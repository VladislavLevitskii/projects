<style>

.event-form {
    max-width: 600px;
    margin: 40px auto;
    padding: 30px;
    background: #fff;
    border-radius: 12px;
    box-shadow: 0 8px 30px rgba(0,0,0,0.1);
    font-family: sans-serif;
    display: flex;
    flex-direction: column;
}

.event-form label {
    display: block;
    font-weight: 700;
    font-size: 0.85rem;
    color: #4a5568;
    margin-bottom: 6px;
    margin-top: 15px;
    text-transform: uppercase;
    letter-spacing: 0.5px;
}

.event-form input[type="text"],
.event-form input[type="date"],
.event-form textarea {
    width: 100%;
    padding: 12px;
    border: 2px solid #edf2f7;
    border-radius: 8px;
    box-sizing: border-box;
    font-size: 1rem;
}

.event-form textarea {
    height: 120px;
    resize: vertical;
}

.date-row {
    display: flex;
    gap: 20px;
}

.date-row > div { flex: 1; }

.workshop-group {
    display: flex;
    gap: 10px;
}

.workshop-group button {
    background: #4a5568;
    color: white;
    border: none;
    padding: 0 20px;
    border-radius: 8px;
    cursor: pointer;
    white-space: nowrap;
}

#itemList {
    display: flex;
    flex-wrap: wrap;
    gap: 10px;
    padding: 0;
    list-style: none;
    margin-top: 15px;
}

#itemList li {
    background: #ebf8ff;
    color: #2b6cb0;
    padding: 6px 12px;
    border-radius: 20px;
    font-size: 0.85rem;
    font-weight: 600;
    border: 1px solid #bee3f8;
    display: flex;
    align-items: center;
    gap: 8px;
    animation: fadeIn 0.3s ease;
}

#itemList li button {
    content: "×";
    background: #fc8181;
    color: white;
    border: none;
    border-radius: 50%;
    width: 18px;
    height: 18px;
    display: flex;
    align-items: center;
    justify-content: center;
    font-size: 10px;
    cursor: pointer;
    padding: 0;
    line-height: 1;
    transition: background 0.2s;
}

#itemList li button:hover {
    background: #e53e3e;
}

#itemList li button.delete-ws::before {
    content: "×";
    display: block;
    font-size: 14px;
    line-height: 1;
}

@keyframes fadeIn {
from { opacity: 0; transform: translateY(5px); }
to { opacity: 1; transform: translateY(0); }
}

.submit-btn {
    background: #2ecc71;
    color: white;
    border: none;
    padding: 15px;
    font-size: 1.1rem;
    font-weight: bold;
    border-radius: 8px;
    margin-top: 30px;
    cursor: pointer;
    transition: transform 0.1s;
}

.submit-btn:hover { background: #27ae60; }
.submit-btn:active { transform: scale(0.98); }

.event-form input[type="file"]::file-selector-button {
    background: #4a5568;
    color: white;
    border: none;
    padding: 8px 16px;
    border-radius: 6px;
    cursor: pointer;
    margin-right: 15px;
    font-weight: 600;
    transition: background 0.2s;
}

.event-form input[type="file"]::file-selector-button:hover {
    background: #2d3748;
}

.event-form input[type="file"] {
    padding: 8px;
    border: 2px dashed #edf2f7;
    background: #f8fafc;
    cursor: pointer;
    color: #718096;
    font-size: 0.9rem;
}

</style>