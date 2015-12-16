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

/**
 * This class handles the UI for network discovery, allowing the user
 * to select what service they want to use for the duration of the
 * current session (or subsequent sessions if it is saved)
 */

public class DiscoveryFragment extends Fragment {
    public static String NODE_ID = "nodeId";
    public static String MONITOR_URL = "monUrl";
    private static String TAG = "DiscoveryFragment";

    @Bind(R.id.node_id) TextView m_nodeId;
    @Bind(R.id.node_data) TextView m_nodeDataView;

    private String m_monitorUrlStr;
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
            m_monitorUrlStr = args.getString(MONITOR_URL);
            m_nodeIdStr = args.getString(NODE_ID);
            m_nodeId.setText(m_nodeIdStr + " @ " + m_monitorUrlStr);
        }

        updateNodeData();

        // TODO: actually make api...

        EventBus.getDefault().register(this);
        return ret;
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        EventBus.getDefault().unregister(this);
    }

    private void updateNodeData() {
        // TODO: load node data from api
        m_nodeDataView.setText("Loading...");
	new DownloadWebpageTask().execute(m_monitorUrlStr+"/node/"+m_nodeIdStr);
    }

    // event bus stuff!
    public void onEvent(MonitorUpdateEvent e) {
        updateNodeData();
    }

    // update the monitor, to be sent via the eventbus
    public static class MonitorUpdateEvent {
        public String monitorUrl;
    }


    // POF code here.
    // FIXME: Have this as a nice api or something later on.
    private class DownloadWebpageTask extends AsyncTask<String, Void, String> {
        @Override
        protected String doInBackground(String... urls) {

            // params comes from the execute() call: params[0] is the url.
            try {
                return downloadUrl(urls[0]);
            } catch (IOException e) {
                return "Unable to retrieve web page. URL may be invalid.";
            }
        }

        // onPostExecute displays the results of the AsyncTask.
        @Override
        protected void onPostExecute(String result) {
            JSONObject j = null;
            try {
                j = new JSONObject(result);
            }
            catch (Exception e) {
                Log.e(TAG,"Error on JSON parse: " + e);
            }
            // debug for now...
            if(j != null) {
                m_nodeDataView.setText(j.toString());
            } else {
                m_nodeDataView.setText("Invalid JSON returned: " + result);
            }
        }

        // Given a URL, establishes an HttpUrlConnection and retrieves
        // the web page content as a InputStream, which it returns as
        // a string.
        private String downloadUrl(String myurl) throws IOException {
            InputStream is = null;
            // Only display the first 500 characters of the retrieved
            // web page content.
            int len = 500;

            try {
                URL url = new URL(myurl);
                HttpURLConnection conn = (HttpURLConnection) url.openConnection();
                conn.setReadTimeout(10000 /* milliseconds */);
                conn.setConnectTimeout(15000 /* milliseconds */);
                conn.setRequestMethod("GET");
                conn.setDoInput(true);
                // Starts the query
                conn.connect();
                int response = conn.getResponseCode();
                Log.d(TAG, "The response is: " + response);
                is = conn.getInputStream();

                // Convert the InputStream into a string
                String contentAsString = readIt(is, len);
                return contentAsString;

                // Makes sure that the InputStream is closed after the app is
                // finished using it.
            } finally {
                if (is != null) {
                    is.close();
                }
            }
        }

        // Reads an InputStream and converts it to a String.
        public String readIt(InputStream stream, int len) throws IOException, UnsupportedEncodingException {
            Reader reader = null;
            reader = new InputStreamReader(stream, "UTF-8");
            char[] buffer = new char[len];
            reader.read(buffer);
            return new String(buffer);
        }
    }
}
