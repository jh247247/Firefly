#!/usr/bin/python3
from flask import Flask as f

app = f(__name__)

@app.route('/')
def index():
    return "Hello, world!"

if __name__ == '__main__':
    app.run(debug=True,
            host='0.0.0.0')
