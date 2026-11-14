(() => {
    const canvas = document.getElementById('canvas');
    const status = document.getElementById('storage-status');
    const loading = document.getElementById('loading');
    const updateButton = document.getElementById('btn-update');
    const availableUpdates = new Set();
    const buttons = document.querySelectorAll('nav button');

    function reportError(error) {
        console.error(error);
        status.textContent = `Error: ${error.message || error}`;
    }

    const emulator = new ZealNative({
        canvas,
        romdisk: 'default.img',
        eeprom: 'eeprom.img',
        tf: 'tf.img',
        imageStorage: 'opfs',
        onImageUpdateAvailable(image) {
            availableUpdates.add(image.name);
            updateButton.hidden = false;
            status.textContent = `Updated images available: ${[...availableUpdates].join(', ')}. ` +
                'Choose Update Images to replace saved copies.';
        },
        onStorageError: reportError,
        onLoadingChange(busy) {
            buttons.forEach(button => { button.disabled = busy; });
            loading.hidden = !busy;
        },
    });

    function attachButton(id, handler) {
        document.getElementById(id).addEventListener('click', async () => {
            try {
                await handler();
            } catch (error) {
                reportError(error);
            } finally {
                canvas.focus();
            }
        });
    }

    function clearUpdateNotice() {
        availableUpdates.clear();
        updateButton.hidden = true;
    }

    // Receiving a key in WASM does not cancel the browser's scrolling action.
    // Keep these events available to the runtime; only suppress their defaults.
    canvas.addEventListener('keydown', event => {
        if ([' ', 'ArrowUp', 'ArrowDown', 'ArrowLeft', 'ArrowRight',
            'PageUp', 'PageDown', 'Home', 'End'].includes(event.key)) {
            event.preventDefault();
        }
    });

    document.getElementById('viewport').addEventListener('click', () => canvas.focus());
    attachButton('btn-reset', () => emulator.reload());
    attachButton('btn-unmute', () => emulator.resumeAudio());
    attachButton('btn-toggle-fps', () => emulator.toggleFps());
    attachButton('btn-clear', async () => {
        if (!window.confirm('Delete saved disk images and restart with the deployed images? Local changes and guest RAM will be lost.')) return;
        await emulator.clearDiskImages();
        clearUpdateNotice();
        status.textContent = 'Saved images cleared; deployed images loaded.';
    });
    attachButton('btn-update', async () => {
        if (!window.confirm(`Replace saved images (${[...availableUpdates].join(', ')}) with the deployed versions? Local changes to these images and guest RAM will be lost.`)) return;
        await emulator.applyImageUpdates([...availableUpdates]);
        clearUpdateNotice();
        status.textContent = 'Updated images loaded.';
    });

    window.addEventListener('load', () => emulator.start().catch(reportError));
})();
