package com.plug.xposed.base;

import android.app.Application;
import android.content.Context;

import com.plug.base.plug_config;

import de.robv.android.xposed.XC_MethodHook;
import de.robv.android.xposed.callbacks.XC_LoadPackage;

public abstract class sub_plug_base {
    public XC_LoadPackage.LoadPackageParam xposed_param;
    public plug_config config;
    public String name;

    public sub_plug_base(plug_config config, XC_LoadPackage.LoadPackageParam xposed_param) {
        this.config = config;
        this.xposed_param = xposed_param;
    }


    public abstract void on_loadclass(String class_name, Class<?> cls, ClassLoader class_loader, XC_MethodHook.MethodHookParam param);

    public abstract void on_360_reinforce(Context context, ClassLoader class_loader);

    public abstract void on_application_class_loader(Application app, ClassLoader class_loader);
}