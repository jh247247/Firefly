package com.redo.rediscover.model;

import android.util.Log;
import java.net.InetAddress;

import com.redo.rediscover.network.RediscoverService;

import retrofit.Retrofit;
import retrofit.GsonConverterFactory;

/**
 * This file acts like an adapter between the monitor and the presenter, GETting and PUTting things to the API.
 * Follows the -singleton- pattern, in order to survive context changes.
 */

public class Monitor {
    private static String TAG = "Monitor";
    private static RediscoverService m_rediscoverService = null;

    // private constructor means we cannot make any instances of this class.
    private Monitor() {}

    public static void setLocation(InetAddress ip, int port) {
        String url = "http:/" + ip + ":" + port;
        Log.d(TAG, "Fully resolved URL: " + url);

        Retrofit r = new Retrofit.Builder()
            .baseUrl(url)
            .addConverterFactory(GsonConverterFactory.create())
            .build();
        m_rediscoverService =
            r.create(RediscoverService.class);
    }

    public static boolean serviceApiExists() {
        return (m_rediscoverService != null);
    }

    // be careful when using this, service may be null since it hasn't been set yet...
    public static RediscoverService getServiceApi() {
        return m_rediscoverService;
    }
}
