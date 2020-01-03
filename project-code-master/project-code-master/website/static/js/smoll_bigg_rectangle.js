// makes micro:bit-like aethetic section at the bottom of pages

let rectangle = $(`#rectangle`);

// left end
rectangle.append($(
    `<div class="smol_rectangle" style="border-radius:0px 0px 0px 15px"></div>
    <div class="thicc_separator"></div>
    <!-- '0' bar -->
    <div class="bigg_rectangle">
        <p>0</p>
    </div>`
));

// middle
for (var i = 1; i < 5; i++) {
    rectangle.append($(`
        <div class="thicc_separator"></div>
        <div class="smol_rectangle"></div>
        <div class="thinn_separator"></div>
        <div class="smol_rectangle"></div>
        <div class="thinn_separator"></div>
        <div class="smol_rectangle"></div>
        <div class="thinn_separator"></div>
        <div class="smol_rectangle"></div>
        <div class="thicc_separator"></div>
        <!-- '1' bar -->
        <div class="bigg_rectangle">
            <p>${i}</p>
        </div>`
    ));
}

// right end
rectangle.append($(
    `<div class="thicc_separator"></div>
    <div class="smol_rectangle" style="border-radius:0px 0px 15px 0px"></div>
    <div class="thinn_separator"></div>`
));

// make big rectangles round
let bigg_rect=$('.bigg_rectangle');
let radius=bigg_rect.width()/2;
bigg_rect.css('border-radius',`${radius}px ${radius}px 0 0`);
