package com.redo.rediscover;

import java.net.InetAddress;

import android.support.v4.app.Fragment;
import android.os.Bundle;
import android.util.Log;
import android.net.nsd.NsdServiceInfo;

import com.redo.rediscover.network.RediscoverService;

import retrofit.Retrofit;



/**
 * This class instance is retained on context changes, this means that
 * member variables in this are not lost on rotation etc.
 *
 * Hence, model (backend) instances should reside in this class.
 */
public class RetainedFragment extends Fragment {
    public static String TAG = "RetainedFragment";


    public RetainedFragment() {

    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setRetainInstance(true);
    }
}
