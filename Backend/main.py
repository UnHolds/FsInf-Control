from flask import Flask, request
import serial
import threading
import requests

ser = serial.Serial('/dev/ttyUSB0', baudrate=115200)
app = Flask(__name__)
open = False

key = "SUPER_SECURE_KEY"


@app.route("/open", methods=['POST'])
def open_fsinf():
    content = request.json
    if content['key'] != key:
        return 'Nope'
    ser.write(b'O')
    return "Opened"


@app.route("/close", methods=['POST'])
def close_fsinf():
    content = request.json
    if content['key'] != key:
        return 'Nope'
    ser.write(b'C')
    return "Closed"

@app.route("/play", methods=['POST'])
def play_fsinf():
    content = request.json
    if content['key'] != key:
        return 'Nope'
    ser.write(b'1')
    return "Play"

@app.route("/led", methods=['POST'])
def led_fsinf():
    content = request.json
    if content['key'] != key:
        return 'Nope'
    ser.write(b'2')
    return "Led"


def send_request(msg_text, cmd_text):
    url = "https://mm.fsinf.at/api/bot/is-it-open"
    jdata = {
        "msg_state": msg_text,
        "cmd_state": cmd_text
    }

    requests.post(url, json=jdata)

def read_serial():
    global open
    while(True):
        r = ser.read(1)
        if (r == b'C' or r == b'c') and open:
            send_request("## :stop_sign: FsInf is closing - Pfiad di!", "## Naaa, oida!")
            open = False
        if (r == b'O' or r == b'o') and not open:
            send_request("## :white_check_mark: FsInf opening - Kumm ume!", "## Fix oida!")
            open = True

if __name__ == '__main__':
    t = threading.Thread(target=read_serial)
    t.start()
    app.run()
