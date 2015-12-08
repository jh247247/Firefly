package com.redo.rediscover;

import android.app.Activity;
import android.os.Bundle;
import android.net.nsd.NsdServiceInfo;
import android.util.Log;
import android.widget.LinearLayout;

import butterknife.ButterKnife;
import butterknife.Bind;

public class MainActivity extends Activity {
    private static String TAG = "MainActivity";

    // move this to a retained fragment at some point.
    private NsdHelper m_nsdHelper;
    private ServiceCallback m_serviceCallback = new ServiceCallback();

    @Bind(R.id.mainLayout) LinearLayout m_mainLayout;

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
	ButterKnife.bind(this);

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
	    Log.d(TAG, "Service resolved: " + n);
	}

	@Override
	public void onServiceLost(NsdServiceInfo n) {
	    Log.d(TAG, "Service lost: " + n);
	}
    }
}
