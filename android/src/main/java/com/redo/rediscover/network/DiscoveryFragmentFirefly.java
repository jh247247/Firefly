package com.redo.rediscover.network;

import android.app.Fragment;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import com.redo.rediscover.R;

import butterknife.Bind;
import butterknife.ButterKnife;
import de.greenrobot.event.EventBus;

/**
 * This class handles the UI for network discovery, allowing the user
 * to select what service they want to use for the duration of the
 * current session (or subsequent sessions if it is saved)
 */

public class DiscoveryFragmentFirefly extends Fragment {
    public static String FIREFLY_ID = "nodeId";
    private static String TAG = "DiscoveryFragmentFirefly";

    @Bind(R.id.firefly_id) TextView m_fireflyId;
    @Bind(R.id.firefly_data) TextView m_fireflyDataView;

    private String m_fireflyIdStr;

    // TODO: monitor url update?
    @Override
    public View onCreateView(LayoutInflater inflater,
                             ViewGroup container,
                             Bundle savedInstanceState) {

        View ret =  inflater.inflate(R.layout.discovery_firefly,
                                     container, false);
        ButterKnife.bind(this,ret);

        // parse passed in vars
        Bundle args = getArguments();

        if(args != null) {
            m_fireflyIdStr = args.getString(FIREFLY_ID);
            m_fireflyId.setText("Firefly ID: " + m_fireflyIdStr);
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

    public static class RequestFireflyIdUpdateEvent {
        public RequestFireflyIdUpdateEvent(String str) {
            id = str;
        }
        public String id;
    }

    public void requestUpdate() {
        EventBus.getDefault().post(new RequestFireflyIdUpdateEvent(m_fireflyIdStr));
    }

    public void updateFirefly(Firefly n) {
        m_fireflyDataView.setText("Val: " + n.val);
    }
}
