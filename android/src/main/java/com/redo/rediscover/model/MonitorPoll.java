package com.redo.rediscover.model;

import android.os.Handler;
import android.util.Log;
import com.redo.rediscover.network.FireflyList;
import com.redo.rediscover.network.NodeList;
import com.redo.rediscover.network.RediscoverService;
import com.redo.rediscover.network.Node;
import com.redo.rediscover.network.Firefly;
import retrofit.Callback;
import retrofit.Response;
import retrofit.Retrofit;

import de.greenrobot.event.EventBus;
import java.lang.Math;
import java.util.List;

// TODO: Change monitor api so that node/firefly lists include data for a subset of the nodes/fireflies.
//       sending one big request is better than a bunch of small ones

/**
 * This class polls the monitor with a predefined rate, give and take a set amount.
 * Results are posted to the eventbus.
 * Another singleton.
 */
public class MonitorPoll {
    private static String TAG = "MonitorPoll";
    private static Handler m_handler = new Handler();

    // ### LIST CALLBACKS ##

    private static Callback<FireflyList> m_fireflyListCallback = new Callback<FireflyList>() {
            @Override
            public void onResponse(Response<FireflyList> resp, Retrofit r) {
                List<String> flist;
                try {
                    flist = resp.body().ids;
                    // post list to eventbus so the ui can be updated
                    EventBus.getDefault().post(new FireflyListLoadedEvent(resp.body()));
                }
                catch(Exception e) {
                    Log.e(TAG, "Failed parsing firefly list: " + e);
                    return;
                }



                for(String id : flist) {
                    Monitor.getServiceApi().fireflyDetails(id).enqueue(m_fireflyIdCallback);
                }
            }

            @Override
            public void onFailure(Throwable t) {
                Log.e(TAG, "Failed retreiving firefly list: " + t);
            }
        };

    private static Callback<NodeList> m_nodeListCallback = new Callback<NodeList>() {
            @Override
            public void onResponse(Response<NodeList> resp, Retrofit r) {
                List<String> nlist;
                try {
                    nlist = resp.body().ids;

                    // post list to eventbus so the ui can be updated
                    EventBus.getDefault().post(new NodeListLoadedEvent(resp.body()));
                }
                catch(Exception e) {
                    Log.e(TAG, "Failed parsing node list: " + e);
                    return;
                }



                for(String id : nlist) {
                    Monitor.getServiceApi().nodeDetails(id).enqueue(m_nodeIdCallback);
                }

            }

            @Override
            public void onFailure(Throwable t) {
                Log.e(TAG, "Failed retreiving node list: " + t);
            }
        };

    // ### INDIVIDUAL ID CALLBACKS ###

    private static Callback<Node> m_nodeIdCallback = new Callback<Node>() {
            @Override
            public void onResponse(Response<Node> resp, Retrofit r) {
                try {
                    Log.d(TAG, "Loaded data of node  " + resp.body().nodeId);
                    EventBus.getDefault().post(new NodeDataLoadedEvent(resp.body()));
                }
                catch (Exception e) {
                    Log.e(TAG, "Failed parsing node data: " + e);
                }
            }

            @Override
            public void onFailure(Throwable t) {
                Log.e(TAG, "Failed retreiving node details: " + t);
            }
        };

    private static Callback<Firefly> m_fireflyIdCallback = new Callback<Firefly>() {
            @Override
            public void onResponse(Response<Firefly> resp, Retrofit r) {
                try {
                    Log.d(TAG, "Loaded data of firefly  " + resp.body().nodeId);
                    EventBus.getDefault().post(new FireflyDataLoadedEvent(resp.body()));
                }
                catch (Exception e) {
                    Log.e(TAG, "Failed parsing node data: " + e);
                }
            }

            @Override
            public void onFailure(Throwable t) {
                Log.e(TAG, "Failed retreiving firefly details: " + t);
            }
        };


    // times are in ms
    // interval to poll monitor for data
    private static final int DEFAULT_UPDATE_INTERVAL = 2500;

    // amount of randomization to add
    private static final int DEFAULT_UPDATE_INTERVAL_SPAN = 1000;

    private static int m_updateInterval = DEFAULT_UPDATE_INTERVAL;
    private static int m_updateIntervalSpan = DEFAULT_UPDATE_INTERVAL_SPAN;

    private MonitorPoll(){}

    public static int getUpdateInterval() {
        return m_updateInterval;
    }

    public static int getUpdateIntervalSpan() {
        return m_updateIntervalSpan;
    }

    public static void setUpdateInterval(int ui) {
        m_updateInterval = ui;
    }

    public static void setUpdateIntervalSpan(int uis) {
        m_updateInterval = uis;
    }

    public static void start() {
        // handler does 2 streams, one for firefly, one for node.
        // do we actually need nodes later on?
        m_handler.postDelayed(new Runnable(){
                @Override
                public void run() {
                    RediscoverService service = Monitor.getServiceApi();
                    service.fireflyIds().enqueue(m_fireflyListCallback);
                    m_handler.postDelayed(this,
                                          (long)(m_updateInterval +
                                                 (Math.random()*m_updateIntervalSpan*2)-m_updateIntervalSpan));
                }
            },m_updateInterval);

        m_handler.postDelayed(new Runnable(){
                @Override
                public void run() {
                    RediscoverService service = Monitor.getServiceApi();
                    service.nodeIds().enqueue(m_nodeListCallback);
                    m_handler.postDelayed(this,
                                          (long)(m_updateInterval +
                                                 (Math.random()*m_updateIntervalSpan*2)-m_updateIntervalSpan));
                }
            },m_updateInterval);
    }

    public static void stop() {
        m_handler.removeCallbacksAndMessages(null);
    }

    public static class NodeListLoadedEvent {
        public NodeList list;
        public NodeListLoadedEvent(NodeList n) {
            list = n;
        }
    }

    public static class FireflyListLoadedEvent {
        public FireflyList list;
        public FireflyListLoadedEvent(FireflyList n) {
            list = n;
        }
    }


    public static class FireflyDataLoadedEvent {
        public String id;
        public Firefly data;
        public FireflyDataLoadedEvent(Firefly f) {
            id = f.fireflyId;
            data = f;
        }
    }

    public static class NodeDataLoadedEvent {
        public String id;
        public Node data;
        public NodeDataLoadedEvent(Node n) {
            id = n.nodeId;
            data = n;
        }
    }

}
