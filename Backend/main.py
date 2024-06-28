from flask import Flask, request
import serial
import threading
import requests

ser = serial.Serial('/dev/ttyUSB0', baudrate=115200)
app = Flask(__name__)
open = False

key = 'SUPER_SECURE_KEY'


@app.route('/open', methods=['POST'])
def open_fsinf():
    content = request.json
    if content['key'] != key:
        print('err:nope')
        return 'err:nope', 400
    ser.write(b'O')
    return 'Opened', 200


@app.route('/close', methods=['POST'])
def close_fsinf():
    content = request.json
    if content['key'] != key:
        print('err:nope')
        return 'err:nope', 400
    ser.write(b'C')
    return 'Closed', 200

@app.route('/play', methods=['POST'])
def play_fsinf():
    content = request.json
    if content['key'] != key:
        print('err:nope')
        return 'err:nope', 400
    ser.write(b'1')
    return 'Play', 200

@app.route('/led', methods=['POST'])
def led_fsinf():
    content = request.json
    if content['key'] != key:
        print('err:nope')
        return 'err:nope', 400
    ser.write(b'2')
    return 'Led', 200

@app.route('/mattermost', methods=['POST'])
def mattermost_fsinf():
    content = request.form
    if content['token'] != key:
        print('err:nope')
        return 'err:nope', 400

    if 'open' == content['text']:
        ser.write(b'O')
    elif 'close' == content['text']:
        ser.write(b'C')
    elif 'play' == content['text']:
        ser.write(b'M')
    elif 'led' == content['text']:
        ser.write(b'L')
    elif '0' == content['text']:
        ser.write(b'0')
    elif '1' == content['text']:
        ser.write(b'1')
    elif '2' == content['text']:
        ser.write(b'2')
    elif '3' == content['text']:
        ser.write(b'3')
    elif '4' == content['text']:
        ser.write(b'4')
    else:
        print('err:unknown')
        return 'err:unknown', 400
    return '# mode set', 200


def send_request(msg_text, cmd_text):
    url = 'https://mm.fsinf.at/api/bot/is-it-open'
    jdata = {
        'msg_state': msg_text,
        'cmd_state': cmd_text
    }

    requests.post(url, json=jdata)

def read_serial():
    global open
    while(True):
        r = ser.read(1)
        if (r == b'C' or r == b'c') and open:
            send_request('## :stop_sign: FsInf is closing - Pfiad di!', '## Naaa, oida!')
            open = False
        if (r == b'O' or r == b'o') and not open:
            send_request('## :white_check_mark: FsInf opening - Kumm ume!', '## Fix oida!')
            open = True

if __name__ == '__main__':
    t = threading.Thread(target=read_serial)
    t.start()
    app.run(host='0.0.0.0')
