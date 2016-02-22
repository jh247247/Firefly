package com.redo.rediscover;

import android.app.Activity;
import android.os.Bundle;
import android.net.nsd.NsdServiceInfo;
import android.util.Log;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.app.FragmentManager;
import android.app.FragmentTransaction;
import android.os.Handler;
import android.app.Fragment;

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
        if(service == null) return; // TODO: log

	Call<NodeList> call = service.nodeIds();
        call.enqueue(new Callback<NodeList>() {
                @Override
                public void onResponse(Response<NodeList> resp, Retrofit retro) {
                    List<String> nodeList = resp.body().ids;

                    FragmentManager fm = getFragmentManager();
                    FragmentTransaction ft = fm.beginTransaction();

                    // Make a fragment that monitors that node id
                    for(String id : nodeList) {
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

        Call<FireflyList> callFirefly = service.fireflyIds();
        callFirefly.enqueue(new Callback<FireflyList>() {
            @Override
            public void onResponse(Response<FireflyList> resp, Retrofit retro) {
                List<String> fireflyList = resp.body().ids;

                FragmentManager fm = getFragmentManager();
                FragmentTransaction ft = fm.beginTransaction();

                // Make a fragment that monitors that node id
                for(String id : fireflyList) {
                    Bundle b = new Bundle();
                    b.putString(DiscoveryFragment.NODE_ID, id);

                    // make sure we don't already have the fragment inflated
                    DiscoveryFragmentFirefly df = (DiscoveryFragmentFirefly)fm.findFragmentByTag(id);
                    if(df != null) {
                        // fragment already exists, try to update contents
                        df.requestUpdate();
                    } else {
                        // make a new fragment...
                        DiscoveryFragmentFirefly f = new DiscoveryFragmentFirefly();
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
                    Log.e(TAG, "Error updating node: " + e.id + " :" + t.toString());
                }
            });
    }

    public void onEvent(final DiscoveryFragmentFirefly.RequestFireflyIdUpdateEvent e) {
        RediscoverService s = m_retained.getServiceApi();
        Call<Firefly> call = s.fireflyDetails(e.id);
        call.enqueue(new Callback<Firefly>() {
                @Override
                public void onResponse(Response<Firefly> resp,
                                       Retrofit retro) {
                    Firefly f = resp.body();
                    FragmentManager fm = getFragmentManager();
                    DiscoveryFragmentFirefly df = (DiscoveryFragmentFirefly)fm.findFragmentByTag(f.fireflyId);
                    if(df != null) {
                        df.updateFirefly(f);
                    }
                }

                @Override
                public void onFailure(Throwable t) {
                    Log.e(TAG,"Error updating Firefly: " + e.id + " :" + t.toString());
                }
            });
    }
}
