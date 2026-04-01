document.addEventListener('DOMContentLoaded', () => {
    const songTableBody = document.getElementById('song-table-body');
    const addSongForm = document.getElementById('add-song-form');
    const statusMessage = document.getElementById('status-message');

    const hostPort = window.location.origin;

    loadSongs();

    function showStatus(message, isSuccess) {
        statusMessage.textContent = message;
        statusMessage.classList.remove('hidden', 'status-success', 'status-error');
        statusMessage.classList.add(isSuccess ? 'status-success' : 'status-error');

        setTimeout(() => {
            statusMessage.classList.add('hidden');
        }, 3000);
    }

    function loadSongs() {
        fetch(`${hostPort}/admin/songs`)
            .then(response => response.json())
            .then(songs => renderSongTable(songs))
            .catch(error => {
                console.error('Error loading songs:', error);
                showStatus('Failed to load songs', false);
            });
    }

    function renderSongTable(songs) {
        songTableBody.innerHTML = '';

        songs.forEach((song, index) => {
            const tr = document.createElement('tr');
            tr.dataset.index = index;
            tr.innerHTML = `
                <td class="view-mode">${song.name}</td>
                <td class="view-mode">${song.artist}</td>
                <td class="view-mode">${song.path}</td>
                <td class="actions">
                    <button class="btn btn-secondary btn-edit">Edit</button>
                    <button class="btn btn-danger btn-delete">Delete</button>
                </td>
            `;
            songTableBody.appendChild(tr);
        });

        // Attach event listeners
        document.querySelectorAll('.btn-edit').forEach((btn, index) => {
            btn.addEventListener('click', () => startEdit(index, songs[index]));
        });

        document.querySelectorAll('.btn-delete').forEach((btn, index) => {
            btn.addEventListener('click', () => deleteSong(index, songs[index]));
        });
    }

    function startEdit(index, song) {
        const row = songTableBody.children[index];

        row.innerHTML = `
            <td class="edit-row"><input type="text" class="edit-name" value="${escapeHtml(song.name)}"></td>
            <td class="edit-row"><input type="text" class="edit-artist" value="${escapeHtml(song.artist)}" disabled></td>
            <td class="edit-row"><input type="text" class="edit-path" value="${escapeHtml(song.path)}"></td>
            <td class="actions">
                <button class="btn btn-primary btn-save">Save</button>
                <button class="btn btn-secondary btn-cancel">Cancel</button>
            </td>
        `;

        row.querySelector('.btn-save').addEventListener('click', () => {
            saveEdit(index, song);
        });

        row.querySelector('.btn-cancel').addEventListener('click', () => {
            loadSongs();
        });
    }

    function saveEdit(index, originalSong) {
        const row = songTableBody.children[index];
        const newName = row.querySelector('.edit-name').value.trim();
        const newPath = row.querySelector('.edit-path').value.trim();

        if (!newName || !newPath) {
            showStatus('Name and path are required', false);
            return;
        }

        // Only update path (name/artist are the key)
        fetch(`${hostPort}/admin/songs`, {
            method: 'PUT',
            headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
            body: `name=${encodeURIComponent(originalSong.name)}&artist=${encodeURIComponent(originalSong.artist)}&path=${encodeURIComponent(newPath)}`
        })
        .then(response => {
            if (response.ok) {
                showStatus('Song updated successfully', true);
                loadSongs();
            } else {
                return response.text().then(text => { throw new Error(text); });
            }
        })
        .catch(error => {
            console.error('Error updating song:', error);
            showStatus('Failed to update song: ' + error.message, false);
        });
    }

    function deleteSong(index, song) {
        if (!confirm(`Delete "${song.name}" by "${song.artist}"?`)) {
            return;
        }

        fetch(`${hostPort}/admin/songs?name=${encodeURIComponent(song.name)}&artist=${encodeURIComponent(song.artist)}`, {
            method: 'DELETE'
        })
        .then(response => {
            if (response.ok) {
                showStatus('Song deleted successfully', true);
                loadSongs();
            } else {
                return response.text().then(text => { throw new Error(text); });
            }
        })
        .catch(error => {
            console.error('Error deleting song:', error);
            showStatus('Failed to delete song: ' + error.message, false);
        });
    }

    addSongForm.addEventListener('submit', (e) => {
        e.preventDefault();

        const name = document.getElementById('song-name').value.trim();
        const artist = document.getElementById('song-artist').value.trim();
        const path = document.getElementById('song-path').value.trim();

        if (!name || !artist || !path) {
            showStatus('All fields are required', false);
            return;
        }

        fetch(`${hostPort}/admin/songs`, {
            method: 'POST',
            headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
            body: `name=${encodeURIComponent(name)}&artist=${encodeURIComponent(artist)}&path=${encodeURIComponent(path)}`
        })
        .then(response => {
            if (response.ok) {
                showStatus('Song added successfully', true);
                addSongForm.reset();
                loadSongs();
            } else {
                return response.text().then(text => { throw new Error(text); });
            }
        })
        .catch(error => {
            console.error('Error adding song:', error);
            showStatus('Failed to add song: ' + error.message, false);
        });
    });

    function escapeHtml(text) {
        const div = document.createElement('div');
        div.textContent = text;
        return div.innerHTML;
    }
});
