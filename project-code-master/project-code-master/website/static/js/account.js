let accountLink = $('#account');
let nav = $('nav');
let navUl = $('<ul></ul>');
nav.append(navUl);

// all the links in the navigation bar
//     name: the link text
//     url: the actual link
//     authorisation: true if the link should only be accessible when logged in
let navLinks = [{
        name: 'Home',
        url: '/JH-project/',
        authorisation: false
    },
    {
        name: 'History Log',
        url: '/JH-project/history/',
        authorisation: true
    },
    {
        name: 'Weather',
        url: '/JH-project/weather-settings/',
        authorisation: true
    },
    {
        name: 'Lock Pattern',
        url: '/JH-project/lock-pattern/',
        authorisation: true
    },
    {
        name: 'Twitter',
        url: '/JH-project/twitter/',
        authorisation: true
    }
];

// check if logged in
$.ajax({
    url: '/JH-project/api/loggedin',
    type: "GET",
    contentType: "application/json; charset=utf-8"
}).done((loggedin) => {
    if (loggedin) {
        // if logged in, make "sign in" button say "sign out", and change it's behaviour
        accountLink.text('Sign out');
        accountLink.click(signout);
        accountLink.attr('href', '#');
        // make the navigation bar for when logged in
        makeNav(true);
    } else {
        // if not logged in, make the "sign in" button say "sign in"
        accountLink.text('Sign in');
        // make the navigation bar for when not logged in
        makeNav(false);
    }
}).fail(console.log);

// when sign out button click, send a sign out request
function signout() {
    $.ajax({
        url: '/JH-project/api/signout',
        type: "POST",
        success: function(data) {
            // after sign out, go to login page
            console.log("signed out");
            window.location.replace('/JH-project/login');
        }
    }).fail(e => {
        window.location.replace('/JH-project/login');
    });
}

// make the navigation bar with the approriate links
function makeNav(logged_in) {
    for (let navLink of navLinks) {
        // if currently on a page that requires login and
        // not logged in, go to the login page
        if (!logged_in && navLink.authorisation && navLink.url === location.pathname) {
            window.location.replace('/JH-project/login');
        }

        // if logged in or if the link doesn't require login, show the link
        if (logged_in || !navLink.authorisation) {
            let li;
            if (navLink.url === location.pathname) {
                // if currently on the link, don't show it as a link
                li = $(`<li class="clicked">${navLink.name}</li>`)
            } else {
                // else show it as a link
                li = $(`<li><a href="${navLink.url}">${navLink.name}</a></li>`);
            }
            navUl.append(li);
        }
    }
}
