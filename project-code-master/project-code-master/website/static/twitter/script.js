let $subscritions = $('#twitter-subscriptions');

let $twitterHandle = $('#twitter-handle');

$('#subscribe').submit(e => {
    subscribe().catch(console.log);
    e.preventDefault();
});

let $message = $('#message');

function message(str, status) {
    $message.empty();

    $p = $(`<p>${str}</p>`);

    $message.append($p);
}

async function subscribe() {
    let twitter_handle = $twitterHandle.val();

    let res = await fetch('/JH-project/api/twitter-subscribe', {
        method: 'POST',
        headers: {
            "Content-Type": "application/json",
        },
        body: JSON.stringify({
            twitter_handle
        })
    });
    if (res.status === 200) {
        addSubscriberRow(twitter_handle);

        message('');
    } else {
        message(await res.text());
    }
}

function addSubscriberRow(twitter_handle) {
    let row = $('<tr></tr>');

    let cross = $(`<td><img src="/JH-project/images/cross.png" width="23px"></td>`);
    cross.css('cursor', 'pointer');
    cross.click(() => {
        unSubscribe(twitter_handle, row).catch(console.log);
    });

    row.append(cross);
    row.append($(`<td>${twitter_handle}</td>`));

    $subscritions.append(row);
}

async function unSubscribe(twitter_handle, row) {
    let res = await fetch(`/JH-project/api/twitter-subscribe/${twitter_handle}`, {
        method: 'DELETE',

    });
    if (res.status === 200) {
        row.remove();
    } else {
        message(await res.text());
    }
}

async function getSubscriptions() {
    let res = await fetch('/JH-project/api/twitter-subscribe');
    let twitter_handles = await res.json();
    for (let twitter_handle of twitter_handles) {
        addSubscriberRow(twitter_handle);
    }
}

getSubscriptions().catch(console.log)
