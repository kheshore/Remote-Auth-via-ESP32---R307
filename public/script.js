document.addEventListener('DOMContentLoaded', () => {
    const registerForm = document.getElementById('registerForm');
    const loginForm = document.getElementById('loginForm');
    const registerAuthText = document.getElementById('registerAuthText');
    const loginAuthText = document.getElementById('loginAuthText');

    registerForm.addEventListener('submit', async (event) => {
        event.preventDefault();
        registerAuthText.style.display = 'block'; 
        const formData = new FormData(registerForm);
        const id = generateID(); 
        formData.delete('multipart'); 
        try {
            const responseData = await register(formData);
            alert(responseData.message);
            registerForm.reset(); 
        } catch (error) {
            console.error('Error registering user:', error);
            alert(error.message);
        } finally {
            registerAuthText.style.display = 'none';
        }
    });

    loginForm.addEventListener('submit', async (event) => {
        event.preventDefault();
        loginAuthText.style.display = 'block'; 
        const formData = new FormData(loginForm);
        formData.delete('multipart'); 
        try {
            const responseData = await login(formData);
            alert(responseData.message);
            loginForm.reset(); 
        } catch (error) {
            console.error('Error logging in:', error);
            alert(error.message);
        } finally {
            loginAuthText.style.display = 'none'; 
        }
    });
});

async function register(formData) {
    formData.append('id', generateID());
    const response = await fetch('/register', {
        method: 'POST',
        body: new URLSearchParams([...formData]), 
        headers: {
            'Content-Type': 'application/x-www-form-urlencoded'
        }
    });
    if (!response.ok) {
        const errorData = await response.json();
        throw new Error(errorData.message || 'Failed to register user');
    }
    return await response.json();
}

async function login(formData) {
    const response = await fetch('/login', {
        method: 'POST',
        body: new URLSearchParams([...formData]),
        headers: {
            'Content-Type': 'application/x-www-form-urlencoded'
        }
    });
    if (!response.ok) {
        const errorData = await response.json();
        throw new Error(errorData.message || 'Failed to log in');
    }
    return await response.json();
}

let id = 0;

function generateID() {
    id = (id % 127) + 1;
    return id;
}
