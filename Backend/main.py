from flask import Flask
import serial
import threading

ser = serial.Serial('/dev/ttyUSB0', baudrate=115200)
app = Flask(__name__)

@app.route("/open", methods=['POST'])
def open_fsinf():
    ser.write(b'O')
    return "Opened"


@app.route("/close", methods=['POST'])
def close_fsinf():
    ser.write(b'C')
    return "Closed"


def read_serial():
    while(True):
        print(ser.read(1))

if __name__ == '__main__':
    t = threading.Thread(target=read_serial)
    t.start()
    app.run()
