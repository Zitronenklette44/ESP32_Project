// === Form validation ===
document.querySelectorAll('form').forEach(form => {
    form.addEventListener('submit', (e) => {
        let valid = true;
        var autoTime = document.getElementById("autoTime").checked;
        form.querySelectorAll('input[type="text"], input[type="password"], input[type="date"], input[type="time"]').forEach(inp => {
            if(inp.value.trim() === "") {
                if((inp.disabled) && autoTime) return;
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


// === autoTime handling ===
document.getElementById("autoTime").addEventListener('change', () => {
    var self = document.getElementById("autoTime");
    var time = document.getElementById("time");
    var date = document.getElementById("date");

    time.disabled = self.checked;
    date.disabled = self.checked;
});


// === show filename in label ===
document.querySelectorAll('.file-upload input').forEach(input => {
    input.addEventListener('change', () => {
        const fileName = input.files[0]?.name || "Datei Hochladen";
        input.previousElementSibling.textContent = fileName;
    });
});
