package com.redo.rediscover.ui;

import android.os.Handler;
import android.support.v7.widget.RecyclerView;
import android.view.View;
import android.view.ViewGroup;
import com.redo.rediscover.R;
import java.util.HashMap;
import java.util.ArrayList;

import retrofit.Call;
import retrofit.Callback;
import retrofit.Response;
import retrofit.Retrofit;

/**
 * Recyclerview requires an adapter to create views when requested, these have to be wrapped in a ViewHolder.
 */

public class FireflyListAdapter extends RecyclerView.Adapter<FireflyListAdapter.ViewHolder>{
    // stores the data as JSON strings
    private ArrayList<String> m_data = new ArrayList<String>();

    // tells us if the ID data exists, and where it is in the array.
    private HashMap<String,Integer> m_idLookup = new HashMap<String,Integer>();

    // Create new views (invoked by the layout manager)
    @Override
    public FireflyListAdapter.ViewHolder onCreateViewHolder(ViewGroup parent,
                                                   int viewType) {
        // // create a new view
        // View itemLayoutView = LayoutInflater.from(parent.getContext())
        //     .inflate(R.layout.item_layout, null);

        // // create ViewHolder
        // ViewHolder viewHolder = new ViewHolder(itemLayoutView);
        // return viewHolder;
	return null;
    }

    // Replace the contents of a view (invoked by the layout manager)
    @Override
    public void onBindViewHolder(ViewHolder viewHolder, int position) {

        // - get data from your itemsData at this position
        // - replace the contents of the view with that itemsData

        // viewHolder.txtViewTitle.setText(itemsData[position].getTitle());
        // viewHolder.imgViewIcon.setImageResource(itemsData[position].getImageUrl());


    }

    // inner class to hold a reference to each item of RecyclerView
    public static class ViewHolder extends RecyclerView.ViewHolder {
        //public TextView txtViewTitle;
        //public ImageView imgViewIcon;

        public ViewHolder(View itemLayoutView) {
            super(itemLayoutView);
            //txtViewTitle = (TextView) itemLayoutView.findViewById(R.id.item_title);
            //imgViewIcon = (ImageView) itemLayoutView.findViewById(R.id.item_icon);
        }
    }

    // Return the size of your itemsData (invoked by the layout manager)
    @Override
    public int getItemCount() {
        return m_data.size();
    }

    //### EVENT BUS ###

}
