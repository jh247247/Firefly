package com.redo.rediscover;

import java.net.InetAddress;
import java.io.IOException;
import java.net.URL;
import java.net.HttpURLConnection;
import java.io.InputStream;
import java.io.UnsupportedEncodingException;
import java.io.Reader;
import java.io.InputStreamReader;

import android.app.Activity;
import android.os.Bundle;
import android.net.nsd.NsdServiceInfo;
import android.net.ConnectivityManager;
import android.util.Log;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.app.Fragment;
import android.app.FragmentManager;
import android.app.FragmentTransaction;
import android.os.AsyncTask;
import android.content.Context;
import android.os.Handler;

import org.json.JSONObject;
import org.json.JSONArray;

import butterknife.ButterKnife;
import butterknife.Bind;

import de.greenrobot.event.EventBus;

import com.redo.rediscover.network.DiscoveryFragment;

public class MainActivity extends Activity {
    private static String TAG = "MainActivity";

    // move this to a retained fragment at some point.
    private NsdHelper m_nsdHelper;
    private ServiceCallback m_serviceCallback = new ServiceCallback();

    private RetainedFragment m_retained;

    private JSONObject m_nodesJson;
    private String m_monitorUrl;

    @Bind(R.id.mainLayout) LinearLayout m_mainLayout;
    @Bind(R.id.mainText) TextView m_mainText; // FIXME: Placeholder,
    // remove later

    // UI update handler
    Handler m_uiHandler = new Handler();
    private static int UI_UPDATE_TIMEOUT = 200;

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
        ButterKnife.bind(this);

        setupRetainedFragment();

        // setup network discovery.
        m_nsdHelper = new NsdHelper(this,m_serviceCallback);
        m_nsdHelper.startDiscovery();
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        ButterKnife.unbind(this);
    }

    class ServiceCallback implements NsdHelper.NsdHelperListener {
        @Override
        public void onServiceResolved(NsdServiceInfo n) {
	    m_nsdHelper.stopDiscovery(); // tell the helper to stop
	    // listening, we already found the service.
	    m_retained.setServiceInfo(n);
        }

        @Override
        public void onServiceLost(NsdServiceInfo n) {
            Log.d(TAG, "Service lost: " + n);
        }
    }

    private void setupRetainedFragment() {
        FragmentManager fm = getFragmentManager();

        // get back retained vars if required
        m_retained = (RetainedFragment)
            fm.findFragmentByTag(RetainedFragment.TAG);

        // first start, fragment does not exist!
        if(m_retained == null) {
            Log.w("MainActivity","Have to create retained fragment!");
            m_retained = new RetainedFragment();
            fm.beginTransaction().add(m_retained,
                                      RetainedFragment.TAG).commit();
        }

        // TODO: get retained instances of our variables...
        // TODO: send other retained instances along event bus.
    }
}
