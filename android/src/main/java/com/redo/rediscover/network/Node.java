package com.redo.rediscover.network;

public class Node {
    public String login;
    public int contributions;
    public Node(String login, int contributions) {
	this.login = login;
	this.contributions = contributions;
    }
}
