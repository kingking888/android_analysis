package com.plug.xposed.tool;

import android.app.Application;
import android.content.Context;

import com.plug.base.plug_config;
import com.plug.xposed.base.sub_plug_base;
import com.units.log;

import java.io.File;

import dalvik.system.DexClassLoader;
import de.robv.android.xposed.XC_MethodHook;
import de.robv.android.xposed.callbacks.XC_LoadPackage;

public class tool_inject_jar extends sub_plug_base {
    public tool_inject_jar(plug_config config, XC_LoadPackage.LoadPackageParam xposed_param) {
        super(config, xposed_param);
        name = "inject_jar";
    }


    @Override
    public void on_loadclass(String class_name, Class<?> cls, ClassLoader class_loader, XC_MethodHook.MethodHookParam param) {

    }

    @Override
    public void on_360_reinforce(Context context, ClassLoader class_loader) {

    }

    @Override
    public void on_application_class_loader(Application app, ClassLoader class_loader) {
        do_hook(app, class_loader);
    }

    void do_hook(Application app, ClassLoader class_loader) {
        try {
            File caller_file = new File(app.getPackageManager().getApplicationInfo("com.caller", 0).sourceDir);
            DexClassLoader dexClassLoader = new DexClassLoader(caller_file.getAbsolutePath(),
                    app.getCacheDir().getAbsolutePath(),
                    null,
                    class_loader);

            dexClassLoader.loadClass("com.plug.export.frida_helper");
            log.i("inject jar success!");
        } catch (Exception e) {
            log.i("inject jar error!" + e);
        }
    }
}
