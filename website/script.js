// File: script.js

// === standards ===
function setStandards(){
    document.body.classList.toggle("dark-mode", true);


}

// document.addEventListener("DOMContentLoaded", setStandards);

setStandards();

// === File preview + resize ===
document.querySelectorAll('input[type="file"]').forEach(input => {
    input.addEventListener('change', (event) => {
        const file = event.target.files[0];
        let previewId = input.name + "_preview";
        let img = document.getElementById(previewId);

        if (!file) {
            // --- Input leer → Preview löschen ---
            if (img) img.remove();
            return;
        }

        // --- Preview anzeigen ---
        const reader = new FileReader();
        reader.onload = function(e) {
            if (!img) {
                img = document.createElement("img");
                img.id = previewId;
                img.width = 135;  // Display width
                img.height = 240; // Display height
                input.parentNode.appendChild(img);
            }
            img.src = e.target.result;
        }
        reader.readAsDataURL(file);

        // --- Bild skalieren ---
        resizeImage(file, 135, 240, function(blob){
            console.log("Image resized!", blob);
            // Hier kannst du blob z.B. via fetch() an ESP32 senden
        });
    });
});

// === Bild skalieren Funktion ===
function resizeImage(file, maxWidth, maxHeight, callback) {
    const reader = new FileReader();
    reader.onload = function(e) {
        const img = new Image();
        img.onload = function() {
            const canvas = document.createElement("canvas");
            const ctx = canvas.getContext("2d");

            const ratio = Math.min(maxWidth / img.width, maxHeight / img.height);
            canvas.width = img.width * ratio;
            canvas.height = img.height * ratio;

            ctx.drawImage(img, 0, 0, canvas.width, canvas.height);
            canvas.toBlob(callback, "image/png");
        }
        img.src = e.target.result;
    }
    reader.readAsDataURL(file);
}

// === Form validation ===
document.querySelectorAll('form').forEach(form => {
    form.addEventListener('submit', (e) => {
        let valid = true;
        form.querySelectorAll('input[type="text"], input[type="password"]').forEach(inp => {
            if(inp.value.trim() === "") {
                valid = false;
                inp.style.border = "1px solid red";
            } else {
                inp.style.border = "";
            }
        });
        if (!valid) {
            e.preventDefault();
            alert("Please fill in all fields!");
        }
    });
});

const toggle = document.getElementById("darkModeToggle");

toggle.addEventListener("change", () => {
    document.body.classList.toggle("dark-mode", toggle.checked);
});

