package com.caller;

import android.graphics.drawable.Drawable;

public class app_info {

    public String appName = "";
    public String packName = "";
    public Drawable icon;

    public String getAppName() {
        return appName;
    }

    public void setAppName(String appName) {
        this.appName = appName;
    }

    public String getPckName() {
        return packName;
    }

    public void setPckName(String packName) {
        this.packName = packName;
    }

    public Drawable getIcon() {
        return icon;
    }

    public void setIcon(Drawable icon) {
        this.icon = icon;
    }

}

