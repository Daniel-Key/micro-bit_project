let autocompleteDiv = $('#cityInputDiv');

// auto-complete the weather
function autoComplete(inp, wsEndpoint) {
    $(document).click(e => {
        let elem = $(e.target);
        if (!elem.hasClass('autocomplete')) {
            // if the user clicks on the page and not in the auto-complete
            // list, make the list disappear
            autocompleteDiv.empty();
        } else if (elem.attr('id') == 'cityInput') {
            // if the user clicks on the autocomplete text box,
            // ensure the autocomplete list is present
            createList();
        }
    });
    // connect to WebSocket
    let protocol = window.location.protocol === 'http:' ? 'ws:' : 'wss:';
    let ws = new WebSocket(`${protocol}//${location.host}/${wsEndpoint}`);

    // the letters of the word in an array
    let word = [];
    // the auto-complete options
    let options = [];

    // a function that re-makes the autocomplete list in the DOM
    function createList() {
        autocompleteDiv.empty();
        // for each city name
        for (let cityName in options) {
            // for all cities with that name
            for (let city of options[cityName]) {
                // make an autocomplete option
                let option = $(`<div class="autocomplete">
                                    <span class="autocomplete">
                                        ${city.name}, ${city.country}
                                    </span>
                                </div>`);
                autocompleteDiv.append(option);

                // when the option is clicked, set it as the selected option
                option.click(() => {
                    inp.val(city.name);
                    autocompleteDiv.empty();

                    let obj = {
                        'type': 'setCity',
                        'id': city.id,
                        'name': city.name
                    };
                    ws.send(JSON.stringify(obj));

                    let message = $(`<span>Location set to ${city.name}, ${city.country}</span>`);

                    $('#message').empty();
                    $('#message').append(message);
                });
            }
        }
    }

    // when the WebSocket sends a list of the options to display, display the options
    ws.onmessage = event => {
        let obj = JSON.parse(event.data);
        options = obj;

        createList();
    };

    let char = undefined;
    let isBackspace = true;
    // when the user types a letter, re-request the options list
    inp.on('input', (a) => {
        let obj = undefined;
        // maintain word array
        if (isBackspace) {
            word.pop();
        } else {
            word.push(char);
        }

        if (word.join('') === inp.val()) { // if word was correctly maintained in the list
            if (isBackspace) { // if backspace
                obj = {
                    'type': 'back'
                }
            } else { // if a letter typed
                obj = {
                    'type': 'char',
                    'char': char
                }
            }
        } else { // if something unexpected happened
            // reset by tranmitting the entire thing
            word = inp.val().split("");
            obj = {
                'type': 'word',
                'word': word
            }
        }
        ws.send(JSON.stringify(obj));
        isBackspace = true;
    });
    inp.on('keypress', e => {
        char = e.originalEvent.key;
        code = e.keyCode;
        isBackspace = false;
    });

}

// start autocomplete
autoComplete(
    $('#cityInput'),
    'JH-project/api/city-autocomplete'
);
