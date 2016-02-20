package com.redo.rediscover.network;

public class Firefly {
    public String fireflyId; // id of the current firefly
    public String nodeId; // id of the node it is associated with
    public int bat; // current battery life of the firefly
    public int temp; // current temperature of the firefly
    public int timer; // current period of transmission (given in 250ms intervals)
    public int timestamp; // timestamp of last update
    public String status; // current status of the firefly, see README.md in firefly dir
}
