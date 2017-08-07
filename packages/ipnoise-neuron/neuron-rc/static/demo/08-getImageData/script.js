// webrtc script file
var webrtc = (function() {

    var getVideo = true,
        getAudio = false,

        video = document.getElementById('webcam'),
        feed = document.getElementById('feed'),
        feedContext = feed.getContext('2d'),
        display = document.getElementById('display'),
        displayContext = display.getContext('2d');

    navigator.getUserMedia ||
        (navigator.getUserMedia = navigator.mozGetUserMedia ||
        navigator.webkitGetUserMedia || navigator.msGetUserMedia);

    window.audioContext ||
        (window.audioContext = window.webkitAudioContext);

    window.requestAnimationFrame ||
        (window.requestAnimationFrame = window.webkitRequestAnimationFrame || 
            window.mozRequestAnimationFrame || 
            window.oRequestAnimationFrame || 
            window.msRequestAnimationFrame || 
            function( callback ){
                window.setTimeout(callback, 1000 / 60);
            });

    function onSuccess(stream) {
        var videoSource,
            audioContext,
            mediaStreamSource;

        if (getVideo) {
            if (window.webkitURL) {
                videoSource = window.webkitURL.createObjectURL(stream);
            } else {
                videoSource = stream;
            }

            video.autoplay = true;
            video.src = videoSource;

            display.width = feed.width = 320;
            display.height = feed.height = 240;

            streamFeed();
        }

        if (getAudio && window.audioContext) {
            audioContext = new window.audioContext();
            mediaStreamSource = audioContext.createMediaStreamSource(stream);
            mediaStreamSource.connect(audioContext.destination);
        }
    }

    function onError() {
        alert('There has been a problem retreiving the streams - are you running on file:/// or did you disallow access?');
    }

    function takePhoto() {
        var photo = document.getElementById('photo'),
            context = photo.getContext('2d');

        photo.width = display.width;
        photo.height = display.height;

        context.drawImage(video, 0, 0, photo.width, photo.height);
    }

    function requestStreams() {
        if (navigator.getUserMedia) {
            navigator.getUserMedia({
                video: getVideo,
                audio: getAudio
            }, onSuccess, onError);
        } else {
            alert('getUserMedia is not supported in this browser.');
        }
    }

    function streamFeed() {
        requestAnimationFrame(streamFeed);
        feedContext.drawImage(video, 0, 0, display.width, display.height);
        imageData = feedContext.getImageData(0, 0, display.width, display.height);
        displayContext.putImageData(imageData, 0, 0);
    }

    function initEvents() {
        var photoButton = document.getElementById('takePhoto');
        photoButton.addEventListener('click', takePhoto, false);
    }

    (function init() {
        requestStreams();
        initEvents();
    }());
})();



