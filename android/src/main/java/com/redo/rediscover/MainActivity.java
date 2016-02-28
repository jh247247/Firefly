package com.redo.rediscover;

import android.support.design.widget.TabLayout;
import android.support.v4.view.ViewPager;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.net.nsd.NsdServiceInfo;
import android.support.v7.widget.Toolbar;
import android.util.Log;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.support.v4.app.FragmentManager;
import android.support.v4.app.FragmentTransaction;
import android.os.Handler;
import android.support.v4.app.Fragment;
import com.redo.rediscover.ui.FireflyListPresenter;

import java.util.List;

import org.json.JSONObject;

import butterknife.ButterKnife;
import butterknife.Bind;

import de.greenrobot.event.EventBus;

import retrofit.Call;
import retrofit.Callback;
import retrofit.Response;
import retrofit.Retrofit;

import com.redo.rediscover.network.DiscoveryFragment;
import com.redo.rediscover.network.DiscoveryFragmentFirefly;
import com.redo.rediscover.network.RediscoverService;
import com.redo.rediscover.network.Node;
import com.redo.rediscover.network.NodeList;
import com.redo.rediscover.network.Firefly;
import com.redo.rediscover.network.FireflyList;
import com.redo.rediscover.network.IdList;
import com.redo.rediscover.model.Monitor;
import com.redo.rediscover.model.MonitorPoll;

public class MainActivity extends AppCompatActivity {
    private static String TAG = "MainActivity";

    // move this to a retained fragment at some point.
    private NsdHelper m_nsdHelper;
    private ServiceCallback m_serviceCallback = new ServiceCallback();

    private RetainedFragment m_retained;

    private JSONObject m_nodesJson;
    private String m_monitorUrl;

    // use the layout to add things around the viewpager (may need work)
    @Bind(R.id.main_layout) LinearLayout m_mainLayout;
    @Bind(R.id.main_pager) ViewPager m_mainPager;

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
        ButterKnife.bind(this);
        setupRetainedFragment();
        if(!Monitor.serviceApiExists()){
            // setup network discovery.
            m_nsdHelper = new NsdHelper(this,m_serviceCallback);
            m_nsdHelper.startDiscovery();
        }

	setupUi();
    }

    public void setupUi() {
	FragmentTransaction ft = getSupportFragmentManager().beginTransaction();
	Fragment fl = (Fragment)(new FireflyListPresenter());
	ft.add(R.id.main_layout,fl);
	ft.commit();

    }

    @Override
    public void onPause() {
        super.onPause();
	MonitorPoll.stop();
    }


    @Override
    public void onResume() {
        super.onResume();
	MonitorPoll.start();
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        ButterKnife.unbind(this);
        EventBus.getDefault().unregister(this);
    }

    class ServiceCallback implements NsdHelper.NsdHelperListener {
        @Override
        public void onServiceResolved(NsdServiceInfo n) {
            m_nsdHelper.stopDiscovery(); // tell the helper to stop
            // listening, we already found the service.
	    Monitor.setLocation(n.getHost(),n.getPort());
        }

        @Override
        public void onServiceLost(NsdServiceInfo n) {
            Log.d(TAG, "Service lost: " + n);
        }
    }

    private void setupRetainedFragment() {
        FragmentManager fm = getSupportFragmentManager();

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
