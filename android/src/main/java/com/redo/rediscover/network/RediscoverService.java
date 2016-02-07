package com.redo.rediscover.network;

import retrofit.Call;
import retrofit.http.GET;
import retrofit.http.Path;

public interface RediscoverService {
    @GET("/node/{nodeId}")
    Call<Node> nodeDetails(@Path("nodeId") String nodeId);
    @GET("/node")
    Call<NodeList> nodeIds();
    @GET("/firefly/{fireflyId}")
    Call<Firefly> fireflyDetails(@Path("fireflyId") String fireflyId);
    @GET("/firefly")
    Call<FireflyList> fireflyIds();
}
