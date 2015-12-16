package com.redo.rediscover;

import android.app.Fragment;
import android.app.Activity;
import android.os.Bundle;
import android.util.Log;

/**
 * This class instance is retained on context changes, this means that
 * member variables in this are not lost on rotation etc.
 *
 * Hence, model (backend) instances should reside in this class.
 */
public class RetainedFragment extends Fragment {
    public static String TAG = "RetainedFragment";

    public RetainedFragment() {

    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
	super.onCreate(savedInstanceState);
	setRetainInstance(true);
    }

    // TODO: getters for saved vars.
}
