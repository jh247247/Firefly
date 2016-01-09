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
import retrofit.Call;
import com.redo.rediscover.network.NodeList;
import com.redo.rediscover.network.RediscoverService;
import retrofit.Callback;
import retrofit.Response;
import retrofit.Retrofit;
import java.util.List;
import com.redo.rediscover.network.Node;

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
    private static int UI_UPDATE_TIMEOUT = 5000;

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        EventBus.getDefault().register(this);
        setContentView(R.layout.main);
        ButterKnife.bind(this);
        setupRetainedFragment();
        if(m_retained.getServiceApi() == null){
            // setup network discovery.
            m_nsdHelper = new NsdHelper(this,m_serviceCallback);
            m_nsdHelper.startDiscovery();
        }

    }

    @Override
    public void onPause() {
	super.onPause();
        m_uiHandler.removeCallbacksAndMessages(null);
    }


    @Override
    public void onResume() {
	super.onResume();
        // update the list every UI_UPDATE_TIMEOUT ms, ugly pls fix
        m_uiHandler.postDelayed(new Runnable() {
                @Override
                public void run() {
                    setupNodeAttrList();
                    m_uiHandler.postDelayed(this,UI_UPDATE_TIMEOUT);
                }
            },UI_UPDATE_TIMEOUT);
	setupNodeAttrList();
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
            m_retained.setServiceInfo(n);
            setupNodeAttrList();
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

    private void setupNodeAttrList() {
        // get node attrs from retained fragment
        RediscoverService service = m_retained.getServiceApi();
        if(service == null) return;

        Call<NodeList> call = service.nodeIds();
        call.enqueue(new Callback<NodeList>() {
                @Override
                public void onResponse(Response<NodeList> resp, Retrofit retro) {
                    List<String> nl = resp.body().nodeIds;

                    // set the main text to show all known node IDS
                    m_mainText.setText(nl.toString());

                    FragmentManager fm = getFragmentManager();
                    FragmentTransaction ft = fm.beginTransaction();

                    // Make a fragment that monitors that node id
                    for(String id : nl) {
                        Bundle b = new Bundle();
                        b.putString(DiscoveryFragment.NODE_ID, id);

                        // make sure we don't already have the fragment inflated
                        DiscoveryFragment df = (DiscoveryFragment)fm.findFragmentByTag(id);
                        if(df != null) {
                            // fragment already exists, try to update contents
                            df.requestUpdate();
                        } else {
                            // make a new fragment...
                            DiscoveryFragment f = new DiscoveryFragment();
                            f.setArguments(b);
                            ft.add(R.id.mainLayout, f, id);
                        }

                        // TODO: handle if node dissapears...
                    }
                    ft.commit();
                }

                @Override
                public void onFailure(Throwable t) {
                    m_mainText.setText("Failure!");
                }
            });
    }

    // update a node that sends it's id along the eventbus with the proper encapsulation.
    public void onEvent(final DiscoveryFragment.RequestNodeIdUpdateEvent e) {
        RediscoverService s = m_retained.getServiceApi();
        Call<Node> call = s.nodeDetails(e.id);
        call.enqueue(new Callback<Node>() {
                @Override
                public void onResponse(Response<Node> resp,
                                       Retrofit retro) {
                    Node n = resp.body();
                    FragmentManager fm = getFragmentManager();
                    DiscoveryFragment df = (DiscoveryFragment)fm.findFragmentByTag(n.nodeId);
                    if(df != null) {
                        df.updateNode(n);
                    }
                }

                @Override
                public void onFailure(Throwable t) {
                    Log.e(TAG,"Error updating node: " + e.id +" :"+t.toString());
                }
            });
    }
}
