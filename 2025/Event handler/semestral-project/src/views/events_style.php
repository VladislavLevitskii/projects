<style>

    #events {
        margin: 0 auto 50px auto;
        list-style-type: none;
        display: flex;
        flex-wrap: wrap;
        gap: 30px;
        padding: 0 50px;
        width: fit-content;
    }

    #events > li > ul {
        list-style: none;
        padding: 20px;
        background: #ffffff;
        border-radius: 12px;
        box-shadow: 0 4px 15px rgba(0,0,0,0.1);
        max-width: 350px;
        font-family: sans-serif;
    }

    #events > li > ul > li {
        padding: 8px 0;
        color: #444;
        border-bottom: 1px solid #eee;
    }
    
    #events > li > ul > li:first-child {
        font-size: 1.2rem;
        font-weight: bold;
        color: #1a1a1a;
        border-bottom: 2px solid #007bff;
        margin-bottom: 10px;
        padding-bottom: 12px;
    }
    
    #events > li > ul > li:nth-last-child(1) {
        border-bottom: none;
        margin-top: 15px;
        text-align: center;
    }
    
    #events > li > ul > li:last-child a {
        display: inline-block;
        background: #007bff;
        color: white;
        padding: 10px 20px;
        border-radius: 6px;
        text-decoration: none;
        transition: background 0.3s;
    }

    #events > li > ul > li:last-child a:hover {
        background: #0056b3;
    }

    #events > li > ul > li:not(:first-child):not(:last-child)::before {
        content: "•";
        color: #007bff;
        font-weight: bold;
        margin-right: 8px;
    }

</style>