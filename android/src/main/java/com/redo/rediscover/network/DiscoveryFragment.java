package com.redo.network.DiscoveryFragment;

import com.redo.rediscover.R;

import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.view.LayoutInflater;
import android.view.ViewGroup;
import android.view.View;
import android.widget.ScrollView;

import butterknife.ButterKnife;
import butterknife.Bind;

/**
 * This class handles the UI for network discovery, allowing the user
 * to select what service they want to use for the duration of the
 * current session (or subsequent sessions if it is saved)
 */

public class DiscoveryFragment extends Fragment {
    @Bind(R.id.service_scroll_view) ScrollView m_serviceScrollView;

    @Override
    public View onCreateView(LayoutInflater inflater,
			     ViewGroup container,
			     Bundle onSavedInstaceState) {
	View ret =  inflater.inflate(R.layout.discovery,
				container, false);
	ButterKnife.bind(this,ret);

	// restore views here...

	return ret;
    }
}
