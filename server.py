import hashlib
import json
import threading
from time import time
from flask import Flask, jsonify, request, send_file # Added 'request' and 'send_file'
from flask_cors import CORS
from twilio.rest import Client

import os
from dotenv import load_dotenv

load_dotenv()

# --- TWILIO SETUP ---
TWILIO_ACCOUNT_SID = os.getenv('TWILIO_ACCOUNT_SID')
TWILIO_AUTH_TOKEN = os.getenv('TWILIO_AUTH_TOKEN')
TWILIO_PHONE_NUMBER = os.getenv('TWILIO_PHONE_NUMBER')
YOUR_CELL_PHONE = os.getenv('YOUR_CELL_PHONE')

twilio_client = None
if TWILIO_ACCOUNT_SID and TWILIO_AUTH_TOKEN:
    try:
        twilio_client = Client(TWILIO_ACCOUNT_SID, TWILIO_AUTH_TOKEN)
    except Exception as e:
        print(f"Twilio initialization error: {e}")

def send_alert_sms():
    if not twilio_client:
        print("Twilio client not configured. Check your .env file.")
        return
    try:
        message = twilio_client.messages.create(
            body="🚨 ALARM ACTIVATED: Forced entry detected at the main door!",
            from_=TWILIO_PHONE_NUMBER,
            to=YOUR_CELL_PHONE
        )
        print(f"SMS Alert sent successfully! Message SID: {message.sid}")
    except Exception as e:
        print(f"Failed to send SMS: {e}")

class Blockchain:
    def __init__(self):
        self.chain = []
        self.create_block(previous_hash='1', data={"event": "System Started", "uid": "SYSTEM"})

    def create_block(self, previous_hash, data):
        block = {
            'index': len(self.chain) + 1,
            'timestamp': time(),
            'data': data,
            'previous_hash': previous_hash or self.hash(self.chain[-1]),
        }
        block['hash'] = self.hash(block)
        self.chain.append(block)
        return block

    @staticmethod
    def hash(block):
        block_string = json.dumps(block, sort_keys=True).encode()
        return hashlib.sha256(block_string).hexdigest()

app = Flask(__name__)
CORS(app)
blockchain = Blockchain()

# ---> NEW: WIRELESS RECEIVER ENDPOINT <---
@app.route('/add_event', methods=['POST'])
def add_event():
    # Catch the JSON payload from the ESP32
    data = request.json
    
    if not data or 'event' not in data or 'uid' not in data:
        return jsonify({"message": "Invalid data format"}), 400

    print(f"Wireless Event Received! {data['event']} | UID: {data['uid']}")

    # Check for Forced Entry and trigger Twilio SMS in the background
    if data['event'] == "FORCED_ENTRY":
        print("⚠️ FORCED ENTRY DETECTED! Triggering Twilio SMS in background...")
        sms_thread = threading.Thread(target=send_alert_sms)
        sms_thread.start()

    # Add block to the blockchain
    previous_hash = blockchain.chain[-1]['hash']
    block = blockchain.create_block(previous_hash, data)
    
    # Let the ESP32 know we received it successfully
    return jsonify({"message": "Block added", "block_index": block['index']}), 201

# ---> NEW: SERVE THE DASHBOARD <---
@app.route('/')
def home():
    # This tells Flask to send your index.html file when someone visits the main page
    return send_file('index.html')

# The endpoint for your HTML Dashboard
@app.route('/chain', methods=['GET'])
def get_chain():
    return jsonify({'chain': blockchain.chain, 'length': len(blockchain.chain)}), 200

if __name__ == '__main__':
    # CHANGED: host='0.0.0.0' allows external devices (the ESP32) on the network to connect
    print("--- Wi-Fi Blockchain Server Running ---")
    app.run(host='0.0.0.0', port=5000)