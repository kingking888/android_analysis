package com.plug.xposed.app_sub_plug;

import android.app.Application;
import android.content.Context;

import com.plug.base.plug_config;
import com.plug.export.frida_helper;
import com.plug.xposed.base.sub_plug_base;
import com.units.log;

import java.io.ByteArrayOutputStream;
import java.net.InetAddress;

import de.robv.android.xposed.XC_MethodHook;
import de.robv.android.xposed.XposedHelpers;
import de.robv.android.xposed.callbacks.XC_LoadPackage;

public class sub_plug_txsd extends sub_plug_base {
    public sub_plug_txsd(plug_config config, XC_LoadPackage.LoadPackageParam xposed_param) {
        super(config, xposed_param);
        name = "sub_plug_txsd";
    }

    @Override
    public void on_loadclass(String class_name, Class<?> cls, ClassLoader class_loader, XC_MethodHook.MethodHookParam param) {

    }

    @Override
    public void on_360_reinforce(Context context, ClassLoader class_loader) {
        ByteArrayOutputStream a ;

    }

    @Override
    public void on_application_class_loader(Application app, ClassLoader class_loader) {
        XposedHelpers.findAndHookMethod("com.meri.service.viruskiller.l$5",
                class_loader,
                "onFinish",
                int.class,
                int.class,
                int.class,
                int.class,
                XposedHelpers.findClass("tcs.bgj", class_loader),
                new XC_MethodHook() {
                    @Override
                    protected void afterHookedMethod(MethodHookParam param) throws Throwable {
                        log.i("---------tcs.bgj------------" + frida_helper.object_2_string(param.args[4]));
                    }
                });
    }
}
