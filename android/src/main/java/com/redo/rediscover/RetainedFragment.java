package com.redo.rediscover;

import java.util.List;
import java.io.IOException;

import android.app.Fragment;
import android.app.Activity;
import android.os.Bundle;
import android.util.Log;
import android.net.nsd.NsdServiceInfo;

import com.redo.rediscover.network.RediscoverService;
import com.redo.rediscover.network.Node;

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

        Retrofit r = new Retrofit.Builder()
            .baseUrl("https://api.github.com")
	    .addConverterFactory(GsonConverterFactory.create())
            .build();
	RediscoverService m_rediscoverService =
            r.create(RediscoverService.class);
	Call<List<Node>> call =
            m_rediscoverService.contributions("square","retrofit");
	List<Node> nodes = null;
	try {
	    nodes = call.execute().body();
	}
	catch(Exception e) {
	    Log.e(TAG,"Error getting json: " + e);
	    return;
	}
	if(nodes == null) {
	    Log.e(TAG,"No contributions!");
	    return;
	}
	for(Node no : nodes) {
	    Log.d(TAG,"Test api: " + no.login + " (" +
            no.contributions + ")");
	}
    }

}
