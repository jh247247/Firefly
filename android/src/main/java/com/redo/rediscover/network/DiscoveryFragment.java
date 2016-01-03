package com.redo.rediscover.network;

import com.redo.rediscover.R;

import java.net.InetAddress;
import java.io.IOException;
import java.net.URL;
import java.net.HttpURLConnection;
import java.io.InputStream;
import java.io.UnsupportedEncodingException;
import java.io.Reader;
import java.io.InputStreamReader;

import android.os.Bundle;
import android.app.Fragment;
import android.view.LayoutInflater;
import android.view.ViewGroup;
import android.view.View;
import android.widget.TextView;
import android.os.AsyncTask;
import android.util.Log;

import butterknife.ButterKnife;
import butterknife.Bind;

import de.greenrobot.event.EventBus;

import org.json.JSONObject;
import org.json.JSONArray;
import com.redo.rediscover.network.Node;

/**
 * This class handles the UI for network discovery, allowing the user
 * to select what service they want to use for the duration of the
 * current session (or subsequent sessions if it is saved)
 */

public class DiscoveryFragment extends Fragment {
    public static String NODE_ID = "nodeId";
    private static String TAG = "DiscoveryFragment";

    @Bind(R.id.node_id) TextView m_nodeId;
    @Bind(R.id.node_data) TextView m_nodeDataView;

    private String m_nodeIdStr;

    // TODO: monitor url update?
    @Override
    public View onCreateView(LayoutInflater inflater,
                             ViewGroup container,
                             Bundle savedInstanceState) {

        View ret =  inflater.inflate(R.layout.discovery,
                                     container, false);
        ButterKnife.bind(this,ret);

        // parse passed in vars
        Bundle args = getArguments();

        if(args != null) {
            m_nodeIdStr = args.getString(NODE_ID);
            m_nodeId.setText("Node ID: " + m_nodeIdStr);
        }

	requestUpdate();

        return ret;
    }

    // event bus stuff!
    public void onEvent(MonitorUpdateEvent e) {

    }

    // update the monitor, to be sent via the eventbus
    public static class MonitorUpdateEvent {
	public MonitorUpdateEvent(String m) {
	    monitorUrl = m;
	}
        public String monitorUrl;
    }

    public static class RequestNodeIdUpdateEvent {
	public RequestNodeIdUpdateEvent(String str) {
	    id = str;
	}
	public String id;
    }

    public void requestUpdate() {
	EventBus.getDefault().post(new RequestNodeIdUpdateEvent(m_nodeIdStr));
    }

    public void updateNode(Node n) {
	m_nodeDataView.setText("Val: " + n.val);
    }
}
