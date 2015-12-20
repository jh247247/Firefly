package com.redo.rediscover;

import java.net.InetAddress;
import java.util.List;
import java.io.IOException;

import android.app.Fragment;
import android.app.Activity;
import android.os.Bundle;
import android.util.Log;
import android.net.nsd.NsdServiceInfo;

import com.redo.rediscover.network.RediscoverService;
import com.redo.rediscover.network.Node;
import com.redo.rediscover.network.NodeList;

import retrofit.Retrofit;
import retrofit.GsonConverterFactory;
import retrofit.Call;



/**
 * This class instance is retained on context changes, this means that
 * member variables in this are not lost on rotation etc.
 *
 * Hence, model (backend) instances should reside in this class.
 */
public class RetainedFragment extends Fragment {
    public static String TAG = "RetainedFragment";

    // when a service is resolved, set it here.
    private NsdServiceInfo m_serviceInfo;
    private RediscoverService m_rediscoverService;

    public RetainedFragment() {

    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setRetainInstance(true);
    }

    public void setServiceInfo(NsdServiceInfo n) {
        m_serviceInfo = n;

        InetAddress ip = n.getHost();
        int port = n.getPort();
        String url = "http:/" + ip + ":" + port;
        Log.d(TAG, "Fully resolved URL: " + url);

        Retrofit r = new Retrofit.Builder()
            .baseUrl(url)
            .addConverterFactory(GsonConverterFactory.create())
            .build();
        m_rediscoverService =
            r.create(RediscoverService.class);
    }

    public RediscoverService getServiceApi() {
	return m_rediscoverService;
    }
}
