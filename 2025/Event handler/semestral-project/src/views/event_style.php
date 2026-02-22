<style>
.event {
    background: white;
    padding: 30px;
    border-radius: 16px;
    box-shadow: 0 10px 30px rgba(0, 0, 0, 0.08);
    border: 1px solid #eee;
    max-width: 700px;
    margin: 2rem auto;
    font-family: 'Inter', sans-serif;
    box-sizing: border-box;
}

.event h1 {
    font-size: 2rem;
    color: #1a1a1a;
    margin-top: 0;
    margin-bottom: 20px;
    letter-spacing: -0.5px;
}

.event p {
    color: #666;
    font-size: 0.95rem;
    margin: 8px 0;
}

.event p:not(.event_image + p) {
    font-weight: 500;
}

.event_image {
    width: 500px;
    object-fit: cover;
    border-radius: 12px;
    margin: 15px 0 25px 0;
    box-shadow: 0 4px 12px rgba(0,0,0,0.1);
}

.event ul {
    display: flex;
    flex-wrap: wrap;
    gap: 8px;
    padding: 0;
    list-style: none;
    margin-top: 10px;
}

.event ul li {
    background: #f0f4ff;
    color: #007bff;
    padding: 6px 14px;
    border-radius: 100px;
    font-size: 0.85rem;
    font-weight: 600;
    border: 1px solid #d0e0ff;
}

.event a, 
.event button {
    display: inline-block;
    margin-top: 25px;
    padding: 12px 24px;
    border-radius: 8px;
    text-decoration: none;
    font-weight: 700;
    font-size: 0.9rem;
    cursor: pointer;
    transition: all 0.2s;
    border: none;
}

.event a[href*="/register/"] {
    background: #28a745;
    color: white;
    width: 100%;
    text-align: center;
}

.event a[href="edit/"] {
    background: #f8f9fa;
    color: #333;
    border: 1px solid #ddd;
}

.event button[type="submit"] {
    background: #fff0f0;
    color: #dc3545;
    border: 1px solid #ffc9c9;
    margin-left: 10px;
}

.event button[type="submit"]:hover {
    background: #dc3545;
    color: white;
}

.event a:hover {
    filter: brightness(0.9);
    transform: translateY(-1px);
}

.form-ref {
    display: flex;
    flex-direction: column;
    margin-top: 15px;
}

.form-ref input[type="text"],
.form-ref input[type="date"],
.form-ref textarea {
    width: 100%;
    padding: 12px;
    border: 2px solid #edf2f7;
    border-radius: 8px;
    box-sizing: border-box;
    font-size: 1rem;
}

.form-ref textarea {
    height: 120px;
    resize: vertical;
}

</style>