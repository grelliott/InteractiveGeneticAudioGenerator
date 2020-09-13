import json
import psutil
import time
from flask import Flask
from flask import render_template
from flask import request
from flask_sockets import Sockets
from geventwebsocket.exceptions import WebSocketError

app = Flask(__name__)
app.debug = True
sockets = Sockets(app)

def are_processes_running(names):
    """ Look up each process name in procs and return dict with status for each one """
    r = { name:"Not Running" for name in names }
    for proc in psutil.process_iter():
        try:
            if proc.name() in names:
                r[proc.name()] = "Running"
        except (psutil.NoSuchProcess, psutil.AccessDenied, psutil.ZombieProcess):
            pass
    return r

def get_system_status():
    """ Check status of key services """
    return are_processes_running(["audiogene", "scsynth"])

def get_cpu_usage():
    return psutil.cpu_percent()

def get_cpu_temp():
    return psutil.sensors_temperatures()["cpu-thermal"][0].current

def get_mem_usage():
    return psutil.virtual_memory().percent

def get_status():
    return {
        "connectionStatus": "Connected",
        "systemStatus": get_system_status(),
        "cpuUsage" : get_cpu_usage(),
        "memUsage" : get_mem_usage(),
        "cpuTemp" : get_cpu_temp()
    }


def authenticate(args):
    """ Check args for certain criteria to determine if client is permitted to connect """
    return True

# Add something to subscribe


# send status
@sockets.route('/status')
def serve_status(ws):
    print("Serving status")
    while not ws.closed:
        try:
            ws.send(json.dumps(get_status()))
            time.sleep(1)
        except WebSocketError:
            print("Socket closed")
            return

@app.route('/')
def index():
    return render_template('index.html')


if __name__ == '__main__':
    from gevent import pywsgi
    from geventwebsocket.handler import WebSocketHandler
    #server = pywsgi.WSGIServer(('192.168.1.29', 5000), app, handler_class=WebSocketHandler, keyfile="server.key", certfile="server.crt")
    server = pywsgi.WSGIServer(('192.168.1.29', 5000), app, handler_class=WebSocketHandler)
    server.serve_forever()

