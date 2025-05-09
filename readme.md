# Raspberry Pi Pico W Flashcard Viewer

## 📘 Description

This Raspberry Pi Pico W project enables the Pico W to:

- Download flashcards (in CSV format) from GitHub Pages via HTTPS  
- Display them on Waveshare's Pico-OLED-1.3 display (over SPI)  
- Show a **random flashcard every 60 seconds**  
- Allow the user to:  
  - Press **KEY1** to flip the card (front/back)  
  - Press **KEY0** to instantly get a new card  
- Automatically **split long flashcards into multiple pages**, displaying each page for 5 seconds

---

## 🎥 Demo
- Watch a short demo of the project in action
- The first part of the video demonstrates how to set up the project.  
- The second part showcases its functionality (**skip to 5:23** to jump straight to that).  

**[▶️ Video Demo](https://your.video.link.here)**
---

## 🚀 How to Use

### 1. Uploading Your Cards

1. Convert your flashcards into CSV format using the `"front","back"` structure.  
2. To simplify this, a Python script (courtesy of ChatGPT) is provided to convert Anki `.apkg` files into this format.  
3. Steps:
   - Ensure **Python 3.4 or later** is installed.
   - Navigate to the `CSV_Conversion_Python_Script` folder.
   - Copy your `.apkg` file into this folder (exported from the Anki desktop app).  
     > **Note:** When exporting from Anki, **deselect all checkboxes** except for **"Support older Anki versions (slower/larger files)"**, which must be selected.
   - Run the script:
     ```bash
     python yourfile.apkg
     ```
   - This will generate a `cards.csv` file. **Do not rename this file.**
   - Create a GitHub Pages repo and upload your `cards.csv` file to the root.

### 2. Setting Up the Microcontroller

1. Use a **Raspberry Pi Pico W with headers** and attach the **Waveshare 1.3" OLED** display.  
2. Configure your Wi-Fi:
   - Open `picohttps.h` inside `PICO_Flashcard_Display_C/lib/HTTPS`
   - Edit the following lines:
     ```c
     #define PICOHTTPS_WIFI_SSID     "YOUR_WIFI_SSID"
     #define PICOHTTPS_WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
     #define PICOHTTPS_HOSTNAME      "username.github.io"
     ```
   - You may also need to update the SSL certificate if it's outdated.

3. Install the following:
   - Raspberry Pi Pico SDK
   - CMake (≥ 3.13)
   - ARM GCC Toolchain
   - Ninja
   - Set them as environment variables

4. Build the project:
   ```bash
   cd PICO_Flashcard_Display_C/build
   cmake -G "Ninja" ..
   ninja
   ```
5. A `main.uf2` file will be created. **Flash this to your Pico** — the program starts automatically.

---

## ⚠️ Limitations

- The Pico stores flashcards in memory; size is limited.  
- In `picohttps.c`, the max response size is set to **16 KB** — increase with caution.  
- Wi-Fi may take time to connect if the signal is weak. The Pico will keep retrying until successful.

---

## 🙏 Credit

- HTTPS functionality based on: [picohttps](https://github.com/marceloalcocer/picohttps)  
- `.apkg` to CSV Python script and flashcard parser in `main.c` generated with help from **ChatGPT**
