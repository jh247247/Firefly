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
import android.app.FragmentManager;
import android.os.AsyncTask;
import android.content.Context;

import org.json.JSONObject;

import butterknife.ButterKnife;
import butterknife.Bind;

public class MainActivity extends Activity {
    private static String TAG = "MainActivity";

    // move this to a retained fragment at some point.
    private NsdHelper m_nsdHelper;
    private ServiceCallback m_serviceCallback = new ServiceCallback();

    private RetainedFragment m_retained;

    private JSONObject m_nodesJson;

    @Bind(R.id.mainLayout) LinearLayout m_mainLayout;
    @Bind(R.id.mainText) TextView m_mainText; // FIXME: Placeholder,
    // remove later

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
            Log.d(TAG, "Service resolved: " + n);
            InetAddress ip = n.getHost();

            // FIXME: POF for http...

            int port = n.getPort();
            String url = "http:/" + ip + ":" + port + "/node";
            Log.d(TAG, "Fully resolved URL: " + url);

            ConnectivityManager connMgr = (ConnectivityManager)
                getSystemService(Context.CONNECTIVITY_SERVICE);
            new DownloadWebpageTask().execute(url);
        }

        @Override
        public void onServiceLost(NsdServiceInfo n) {
            Log.d(TAG, "Service lost: " + n);
        }
    }

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
            m_nodesJson = null;
            try {
                m_nodesJson = new JSONObject(result);
            }
            catch (Exception e) {
                Log.e(TAG,"Error on JSON parse: " + e);
            }
	    if(m_nodesJson != null) {
		m_mainText.setText(m_nodesJson.toString());
	    } else {
		m_mainText.setText("Invalid JSON returned: " + result);
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
