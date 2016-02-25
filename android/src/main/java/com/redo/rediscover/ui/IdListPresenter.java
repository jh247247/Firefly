package com.redo.rediscover.ui;

import android.support.v4.app.Fragment;
import android.os.Bundle;
import android.support.v7.widget.DefaultItemAnimator;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import butterknife.Bind;
import butterknife.ButterKnife;

import com.redo.rediscover.R;
import java.util.LinkedHashMap;

import retrofit.Call;
import retrofit.Callback;
import retrofit.Response;
import retrofit.Retrofit;

/**
 * This class presents a list and retreives the id at a given url.
 * This list is kept up to date at a given interval (passed in the bundle)
 * and order is retained, even if new ids are added or removed.
 *
 * Every (interval) milliseconds, this fragment updates the ui by loading the given url.
 * Assuming that it receives valid JSON, it then uses the "ids" field to populate the id list.
 * This list is then iterated over and
 */

public class IdListPresenter extends Fragment {
    public static String TAG = "IdListPresenter";
    public static String NAME = "NAME";
    public static String DEFAULT_NAME = "ERROR";
    private static int DEFAULT_UPDATE_INTERVAL = 5000;

    private IdListAdapter m_adapter;

    @Bind(R.id.presenter_main_list) RecyclerView m_mainList;

    @Override
    public View onCreateView(LayoutInflater inflater,
                             ViewGroup container,
                             Bundle savedInstanceState) {
    	View ret = inflater.inflate(R.layout.presenter, container, false);

	ButterKnife.bind(this,ret);

	//parseArgs();
	setupUi();

	return ret;
    }

    // private void parseArgs() {
    // 	// get args
    // 	Bundle args = getArguments();
    // 	String name;
    // 	if(args != null) {
    // 	    name = args.getString(NAME);
    // 	} else {
    // 	    name = DEFAULT_NAME;
    // 	}
    // 	m_adapter = new IdListAdapter(name);
    // }

    private void setupUi() {
	// assumes m_mainList is already bound.
	// TODO: check if activity leaks...
	m_mainList.setLayoutManager(new LinearLayoutManager(getActivity()));
	m_mainList.setAdapter(m_adapter);
	m_mainList.setItemAnimator(new DefaultItemAnimator());
    }

}
