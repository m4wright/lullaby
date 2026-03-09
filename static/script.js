document.addEventListener('DOMContentLoaded', () => {
    const songList = document.getElementById('song-list');
    const playPauseBtn = document.getElementById('play-pause-btn');
    const prevBtn = document.getElementById('prev-btn');
    const nextBtn = document.getElementById('next-btn');

    let songs = [];
    let currentSongIndex = -1;
    let isPlaying = false;

    const hostPort = window.location.origin;

    getMusicStatus();

    const eventSource = new EventSource(`${hostPort}/music/subscribe/now-playing`);

    initEventSource();

    function updateNowPlayingUI(name, artist, playing) {

        isPlaying = playing;

        console.log("Updating now playing UI: ", { name, artist, playing });

        // Also update the active song in the list if name/artist are provided
        if (name && artist) {
            const index = songs.findIndex(s => s.name === name && s.artist === artist);
            if (index !== -1) {
                currentSongIndex = index;
            }
        }

        updateSongListActiveState();
    }

    function initEventSource() {
        eventSource.onmessage = (event) => {
            const message = JSON.parse(event.data);
            updateNowPlayingUI(message.name, message.artist, message.playing);
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

                updateNowPlayingUI(nowPlaying.name, nowPlaying.artist, nowPlaying.playing);

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
});