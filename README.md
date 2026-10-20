# Ether-Log: Hardware Entry Logging & Blockchain Security Prototype 🛡️⛓️

A smart IoT access control and tamper-evident security system. It pairs an **ESP32 microcontroller** (with RFID, OLED display, door sensors, and audio-visual indicators) with a **Flask-powered cryptographic blockchain backend**, instant **Twilio SMS alerts**, and a **real-time live dashboard**.

---

## 🌟 Key Features

- **RFID Access Control**: Authenticates physical RFID keycards/tags using the MFRC522 module.
- **Door Tamper & Forced Entry Detection**: Monitors door magnetic sensors. Detects unauthorized physical door opens and immediately triggers local and remote alarms.
- **Cryptographic Blockchain Ledger**: Every entry, denial, and door event is immutably recorded into a SHA-256 hashed blockchain maintained by the Flask backend.
- **Automated SMS Emergency Alerts**: Dispatches background SMS notifications to administrators via Twilio whenever an intrusion or forced entry is detected.
- **Live Dark-Mode Web Dashboard**: Responsive UI built with Tailwind CSS displaying real-time metrics, system status, interactive filters, toast notifications, and block hash records.
- **Secure Credentials Architecture**: Hardware and server credentials (Wi-Fi, Twilio auth tokens, server IPs) are decoupled from the codebase using `.env` and `secrets.h`.

---

## 🏗️ Architecture Overview

```
[ ESP32 + MFRC522 RFID + Door Sensor + OLED ]
                      │
           (HTTP POST JSON / Wi-Fi)
                      ▼
             [ Flask Server API ]
            ┌─────────┴─────────┐
            ▼                   ▼
   [ Blockchain Ledger ]  [ Twilio SMS Alert ]
            │             (On Forced Entry)
   (HTTP GET /chain)
            ▼
   [ Real-Time Web Dashboard ]
```

---

## 🛠️ Hardware Requirements

| Component | Purpose / Pin Connection (ESP32) |
| :--- | :--- |
| **ESP32 Dev Module** | Main Wi-Fi microcontroller |
| **MFRC522 RFID Reader** | SPI: `SS: 5`, `RST: 27`, `SCK: 18`, `MISO: 19`, `MOSI: 23` |
| **SSD1306 OLED Display (128x64)** | I2C: `SDA: 21`, `SCL: 22` |
| **Magnetic Door Sensor** | Digital Input (Pullup): `GPIO 4` |
| **Green LED** | Access Granted Indicator: `GPIO 2` |
| **Red LED** | Access Denied Indicator: `GPIO 15` |
| **Buzzer** | Audio feedback / Alarm: `GPIO 13` |

---

## 📂 Project Structure

```
├── sketch_jul06a/
│   ├── sketch_jul06a.ino      # ESP32 Arduino firmware
│   ├── secrets.h              # Wi-Fi credentials & server endpoint (git-ignored)
│   └── secrets.example.h      # Template for Wi-Fi credentials
├── server.py                  # Flask REST API & Blockchain engine
├── index.html                 # Real-time web dashboard
├── requirements.txt           # Python dependencies
├── .env                       # Twilio & environment secrets (git-ignored)
├── .env.example               # Template for environment variables
└── .gitignore                 # Version control protection
```

---

## 🚀 Getting Started

### 1. Backend Server Setup

1. **Clone the repository**:
   ```bash
   git clone https://github.com/rishabhjaincares-droid/Ether-Log-Hardware-Entry-Logging-Prototype-.git
   cd Ether-Log-Hardware-Entry-Logging-Prototype-
   ```

2. **Create and activate a virtual environment**:
   ```bash
   python -m venv .venv
   # Windows:
   .venv\Scripts\activate
   # macOS/Linux:
   source .venv/bin/activate
   ```

3. **Install dependencies**:
   ```bash
   pip install -r requirements.txt
   ```

4. **Configure environment variables**:
   Copy `.env.example` to `.env` and fill in your Twilio credentials:
   ```env
   TWILIO_ACCOUNT_SID=your_twilio_account_sid
   TWILIO_AUTH_TOKEN=your_twilio_auth_token
   TWILIO_PHONE_NUMBER=your_twilio_phone_number
   YOUR_CELL_PHONE=your_personal_phone_number
   ```

5. **Start the Flask server**:
   ```bash
   python server.py
   ```
   The server runs on `http://0.0.0.0:5000`. You can view the dashboard by navigating to `http://localhost:5000` in your web browser.

---

### 2. ESP32 Firmware Setup

1. Open `sketch_jul06a/sketch_jul06a.ino` in the **Arduino IDE**.
2. Install required Arduino libraries via the Library Manager:
   - `MFRC522`
   - `Adafruit GFX Library`
   - `Adafruit SSD1306`
3. Configure your network credentials in `sketch_jul06a/secrets.h`:
   ```cpp
   const char* ssid = "YOUR_WIFI_SSID";
   const char* password = "YOUR_WIFI_PASSWORD";
   const char* serverName = "http://<YOUR_LAPTOP_IP>:5000/add_event";
   ```
4. Select your ESP32 board and port, then click **Upload**.

---

## 🔒 Security & Privacy

- Sensitive tokens and private phone numbers are excluded from Git history via `.gitignore`.
- Always keep your `.env` and `secrets.h` private. Only share `.env.example` and `secrets.example.h`.

---

## 📜 License

This project is open-source and available under the [MIT License](LICENSE).
