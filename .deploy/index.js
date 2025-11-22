const DB_NAME = "ZealStorage";
const DB_VERSION = 1;
const STORE_NAME = "images";

function openDB() {
  return new Promise((resolve, reject) => {
    const request = indexedDB.open(DB_NAME, DB_VERSION);

    request.onerror = () => reject(request.error);
    request.onsuccess = () => resolve(request.result);

    request.onupgradeneeded = (event) => {
      const db = event.target.result;
      if (!db.objectStoreNames.contains(STORE_NAME)) {
        db.createObjectStore(STORE_NAME);
      }
    };
  });
}

async function saveImage(name, data) {
  const db = await openDB();
  return new Promise((resolve, reject) => {
    const transaction = db.transaction([STORE_NAME], "readwrite");
    const store = transaction.objectStore(STORE_NAME);
    const request = store.put(data, name);
    console.log("saveImage", name);

    request.onsuccess = () => resolve();
    request.onerror = () => reject(request.error);
  });
}

async function loadImage(name) {
  const db = await openDB();
  return new Promise((resolve, reject) => {
    const transaction = db.transaction([STORE_NAME], "readonly");
    const store = transaction.objectStore(STORE_NAME);
    const request = store.get(name);

    request.onsuccess = () => resolve(request.result);
    request.onerror = () => reject(request.error);
  });
}

async function deleteImage(name) {
  const db = await openDB();
  return new Promise((resolve, reject) => {
    const transaction = db.transaction([STORE_NAME], "readwrite");
    const store = transaction.objectStore(STORE_NAME);
    const request = store.delete(name);
    console.log("deleteImage", name);

    request.onsuccess = () => resolve();
    request.onerror = () => reject(request.error);
  });
}

function dispatchKeyCode(e, name, key, code, keyCode) {
  e.preventDefault();
  e.stopPropagation();
  const synthetic = new KeyboardEvent(name, {
    key,
    code,
    keyCode,
    which: keyCode,
    bubbles: true,
    cancelable: true,
  });
  e.target.dispatchEvent(synthetic);
}

function attachButton(selector, handler) {
    document.querySelector(selector).addEventListener('click', handler);
}

(() => {
  const canvas = document.getElementById("canvas");
  const viewport = document.getElementById('viewport');
  viewport.addEventListener('click', () => {
    canvas.focus();
  });

  let moduleInstance = null;
  function loadModule(eepromImage, tfImage) {
    const defaultModule = {
      arguments: ["-e", "eeprom.img", "-t", "tf.img"],
      print: function (text) {
        console.log("Log: " + text);
      },
      printErr: function (text) {
        console.log("Error: " + text);
      },
      canvas: (function () {
        return canvas;
      })(),
      onRuntimeInitialized: function () {
        this.FS.writeFile("/eeprom.img", eepromImage);
        this.FS.writeFile("/tf.img", tfImage);
        canvas.setAttribute("tabindex", "0");
        canvas.focus();
      },
    };
    NativeModule(defaultModule).then((mod) => (moduleInstance = mod));
  }

  async function load() {
    let eepromImage = await loadImage("eeprom.img");
    let tfImage = await loadImage("tf.img");

    if (!eepromImage || !tfImage) {
      console.log("Fetching images over network...");
      [eepromImage, tfImage] = await Promise.all([
        fetch("eeprom.img")
          .then((response) => {
            if (!response.ok) throw new Error("Failed to fetch eeprom.img");
            return response.arrayBuffer();
          })
          .then((buffer) => new Uint8Array(buffer))
          .then(async (file) => {
            console.log("saving", "eeprom.img");
            await saveImage("eeprom.img", file);
            return file;
          }),
        fetch("tf.img")
          .then((response) => {
            if (!response.ok) throw new Error("Failed to fetch tf.img");
            return response.arrayBuffer();
          })
          .then((buffer) => new Uint8Array(buffer))
          .then(async (file) => {
            console.log("saving", "tf.img");
            await saveImage("tf.img", file);
            return file;
          }),
      ]).catch((err) => console.error(err));
    } else {
      console.log("Images loaded from IndexDB");
    }
    loadModule(eepromImage, tfImage);
  }

  async function reset() {
    moduleInstance?._zeal_exit();
    moduleInstance = null;
    setTimeout(() => {
      load();
    }, 100);
  }

  window.addEventListener("load", load);

  function resumeAudioIfNeeded() {
    if (
      NativeModule.audioContext &&
      NativeModule.audioContext.state === "suspended"
    ) {
      NativeModule.audioContext.resume().then(() => {
        console.log("Audio context resumed");
      });
    }
    canvas.focus();
  }

  attachButton("#btn-unmute", resumeAudioIfNeeded);
  attachButton("#btn-reset", reset);

  window.toggleFPS = () => {
    if (!moduleInstance) return;
    const show_fps = !!moduleInstance.getValue(moduleInstance._show_fps, "i8");
    console.log("show_fps", show_fps);
    moduleInstance.setValue(moduleInstance._show_fps, !show_fps ? 1 : 0, "i8");
    canvas.focus();
  };
  attachButton("#btn-toggle-fps", toggleFPS);

  attachButton('#btn-save', async () => {
    if(!moduleInstance) {
        console.error('no moduleInstance, can not access images');
        return;
    }
    try {
        const eeprom = moduleInstance.FS.readFile('/eeprom.img');
        const tf = moduleInstance.FS.readFile('/tf.img');
        await saveImage('eeprom.img', eeprom);
        await saveImage('tf.img', tf);
        console.log('Images saved');
    } catch(err) {
        console.error('Error saving images', err);
    }
    canvas.focus();
  });
  attachButton('#btn-clear', async () => {
    try {
        await deleteImage('eeprom.img');
        await deleteImage('tf.img');
        console.log('Images deleted');
        return reset();
    } catch(err) {
        console.error("Error deleting images", err);
    }
  });
})();
