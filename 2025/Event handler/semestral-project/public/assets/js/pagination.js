function jump_to_page(index_page) {
    page = index_page;
    const min_index = items_per_page * (index_page - 1) + 1;
    const max_index = items_per_page * index_page;
    Array.from(list.children).forEach((child, index) => {
        if (index + 1 > max_index ||  index + 1 < min_index) {
            child.style.display = 'none';
        } else {
            child.style.display = 'list-item';
        }
    });
    Array.from(buttons.children).forEach((child, index) => {
        if (index + 1 == index_page) {
            child.classList.add('selected');
        } else {
            child.classList.remove('selected');
        }
    });
}

function create_buttons() {
    if (number_of_pages === 1) {
        return;
    }
    for (let index = 1; index < number_of_pages + 1; index++) {
        const button = document.createElement('div');
        button.textContent = index;
        button.addEventListener('click', () => jump_to_page(button.textContent));
        buttons.appendChild(button);
    }
}

const list = document.getElementById('events');
const total_items = list.childElementCount;
const items_per_page = 7;
const number_of_pages = Math.floor(total_items / (items_per_page + 1)) + 1;
const buttons = document.getElementById('selector');
let page = 0;
create_buttons();
jump_to_page(1);