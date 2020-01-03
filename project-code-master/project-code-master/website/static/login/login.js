// helper for registering a callback on a form
function onSubmit(form, func) {
    $(form).submit(e => {
        func().catch(console.log);
        e.preventDefault();
    });
}

// register callbacks for the register and login forms
onSubmit('#registerForm', register);
onSubmit('#loginForm', sendCredentials);

// show a message to the user
let $message = $('#message');
function message(str, status) {
    $message.empty();

    $p = $(`<p>${str}</p>`);
    let color;
    if (status === 200) {
        color = 'darkgreen';
    } else {
        color = 'darkred';
    }
    $p.css('color', color);

    $message.append($p);
}

// when the user attempts to log in or register, send the username and
// password to the server to a path on the server, display the response
// and return the status code
async function loginPost(pathEnd) {
    let username = $("#usernameInput").val();
    let password = $("#passwordInput").val();
    // send request
    let res = await fetch(`/JH-project/api/${pathEnd}`, {
        method: 'POST',
        headers: {
            "Content-Type": "application/json",
        },
        body: JSON.stringify({
            username,
            password
        })
    });
    // display the resonse
    message(await res.json(), res.status);
    // return status code
    return res.status;
}

// send a register request when the register button is clicked
async function register() {
    await loginPost('register');
}

// send a login request when the login button is clicked
async function sendCredentials() {
    let status = await loginPost('credentials');
    // if login successful, redirect to the home page
    if (status === 200) {
        window.location.replace("/JH-project");
    }
}
