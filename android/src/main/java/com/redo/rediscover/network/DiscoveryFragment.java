package com.redo.rediscover.network;

import com.redo.rediscover.R;

import android.os.Bundle;
import android.app.Fragment;
import android.view.LayoutInflater;
import android.view.ViewGroup;
import android.view.View;
import android.widget.TextView;

import butterknife.ButterKnife;
import butterknife.Bind;

/**
 * This class handles the UI for network discovery, allowing the user
 * to select what service they want to use for the duration of the
 * current session (or subsequent sessions if it is saved)
 */

public class DiscoveryFragment extends Fragment {
    public static String NODE_ID = "nodeId";

    @Bind(R.id.node_data) TextView m_nodeData;

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
	    m_nodeData.setText(args.getString(NODE_ID));
	}


	// TODO: load node data from api
	// TODO: actually make api...

	// restore views here...

	return ret;
    }
}
