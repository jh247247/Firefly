package com.redo.rediscover.network;

import java.util.List;

public class Node {
    public String nodeId; // id of the given node
    public int timestamp; // timestamp of last update
    public List<String> fireflies; // list of associated firefly ids
}
