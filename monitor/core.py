#!/usr/bin/python3

# this file sets up the api

import node, firefly


def api_setup(api):
    node.setup(api)
    firefly.setup(api)

if __name__ == '__main__':
    print("WRITE TEST")
