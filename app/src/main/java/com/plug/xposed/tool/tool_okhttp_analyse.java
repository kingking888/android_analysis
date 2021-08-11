package com.plug.xposed.tool;

import android.app.Application;
import android.content.Context;

import com.plug.base.plug_config;
import com.plug.xposed.base.sub_plug_base;
import com.units.log;
import com.units.units;

import de.robv.android.xposed.XC_MethodHook;
import de.robv.android.xposed.XposedHelpers;
import de.robv.android.xposed.callbacks.XC_LoadPackage;

public class tool_okhttp_analyse extends sub_plug_base {
    static String okhttp_class_name = "okhttp3.OkHttpClient";

    public tool_okhttp_analyse(plug_config config, XC_LoadPackage.LoadPackageParam xposed_param) {
        super(config, xposed_param);
        name = "okhttp_analyse";
    }

    public void do_hook(ClassLoader class_loader) {
        XposedHelpers.findAndHookMethod(okhttp_class_name, class_loader, "newCall",
                XposedHelpers.findClass("okhttp3.Request", class_loader),
                new XC_MethodHook() {
                    @Override
                    protected void beforeHookedMethod(MethodHookParam param) {
                        log.i("newCall -> " + param.args[0].toString());
                        units.log_stack();
                    }
                }
        );
    }

    public void on_loadclass(String class_name, Class<?> cls, ClassLoader class_loader, XC_MethodHook.MethodHookParam param) {
        if (class_name.equals("okhttp3")) {
            do_hook(class_loader);
        }
    }

    public void on_360_reinforce(Context context, ClassLoader class_loader) {
        do_hook(class_loader);
    }

    @Override
    public void on_application_class_loader(Application app, ClassLoader class_loader) {
        do_hook(class_loader);
    }
}
