# app.py
from flask import Flask, render_template
from flask_socketio import SocketIO
from watchdog.observers import Observer
from watchdog.events import FileSystemEventHandler
import markdown
import os
import threading
import sys

app = Flask(__name__)
app.config['SECRET_KEY'] = 'secret!'
socketio = SocketIO(app)

# Обратите внимание на использование os.path.join и raw строки для путей
md_files = [
    r"C:\Projects\suai\totally-not-rabbitmq\cmake-build-release-mingw-ucrt64\src\tasks.md",
    r"C:\Projects\suai\totally-not-rabbitmq\cmake-build-release-mingw-ucrt64\src\users.md"
]
data1 = ""
data2 = ""

class MDFileHandler(FileSystemEventHandler):
    def on_modified(self, event):
        try:
            # Проверяем, что путь совпадает с любым из наших файлов
            if event.src_path.replace("\\", "/") in [f.replace("\\", "/") for f in md_files]:
                print(f"File modified: {event.src_path}")
                update_data()
                socketio.emit('update', {'data1': data1, 'data2': data2})
        except Exception as e:
            print(f"Error in on_modified: {e}", file=sys.stderr)

def update_data():
    global data1, data2
    combined_content1 = ""
    combined_content2 = ""
    for idx, file in enumerate(md_files):
        try:
            with open(file, 'r', encoding='utf-8') as f:
                if idx == 0:
                    combined_content1 += f.read() + "\n\n"
                elif idx == 1:
                    combined_content2 += f.read() + "\n\n"
        except Exception as e:
            print(f"Error reading {file}: {e}", file=sys.stderr)
    data1 = markdown.markdown(combined_content1, extensions=['tables'])
    data2 = markdown.markdown(combined_content2, extensions=['tables'])
    print("Data updated")

@app.route('/')
def index():
    return render_template('index.html', table1=data1, table2=data2)

if __name__ == "__main__":
    try:
        update_data()  # Initial load
        event_handler = MDFileHandler()
        observer = Observer()
        for file in md_files:
            observer.schedule(event_handler, path=os.path.dirname(file), recursive=False)
        observer_thread = threading.Thread(target=observer.start)
        observer_thread.daemon = True
        observer_thread.start()

        socketio.run(app, debug=True, use_reloader=False)
    except Exception as e:
        print(f"Error in main: {e}", file=sys.stderr)
