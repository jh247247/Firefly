package com.redo.rediscover.network;

import java.util.List;

import retrofit.Retrofit;
import retrofit.Call;
import retrofit.http.GET;
import retrofit.http.Path;

public interface RediscoverService {
    @GET("/node/{nodeId}")
    Call<Node> nodeDetails(@Path("nodeId") String nodeId);
    @GET("/node")
    Call<NodeList> nodeIds();
}
