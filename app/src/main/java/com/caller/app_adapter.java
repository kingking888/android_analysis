package com.caller;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.RelativeLayout;
import android.widget.TextView;

import java.util.ArrayList;

/**
 * Created by acpm on 04/10/15.
 */
public class app_adapter extends BaseAdapter {

    private LayoutInflater inflater;
    private ArrayList<app_info> mApps;

    public app_adapter(Context context, ArrayList<app_info> mApps) {
        this.inflater = LayoutInflater.from(context);
        this.mApps = mApps;
    }

    @Override
    public final int getCount() {
        return mApps.size();
    }

    public final Object getItem(int position) {
        return mApps.get(position);
    }

    public final long getItemId(int position) {
        return position;
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        ViewHandler handler;

        if (convertView == null) {
            convertView = inflater.inflate(R.layout.app_list, null);
            handler = new ViewHandler();
            handler.textLabel = (TextView) convertView.findViewById(R.id.textViewLable);
            handler.packLabel = (TextView) convertView.findViewById(R.id.textViewPackLable);
            handler.bypassLabel = (TextView) convertView.findViewById(R.id.bypassed_txt);
            handler.iconImage = (ImageView) convertView.findViewById(R.id.imageViewIcon);
            convertView.setTag(handler);
        } else {
            handler = (ViewHandler) convertView.getTag();
        }

        app_info info = this.mApps.get(position);
        handler.iconImage.setImageDrawable(info.getIcon());
        RelativeLayout.LayoutParams layoutParams = new RelativeLayout.LayoutParams(90, 90);
        handler.iconImage.setLayoutParams(layoutParams);
        handler.textLabel.setText(info.getAppName());
        handler.packLabel.setText(info.getPckName());

        return convertView;
    }
}

class ViewHandler {
    TextView textLabel;
    TextView packLabel;
    TextView bypassLabel;
    ImageView iconImage;
}
