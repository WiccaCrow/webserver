var swiper = new Swiper('.mySwiper', {
    direction: 'vertical',
    slidesPerView: 1,
    speed: 900,
    mousewheel: true,
    keyboard: {
        enabled: true,
    },
});

if (document.cookie) {
    const runningLine = document.createElement('article');

    runningLine.innerHTML = `
        <div class="ticker-wrapper">
            <p>Do you like cookies? 🍪 From the JS code, you can read cookies that are not marked with the HttpOnly option.</p>
            <p>Look at the cookies of this page: ${document.cookie}</p>
        </div>`;

    document.body.appendChild(runningLine);
}
