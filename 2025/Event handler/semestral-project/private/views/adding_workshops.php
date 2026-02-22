<script>
    let itemsArray = [];

    function addItem() {
        const input = document.getElementById('itemInput');
        const value = input.value.trim();
        
        if (value && !value.includes(" ")) {
            itemsArray.push(value);
            
            const li = document.createElement('li');
            li.textContent = value + " ";
            
            const button = document.createElement('button');
            button.type = 'button';
            button.className = 'delete-ws';
            
            button.onclick = function() {
                li.remove();
                
                const index = itemsArray.indexOf(value);
                if (index > -1) {
                    itemsArray.splice(index, 1);
                }
                
                document.getElementById('hiddenItems').value = itemsArray.join(' ');
            };
            
            li.appendChild(button);
            document.getElementById('itemList').appendChild(li);
            document.getElementById('hiddenItems').value = itemsArray.join(' ');
            
            input.value = '';
        } else {
            alert("Item should not have spaces !");
        }
    }

</script>