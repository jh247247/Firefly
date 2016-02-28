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


public class FireflyListPresenter extends Fragment {
    public static String TAG = "FireflyListPresenter";

    private static FireflyListAdapter m_adapter = new FireflyListAdapter();

    @Bind(R.id.presenter_main_list) RecyclerView m_mainList;

    @Override
    public View onCreateView(LayoutInflater inflater,
                             ViewGroup container,
                             Bundle savedInstanceState) {
    	View ret = inflater.inflate(R.layout.firefly_list_presenter, container, false);

	ButterKnife.bind(this,ret);

	//parseArgs();
	setupUi();

	return ret;
    }

    private void setupUi() {
	// assumes m_mainList is already bound.
	// TODO: check if activity leaks...
	m_mainList.setLayoutManager(new LinearLayoutManager(getActivity()));
	m_mainList.setAdapter(m_adapter);
	m_mainList.setItemAnimator(new DefaultItemAnimator());
    }

}
