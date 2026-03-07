document.addEventListener('DOMContentLoaded', () => {
    const songList = document.getElementById('song-list');
    const playPauseBtn = document.getElementById('play-pause-btn');
    const prevBtn = document.getElementById('prev-btn');
    const nextBtn = document.getElementById('next-btn');
    const shuffleBtn = document.getElementById('shuffle-btn');
    const repeatBtn = document.getElementById('repeat-btn');
    const currentSongEl = document.getElementById('current-song');
    const currentTimeEl = document.getElementById('current-time');
    const totalDurationEl = document.getElementById('total-duration');

    let songs = [];
    let currentSongIndex = -1;
    let isShuffle = false;
    let isRepeat = false;

    const hostPort = window.location.origin;

    fetch(`${hostPort}/music`)
        .then(response => response.json())
        .then(data => {
            songs = data;
            renderSongList();
        })
        .catch(error => console.error('Error fetching songs:', error));

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
    }

    // Play a song
    function playSong(index) {
        if (index >= 0 && index < songs.length) {
            currentSongIndex = index;
            const song = songs[currentSongIndex];

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

    // TODO: Eventually use websockets to get updates when the song changes?
    function updateUI() {
        const currentSong = songs[currentSongIndex];
        if (currentSong) {
            currentSongEl.textContent = `${currentSong.name} - ${currentSong.artist}`;
        } else {
            currentSongEl.textContent = 'No song selected';
        }

        document.querySelectorAll('.song-item').forEach((item, index) => {
            if (index === currentSongIndex) {
                item.classList.add('active');
            } else {
                item.classList.remove('active');
            }
        });
    }

    // Play/pause button
    playPauseBtn.addEventListener('click', () => {
        fetch(`${hostPort}/music/media/toggle_pause`)
            .catch(error => console.error('Error toggling between play and pause:', error));
    });

    // Previous button
    prevBtn.addEventListener('click', () => {
        playPreviousSong();
        prevBtn.classList.add('clicked');
        setTimeout(() => prevBtn.classList.remove('clicked'), 200);
    });

    // Next button
    nextBtn.addEventListener('click', () => {
        playNextSong();
        nextBtn.classList.add('clicked');
        setTimeout(() => nextBtn.classList.remove('clicked'), 200);
    });

    // Shuffle button
    shuffleBtn.addEventListener('click', () => {
        isShuffle = !isShuffle;
        shuffleBtn.classList.toggle('active', isShuffle);
    });

    function formatTime(seconds) {
        const minutes = Math.floor(seconds / 60);
        const secs = Math.floor(seconds % 60);
        return `${minutes}:${secs < 10 ? '0' : ''}${secs}`;
    }

    function getRandomIndex() {
        let index = Math.floor(Math.random() * songs.length);
        while (index === currentSongIndex) {
            index = Math.floor(Math.random() * songs.length);
        }
        return index;
    }
});