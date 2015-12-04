#!/usr/bin/python3

# this file sets up the api

import node

def api_setup(api):
    api.add_resource(node.node_res,'/')

    print('API SETUP DONE')

if __name__ == '__main__':
    print("WRITE TEST")
