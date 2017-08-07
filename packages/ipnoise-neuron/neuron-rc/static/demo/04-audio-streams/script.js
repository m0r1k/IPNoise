// webrtc script file

navigator.getUserMedia ||
  (navigator.getUserMedia = navigator.mozGetUserMedia ||
  navigator.webkitGetUserMedia || navigator.msGetUserMedia);

window.audioContext ||
  (window.audioContext = window.webkitAudioContext);

if (navigator.getUserMedia) {
    navigator.getUserMedia({
      video: true,
      audio: true
    }, onSuccess, onError);
} else {
    alert('getUserMedia is not supported in this browser.');
}

function onSuccess(stream) {
    var video = document.getElementById('webcam');
    var videoSource;
    var audioContext;
    var mediaStreamSource;

    if (window.webkitURL) {
      videoSource = window.webkitURL.createObjectURL(stream);
    } else {
      videoSource = stream;
    }

    video.autoplay = true;
    video.src = videoSource;

    if (window.audioContext) {
        audioContext = new window.audioContext();
        mediaStreamSource = audioContext.createMediaStreamSource(stream);
        mediaStreamSource.connect(audioContext.destination);
    }
}


function onError() {
    alert('There has been a problem retreiving the streams - are you running on file:/// or did you disallow access?');
}