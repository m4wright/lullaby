document.addEventListener('DOMContentLoaded', () => {
    const songList = document.getElementById('song-list');
    const playPauseBtn = document.getElementById('play-pause-btn');
    const prevBtn = document.getElementById('prev-btn');
    const nextBtn = document.getElementById('next-btn');
    const volumeSlider = document.getElementById('volume-slider');
    const volumeDisplay = document.getElementById('volume-display');
    const timerDisplay = document.getElementById('timer-display');
    const timerModal = document.getElementById('timer-modal');
    const timerModalClose = document.getElementById('timer-modal-close');
    const timerEnabledCheckbox = document.getElementById('timer-enabled-checkbox');
    const timerDurationSelect = document.getElementById('timer-duration-select');
    const timerStatusText = document.getElementById('timer-status-text');

    let songs = [];
    let currentSongIndex = -1;
    let isPlaying = false;
    let currentVolume = 100;

    // Timer state
    let timerEnabled = false;
    let timerDuration = 15; // minutes
    let timerRemaining = null; // seconds, null when not counting down

    const hostPort = window.location.origin;

    getMusicStatus();

    const eventSource = new EventSource(`${hostPort}/music/subscribe/now-playing`);

    initEventSource();

    function updateNowPlayingUI(name, artist, playing, volume, timerState) {

        isPlaying = playing;

        console.log("Updating now playing UI: ", { name, artist, playing });

        // Update the volume slider
        currentVolume = Math.round(volume);
        volumeSlider.value = currentVolume;
        volumeDisplay.textContent = currentVolume + '%';

        // Update timer state from backend
        if (timerState) {
            updateTimerState(timerState.enabled, timerState.duration_minutes, timerState.remaining_seconds);
        }

        // Also update the active song in the list if name/artist are provided
        if (name && artist) {
            const index = songs.findIndex(s => s.name === name && s.artist === artist);
            if (index !== -1) {
                currentSongIndex = index;
            }
        }

        updateSongListActiveState();
    }

    function updateTimerState(enabled, duration, remaining) {
        timerEnabled = enabled;
        timerDuration = duration;
        timerRemaining = remaining;

        // Update timer display
        if (timerEnabled && timerRemaining !== null) {
            const mins = Math.floor(timerRemaining / 60);
            const secs = timerRemaining % 60;
            timerDisplay.textContent = `⏱ ${mins}:${secs.toString().padStart(2, '0')}`;
            timerDisplay.classList.add('active');
        } else {
            timerDisplay.textContent = '⏱ --:--';
            timerDisplay.classList.remove('active');
        }

        // Update modal UI
        timerEnabledCheckbox.checked = timerEnabled;
        timerDurationSelect.value = timerDuration.toString();
        timerStatusText.textContent = timerEnabled && timerRemaining !== null
            ? `Status: Active - ${Math.floor(timerRemaining / 60)}:${(timerRemaining % 60).toString().padStart(2, '0')} remaining`
            : 'Status: Inactive';
    }

    function initEventSource() {
        eventSource.onmessage = (event) => {
            const message = JSON.parse(event.data);
            updateNowPlayingUI(message.name, message.artist, message.playing, message.volume, message.timer);
        };
    }

    // New function to manage active state in song list based on currentSongIndex
    function updateSongListActiveState() {

        let currentSongClass = (isPlaying ? 'active' : 'paused');

        document.querySelectorAll('.song-item').forEach((item, index) => {
            item.classList.remove('active');
            item.classList.remove('paused');

            if (index === currentSongIndex) {
                item.classList.add(currentSongClass);
            }
        });
    }
    function getMusicStatus() {
        fetch(`${hostPort}/music`)
            .then(response => response.json())
            .then(status => {
                songs = status['songs'];

                var nowPlaying = status.now_playing;

                updateNowPlayingUI(nowPlaying.name, nowPlaying.artist, nowPlaying.playing, nowPlaying.volume, status.timer);

                renderSongList();
            })
            .catch(error => console.error('Error fetching songs:', error));
    }


    // Render song list
    function renderSongList() {
        songList.innerHTML = '';
        songs.forEach((song, index) => {
            const li = document.createElement('li');
            li.classList.add('song-item');
            li.dataset.index = index;
            li.innerHTML = `
                <div class="song-item-info">
                    <div>${song.name}</div>
                    <div class="artist">${song.artist}</div>
                </div>
                <div class="play-icon">▶</div>
            `;
            li.addEventListener('click', () => {
                playSong(index);
            });
            songList.appendChild(li);
        });
        updateSongListActiveState();
    }

    function playSong(index) {
        if (index >= 0 && index < songs.length) {
            const song = songs[index];

            fetch(`${hostPort}/music/media/play?name=${encodeURIComponent(song.name)}&artist=${encodeURIComponent(song.artist)}`)
                .catch(error => console.error('Error playing song:', error));
        }
    }

    function playPreviousSong() {
        fetch(`${hostPort}/music/media/play_previous`)
            .catch(error => console.error('Error playing previous song:', error));
    }

    function playNextSong() {
        fetch(`${hostPort}/music/media/play_next`)
            .catch(error => console.error('Error playing next song:', error));
    }


    playPauseBtn.addEventListener('click', () => {
        fetch(`${hostPort}/music/media/toggle_pause`)
            .catch(error => console.error('Error toggling between play and pause:', error));
    });

    prevBtn.addEventListener('click', () => {
        playPreviousSong();
        prevBtn.classList.add('clicked');
        setTimeout(() => prevBtn.classList.remove('clicked'), 200);
    });

    nextBtn.addEventListener('click', () => {
        playNextSong();
        nextBtn.classList.add('clicked');
        setTimeout(() => nextBtn.classList.remove('clicked'), 200);
    });

    // Volume control
    function getVolume() {
        fetch(`${hostPort}/volume`)
            .then(response => response.json())
            .then(data => {
                currentVolume = Math.round(data.volume);
                volumeSlider.value = currentVolume;
                volumeDisplay.textContent = currentVolume + '%';
            })
            .catch(error => console.error('Error fetching volume:', error));
    }

    function setVolume(volume) {
        fetch(`${hostPort}/volume?volume=${volume}`, { method: 'PUT' })
            .catch(error => console.error('Error setting volume:', error));
    }

    volumeSlider.addEventListener('input', () => {
        const volume = volumeSlider.value;
        volumeDisplay.textContent = volume + '%';
        setVolume(volume);
    });

    // Timer modal handling
    timerDisplay.addEventListener('click', () => {
        timerModal.classList.add('show');
    });

    timerModalClose.addEventListener('click', () => {
        timerModal.classList.remove('show');
    });

    timerModal.addEventListener('click', (e) => {
        if (e.target === timerModal) {
            timerModal.classList.remove('show');
        }
    });

    // Timer enabled toggle
    timerEnabledCheckbox.addEventListener('change', () => {
        const enabled = timerEnabledCheckbox.checked;
        fetch(`${hostPort}/timer/enabled?enabled=${enabled}`, { method: 'POST' })
            .catch(error => console.error('Error toggling timer:', error));
    });

    // Timer duration change
    timerDurationSelect.addEventListener('change', () => {
        const duration = parseInt(timerDurationSelect.value, 10);
        fetch(`${hostPort}/timer/duration?minutes=${duration}`, { method: 'POST' })
            .catch(error => console.error('Error setting timer duration:', error));
    });

    // Countdown timer update every second
    setInterval(() => {
        if (timerEnabled && timerRemaining !== null && timerRemaining > 0) {
            timerRemaining--;
            const mins = Math.floor(timerRemaining / 60);
            const secs = timerRemaining % 60;
            timerDisplay.textContent = `⏱ ${mins}:${secs.toString().padStart(2, '0')}`;
            timerStatusText.textContent = `Status: Active - ${mins}:${secs.toString().padStart(2, '0')} remaining`;
        }
    }, 1000);
});